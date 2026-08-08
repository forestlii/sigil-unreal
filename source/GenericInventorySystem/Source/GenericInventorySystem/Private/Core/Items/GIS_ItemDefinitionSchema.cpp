// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Items/GIS_ItemDefinitionSchema.h"
#include "GIS_InventorySystemSettings.h"
#include "Items/GIS_ItemDefinition.h"
#include "Fragments/GIS_ItemFragment.h"
#include "Misc/DataValidation.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemDefinitionSchema)

bool UGIS_ItemDefinitionSchema::TryValidateItemDefinition(const UGIS_ItemDefinition* Definition, FText& OutError)
{
	if (Definition == nullptr)
	{
		OutError = FText::FromString(TEXT("Item definition is null."));
		return false;
	}

	if (const UGIS_InventorySystemSettings* Settings = UGIS_InventorySystemSettings::Get())
	{
		FString AssetPath = Definition->GetPathName();
		const UGIS_ItemDefinitionSchema* Schema = Settings->GetItemDefinitionSchemaForAsset(AssetPath);
		if (Schema)
		{
			return Schema->TryValidate(Definition, OutError);
		}
		// OutError = FText::FromString(FString::Format(TEXT("No valid schema found for item definition at path: {0}."), {AssetPath}));
	}
	return true;
}

void UGIS_ItemDefinitionSchema::TryPreSaveItemDefinition(UGIS_ItemDefinition* Definition, FText& OutError)
{
	if (Definition == nullptr)
	{
		OutError = FText::FromString(TEXT("Item definition is null."));
		return;
	}

	if (const UGIS_InventorySystemSettings* Settings = UGIS_InventorySystemSettings::Get())
	{
		FString AssetPath = Definition->GetPathName();
		const UGIS_ItemDefinitionSchema* Schema = Settings->GetItemDefinitionSchemaForAsset(AssetPath);
		if (Schema)
		{
			Schema->TryPreSave(Definition, OutError);
		}
		// OutError = FText::FromString(FString::Format(TEXT("No valid schema found for item definition at path: {0}."), {AssetPath}));
	}
}

bool UGIS_ItemDefinitionSchema::TryValidate(const UGIS_ItemDefinition* Definition, FText& OutError) const
{
	if (Definition == nullptr)
	{
		OutError = FText::FromString(TEXT("Item definition is null."));
		return false;
	}

	// Validate parent tag: all ItemTags must be children of RequiredParentTag
	if (RequiredParentTag.IsValid())
	{
		for (const FGameplayTag& Tag : Definition->ItemTags.GetGameplayTagArray())
		{
			if (!Tag.MatchesTag(RequiredParentTag))
			{
				OutError = FText::FromString(FString::Format(TEXT("Tag {0} is not a child of required parent tag {1}."), {Tag.ToString(), RequiredParentTag.ToString()}));
				return false;
			}
		}
	}

	FGIS_ItemDefinitionValidationEntry FoundEntry;
	bool bFoundEntry = false;
	int32 ValidationEntryIndex = -1;

	// Find matching validation entry
	for (int32 Index = 0; Index < ValidationEntries.Num(); ++Index)
	{
		const FGIS_ItemDefinitionValidationEntry& Entry = ValidationEntries[Index];
		if (!Entry.ItemTagQuery.IsEmpty() && Definition->ItemTags.MatchesQuery(Entry.ItemTagQuery))
		{
			FoundEntry = Entry;
			bFoundEntry = true;
			ValidationEntryIndex = Index;
			break;
		}
	}

	if (!bFoundEntry)
	{
		return true; // No matching entry
	}

	TMap<TSubclassOf<UGIS_ItemFragment>, int32> FragmentClassMap;
	TArray<TSubclassOf<UGIS_ItemFragment>> FragmentClasses;

	// Build fragment class map and check for duplicate fragment types
	for (const UGIS_ItemFragment* Fragment : Definition->Fragments)
	{
		if (Fragment != nullptr)
		{
			TSubclassOf<UGIS_ItemFragment> FragmentClass = Fragment->GetClass();
			int32& Count = FragmentClassMap.FindOrAdd(FragmentClass);
			Count++;
			if (Count > 1)
			{
				OutError = FText::FromString(FString::Format(TEXT("Duplicate fragment type {0} found in Fragments array for schema {1}."), {FragmentClass->GetName(), GetPathName()}));
				return false;
			}
		}
	}

	FragmentClassMap.GetKeys(FragmentClasses);

	// Validate common required fragments
	for (const TSoftClassPtr<UGIS_ItemFragment>& CommonFragment : CommonRequiredFragments)
	{
		if (UClass* LoadedClass = CommonFragment.LoadSynchronous())
		{
			if (!FragmentClasses.Contains(LoadedClass))
			{
				OutError = FText::FromString(
					FString::Format(TEXT("Missing common required fragment of type {0} for ValidationEntry {1} in schema {2}."), {LoadedClass->GetName(), ValidationEntryIndex, GetPathName()}));
				return false;
			}
		}
	}

	// Validate forbidden fragments, excluding common required fragments and required fragments
	TSet<TSoftClassPtr<UGIS_ItemFragment>> EffectiveRequiredFragments;
	for (const TSoftClassPtr<UGIS_ItemFragment>& RequiredFragment : FoundEntry.RequiredFragments)
	{
		if (!CommonRequiredFragments.Contains(RequiredFragment))
		{
			EffectiveRequiredFragments.Add(RequiredFragment);
		}
	}
	for (const TSoftClassPtr<UGIS_ItemFragment>& ForbiddenFragment : FoundEntry.ForbiddenFragments)
	{
		if (UClass* LoadedClass = ForbiddenFragment.LoadSynchronous())
		{
			if (CommonRequiredFragments.Contains(ForbiddenFragment))
			{
				OutError = FText::FromString(FString::Format(
					TEXT("Forbidden fragment {0} in ValidationEntry {1} is a common required fragment in schema {2}."), {LoadedClass->GetName(), ValidationEntryIndex, GetPathName()}));
				return false;
			}
			if (EffectiveRequiredFragments.Contains(ForbiddenFragment))
			{
				OutError = FText::FromString(
					FString::Format(TEXT("Forbidden fragment {0} in ValidationEntry {1} is a required fragment in schema {2}."), {LoadedClass->GetName(), ValidationEntryIndex, GetPathName()}));
				return false;
			}
			if (FragmentClasses.Contains(LoadedClass))
			{
				OutError = FText::FromString(FString::Format(
					TEXT("Forbidden fragment of type {0} found in definition for ValidationEntry {1} in schema {2}."), {LoadedClass->GetName(), ValidationEntryIndex, GetPathName()}));
				return false;
			}
		}
	}

	// Validate required fragments, excluding those already in CommonRequiredFragments
	for (const TSoftClassPtr<UGIS_ItemFragment>& RequiredFragment : EffectiveRequiredFragments)
	{
		if (UClass* LoadedClass = RequiredFragment.LoadSynchronous())
		{
			if (!FragmentClasses.Contains(LoadedClass))
			{
				OutError = FText::FromString(
					FString::Format(TEXT("Missing required fragment of type {0} for ValidationEntry {1} in schema {2}."), {LoadedClass->GetName(), ValidationEntryIndex, GetPathName()}));
				return false;
			}
		}
	}

	// Validate required float attributes
	for (const FGIS_GameplayTagFloat& RequiredFloat : FoundEntry.RequiredFloatAttributes)
	{
		bool bFound = false;
		for (const FGIS_GameplayTagFloat& FloatAttr : Definition->StaticFloatAttributes)
		{
			if (FloatAttr.Tag == RequiredFloat.Tag)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			OutError = FText::FromString(
				FString::Format(TEXT("Missing required float attribute {0} for ValidationEntry {1} in schema {2}."), {RequiredFloat.Tag.ToString(), ValidationEntryIndex, GetPathName()}));
			return false;
		}
	}

	// Validate required integer attributes
	for (const FGIS_GameplayTagInteger& RequiredInteger : FoundEntry.RequiredIntegerAttributes)
	{
		bool bFound = false;
		for (const FGIS_GameplayTagInteger& IntegerAttr : Definition->StaticIntegerAttributes)
		{
			if (IntegerAttr.Tag == RequiredInteger.Tag)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			OutError = FText::FromString(
				FString::Format(TEXT("Missing required integer attribute {0} for ValidationEntry {1} in schema {2}."), {RequiredInteger.Tag.ToString(), ValidationEntryIndex, GetPathName()}));
			return false;
		}
	}

	// Validate bUnique
	if (FoundEntry.bEnforceUnique && Definition->bUnique != FoundEntry.RequiredUniqueValue)
	{
		OutError = FText::FromString(FString::Format(
			TEXT("bUnique must be {0} for ValidationEntry {1} in schema {2}."), {FoundEntry.RequiredUniqueValue ? TEXT("true") : TEXT("false"), ValidationEntryIndex, GetPathName()}));
		return false;
	}

	return true;
}

void UGIS_ItemDefinitionSchema::TryPreSave(UGIS_ItemDefinition* Definition, FText& OutError) const
{
	if (Definition == nullptr)
	{
		OutError = FText::FromString(TEXT("Item definition is null."));
		return;
	}

	FGIS_ItemDefinitionValidationEntry FoundEntry;
	bool bFoundEntry = false;

	// Find matching validation entry
	for (const FGIS_ItemDefinitionValidationEntry& Entry : ValidationEntries)
	{
		if (!Entry.ItemTagQuery.IsEmpty() && Definition->ItemTags.MatchesQuery(Entry.ItemTagQuery))
		{
			FoundEntry = Entry;
			bFoundEntry = true;
			break;
		}
	}

	// Collect required fragments
	TSet<TSoftClassPtr<UGIS_ItemFragment>> RequiredFragmentSet;
	for (const TSoftClassPtr<UGIS_ItemFragment>& CommonFragment : CommonRequiredFragments)
	{
		RequiredFragmentSet.Add(CommonFragment);
	}
	if (bFoundEntry)
	{
		for (const TSoftClassPtr<UGIS_ItemFragment>& RequiredFragment : FoundEntry.RequiredFragments)
		{
			if (!CommonRequiredFragments.Contains(RequiredFragment))
			{
				RequiredFragmentSet.Add(RequiredFragment);
			}
		}
	}

	// Collect forbidden fragments
	TSet<TSoftClassPtr<UGIS_ItemFragment>> ForbiddenFragmentSet;
	if (bFoundEntry)
	{
		for (const TSoftClassPtr<UGIS_ItemFragment>& ForbiddenFragment : FoundEntry.ForbiddenFragments)
		{
			if (!CommonRequiredFragments.Contains(ForbiddenFragment) && !RequiredFragmentSet.Contains(ForbiddenFragment))
			{
				ForbiddenFragmentSet.Add(ForbiddenFragment);
			}
		}
	}

	// Build map of existing fragments, keeping only the first instance of each type
	TMap<TSubclassOf<UGIS_ItemFragment>, TObjectPtr<UGIS_ItemFragment>> ExistingFragmentMap;
	for (TObjectPtr<UGIS_ItemFragment> Fragment : Definition->Fragments)
	{
		if (Fragment != nullptr)
		{
			TSubclassOf<UGIS_ItemFragment> FragmentClass = Fragment->GetClass();
			if (!ExistingFragmentMap.Contains(FragmentClass))
			{
				ExistingFragmentMap.Add(FragmentClass, Fragment);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Removed duplicate fragment of type %s from ItemDefinition for schema %s."), *FragmentClass->GetName(), *GetPathName());
			}
		}
	}

	// Remove forbidden fragments
	for (const TSoftClassPtr<UGIS_ItemFragment>& ForbiddenFragment : ForbiddenFragmentSet)
	{
		if (UClass* ForbiddenClass = ForbiddenFragment.LoadSynchronous())
		{
			if (ExistingFragmentMap.Remove(ForbiddenClass))
			{
				UE_LOG(LogTemp, Warning, TEXT("Removed forbidden fragment of type %s from ItemDefinition for schema %s."), *ForbiddenClass->GetName(), *GetPathName());
			}
		}
	}

	// Add missing required fragments
	for (const TSoftClassPtr<UGIS_ItemFragment>& RequiredFragment : RequiredFragmentSet)
	{
		if (UClass* FragmentClass = RequiredFragment.LoadSynchronous())
		{
			if (!ExistingFragmentMap.Contains(FragmentClass))
			{
				UGIS_ItemFragment* NewFragment = NewObject<UGIS_ItemFragment>(Definition, FragmentClass);
				if (NewFragment)
				{
					ExistingFragmentMap.Add(FragmentClass, NewFragment);
					UE_LOG(LogTemp, Warning, TEXT("Added missing required fragment of type %s to ItemDefinition for schema %s."), *FragmentClass->GetName(), *GetPathName());
				}
			}
		}
	}

	// Sort fragments according to FragmentOrder
	TArray<TObjectPtr<UGIS_ItemFragment>> NewFragments;
	TArray<TObjectPtr<UGIS_ItemFragment>> NonRequiredFragments;

	// Cache loaded fragment classes to avoid repeated LoadSynchronous calls
	TMap<TSoftClassPtr<UGIS_ItemFragment>, UClass*> FragmentClassCache;
	for (const TSoftClassPtr<UGIS_ItemFragment>& OrderedFragment : FragmentOrder)
	{
		UClass* FragmentClass = FragmentClassCache.FindOrAdd(OrderedFragment, OrderedFragment.LoadSynchronous());
		if (FragmentClass)
		{
			if (TObjectPtr<UGIS_ItemFragment>* ExistingFragment = ExistingFragmentMap.Find(FragmentClass))
			{
				NewFragments.Add(*ExistingFragment);
				ExistingFragmentMap.Remove(FragmentClass);
			}
		}
	}

	// Append remaining non-required fragments
	ExistingFragmentMap.GenerateValueArray(NonRequiredFragments);
	for (const TObjectPtr<UGIS_ItemFragment>& Fragment : NonRequiredFragments)
	{
		if (Fragment && !FragmentOrder.Contains(Fragment->GetClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Fragment type %s is not included in FragmentOrder, appended to end for schema %s."), *Fragment->GetClass()->GetName(), *GetPathName());
		}
	}
	NewFragments.Append(NonRequiredFragments);

	// Update the definition's fragment array
	Definition->Fragments = MoveTemp(NewFragments);

	// Auto-fix required attributes and bUnique if a matching entry was found
	if (bFoundEntry)
	{
		// Auto-fix float attributes
		for (const FGIS_GameplayTagFloat& RequiredFloat : FoundEntry.RequiredFloatAttributes)
		{
			bool bFound = false;
			for (FGIS_GameplayTagFloat& FloatAttr : Definition->StaticFloatAttributes)
			{
				if (FloatAttr.Tag == RequiredFloat.Tag)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
			{
				Definition->StaticFloatAttributes.Add(RequiredFloat);
				UE_LOG(LogTemp, Warning, TEXT("Added missing required float attribute %s to ItemDefinition for schema %s."), *RequiredFloat.Tag.ToString(), *GetPathName());
			}
		}

		// Auto-fix integer attributes
		for (const FGIS_GameplayTagInteger& RequiredInteger : FoundEntry.RequiredIntegerAttributes)
		{
			bool bFound = false;
			for (FGIS_GameplayTagInteger& IntegerAttr : Definition->StaticIntegerAttributes)
			{
				if (IntegerAttr.Tag == RequiredInteger.Tag)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
			{
				Definition->StaticIntegerAttributes.Add(RequiredInteger);
				UE_LOG(LogTemp, Warning, TEXT("Added missing required integer attribute %s to ItemDefinition for schema %s."), *RequiredInteger.Tag.ToString(), *GetPathName());
			}
		}

		// Auto-fix bUnique
		if (FoundEntry.bEnforceUnique)
		{
			if (Definition->bUnique != FoundEntry.RequiredUniqueValue)
			{
				Definition->bUnique = FoundEntry.RequiredUniqueValue;
				UE_LOG(LogTemp, Warning, TEXT("Set bUnique to %s for ItemDefinition for schema %s."), FoundEntry.RequiredUniqueValue ? TEXT("true") : TEXT("false"), *GetPathName());
			}
		}
	}

	// Mark the definition as modified
	Definition->Modify();
}

#if WITH_EDITOR
void UGIS_ItemDefinitionSchema::PreSave(FObjectPreSaveContext SaveContext)
{
	// Remove RequiredFragments and ForbiddenFragments that overlap with CommonRequiredFragments
	for (int32 EntryIndex = 0; EntryIndex < ValidationEntries.Num(); ++EntryIndex)
	{
		FGIS_ItemDefinitionValidationEntry& Entry = ValidationEntries[EntryIndex];
		int32 InitialRequiredCount = Entry.RequiredFragments.Num();
		int32 InitialForbiddenCount = Entry.ForbiddenFragments.Num();

		Entry.RequiredFragments.RemoveAll([&](const TSoftClassPtr<UGIS_ItemFragment>& Fragment)
		{
			if (CommonRequiredFragments.Contains(Fragment))
			{
				if (UClass* LoadedClass = Fragment.LoadSynchronous())
				{
					UE_LOG(LogTemp, Warning, TEXT("Removed fragment %s from ValidationEntry %d RequiredFragments as it is already in CommonRequiredFragments for schema %s."), *LoadedClass->GetName(),
					       EntryIndex, *GetPathName());
				}
				return true;
			}
			return false;
		});

		Entry.ForbiddenFragments.RemoveAll([&](const TSoftClassPtr<UGIS_ItemFragment>& Fragment)
		{
			if (CommonRequiredFragments.Contains(Fragment))
			{
				if (UClass* LoadedClass = Fragment.LoadSynchronous())
				{
					UE_LOG(LogTemp, Warning, TEXT("Removed fragment %s from ValidationEntry %d ForbiddenFragments as it is in CommonRequiredFragments for schema %s."), *LoadedClass->GetName(),
					       EntryIndex, *GetPathName());
				}
				return true;
			}
			return false;
		});

		if (Entry.RequiredFragments.Num() < InitialRequiredCount)
		{
			UE_LOG(LogTemp, Warning, TEXT("Removed %d fragment(s) from ValidationEntry %d RequiredFragments for schema %s."), InitialRequiredCount - Entry.RequiredFragments.Num(), EntryIndex,
			       *GetPathName());
		}
		if (Entry.ForbiddenFragments.Num() < InitialForbiddenCount)
		{
			UE_LOG(LogTemp, Warning, TEXT("Removed %d fragment(s) from ValidationEntry %d ForbiddenFragments for schema %s."), InitialForbiddenCount - Entry.ForbiddenFragments.Num(), EntryIndex,
			       *GetPathName());
		}
	}

	// Mark the schema as modified if changes were made
	Modify();

	Super::PreSave(SaveContext);
}

EDataValidationResult UGIS_ItemDefinitionSchema::IsDataValid(class FDataValidationContext& Context) const
{
	// Validate that RequiredFragments do not overlap with CommonRequiredFragments
	for (int32 EntryIndex = 0; EntryIndex < ValidationEntries.Num(); ++EntryIndex)
	{
		const FGIS_ItemDefinitionValidationEntry& Entry = ValidationEntries[EntryIndex];
		for (const TSoftClassPtr<UGIS_ItemFragment>& RequiredFragment : Entry.RequiredFragments)
		{
			if (CommonRequiredFragments.Contains(RequiredFragment))
			{
				if (UClass* LoadedClass = RequiredFragment.LoadSynchronous())
				{
					Context.AddWarning(FText::FromString(
						FString::Format(
							TEXT("ValidationEntry {0} contains fragment {1} in RequiredFragments, which is already in CommonRequiredFragments for schema {2}."),
							{EntryIndex, LoadedClass->GetName(), GetPathName()})));
				}
			}
		}

		// Validate that ForbiddenFragments do not overlap with CommonRequiredFragments or RequiredFragments
		TSet<TSoftClassPtr<UGIS_ItemFragment>> EffectiveRequiredFragments;
		for (const TSoftClassPtr<UGIS_ItemFragment>& RequiredFragment : Entry.RequiredFragments)
		{
			if (!CommonRequiredFragments.Contains(RequiredFragment))
			{
				EffectiveRequiredFragments.Add(RequiredFragment);
			}
		}
		for (const TSoftClassPtr<UGIS_ItemFragment>& ForbiddenFragment : Entry.ForbiddenFragments)
		{
			if (CommonRequiredFragments.Contains(ForbiddenFragment))
			{
				if (UClass* LoadedClass = ForbiddenFragment.LoadSynchronous())
				{
					Context.AddError(FText::FromString(
						FString::Format(
							TEXT("ValidationEntry {0} contains fragment {1} in ForbiddenFragments, which is in CommonRequiredFragments for schema {2}."),
							{EntryIndex, LoadedClass->GetName(), GetPathName()})));
					return EDataValidationResult::Invalid;
				}
			}
			if (EffectiveRequiredFragments.Contains(ForbiddenFragment))
			{
				if (UClass* LoadedClass = ForbiddenFragment.LoadSynchronous())
				{
					Context.AddError(FText::FromString(
						FString::Format(
							TEXT("ValidationEntry {0} contains fragment {1} in ForbiddenFragments, which is in RequiredFragments for schema {2}."),
							{EntryIndex, LoadedClass->GetName(), GetPathName()})));
					return EDataValidationResult::Invalid;
				}
			}
		}
	}

	// Validate FragmentOrder contains no duplicates
	TSet<TSoftClassPtr<UGIS_ItemFragment>> FragmentOrderSet;
	for (const TSoftClassPtr<UGIS_ItemFragment>& Fragment : FragmentOrder)
	{
		if (FragmentOrderSet.Contains(Fragment))
		{
			if (UClass* LoadedClass = Fragment.LoadSynchronous())
			{
				Context.AddError(FText::FromString(FString::Format(TEXT("FragmentOrder contains duplicate fragment {0} for schema {1}."), {LoadedClass->GetName(), GetPathName()})));
				return EDataValidationResult::Invalid;
			}
		}
		FragmentOrderSet.Add(Fragment);
	}

	// Suggest including all CommonRequiredFragments and RequiredFragments in FragmentOrder
	TSet<TSoftClassPtr<UGIS_ItemFragment>> AllRequiredFragments = TSet<TSoftClassPtr<UGIS_ItemFragment>>(CommonRequiredFragments);
	for (const FGIS_ItemDefinitionValidationEntry& Entry : ValidationEntries)
	{
		for (const TSoftClassPtr<UGIS_ItemFragment>& RequiredFragment : Entry.RequiredFragments)
		{
			if (!CommonRequiredFragments.Contains(RequiredFragment))
			{
				AllRequiredFragments.Add(RequiredFragment);
			}
		}
	}
	for (const TSoftClassPtr<UGIS_ItemFragment>& RequiredFragment : AllRequiredFragments)
	{
		if (!FragmentOrder.Contains(RequiredFragment))
		{
			if (UClass* LoadedClass = RequiredFragment.LoadSynchronous())
			{
				Context.AddWarning(FText::FromString(FString::Format(TEXT("Required fragment {0} is not included in FragmentOrder for schema {1}."), {LoadedClass->GetName(), GetPathName()})));
			}
		}
	}

	return Context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif
