// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Items/GIS_ItemDefinition.h"
#include "Items/GIS_ItemInstance.h"
#include "Engine/Texture2D.h"
#include "Items/GIS_ItemDefinitionSchema.h"
#include "Fragments/GIS_ItemFragment.h"
#include "Misc/DataValidation.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemDefinition)

UGIS_ItemDefinition::UGIS_ItemDefinition(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bUnique = false;
}

const UGIS_ItemFragment* UGIS_ItemDefinition::GetFragment(TSubclassOf<UGIS_ItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UGIS_ItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

bool UGIS_ItemDefinition::HasFloatAttribute(FGameplayTag AttributeTag) const
{
	return FloatAttributeMap.Contains(AttributeTag);
}

bool UGIS_ItemDefinition::HasIntegerAttribute(FGameplayTag AttributeTag) const
{
	return IntegerAttributeMap.Contains(AttributeTag);
}

float UGIS_ItemDefinition::GetFloatAttribute(FGameplayTag AttributeTag) const
{
	return FloatAttributeMap.Contains(AttributeTag) ? FloatAttributeMap[AttributeTag] : 0;
}

int32 UGIS_ItemDefinition::GetIntegerAttribute(FGameplayTag AttributeTag) const
{
	return IntegerAttributeMap.Contains(AttributeTag) ? IntegerAttributeMap[AttributeTag] : 0;
}

const UGIS_ItemFragment* UGIS_ItemDefinition::FindFragmentOfItemDefinition(TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, TSubclassOf<UGIS_ItemFragment> FragmentClass)
{
	if (!ItemDefinition.IsNull())
	{
		const UGIS_ItemDefinition* LoadedDefinition = ItemDefinition.LoadSynchronous();
		if (LoadedDefinition != nullptr)
		{
			return LoadedDefinition->GetFragment(FragmentClass);
		}
	}
	return nullptr;
}

#if WITH_EDITOR
void UGIS_ItemDefinition::PreSave(FObjectPreSaveContext SaveContext)
{
	FloatAttributeMap.Empty();
	for (const FGIS_GameplayTagFloat& Attribute : StaticFloatAttributes)
	{
		FloatAttributeMap.Add(Attribute.Tag, Attribute.Value);
	}

	IntegerAttributeMap.Empty();
	for (const FGIS_GameplayTagInteger& Attribute : StaticIntegerAttributes)
	{
		IntegerAttributeMap.Add(Attribute.Tag, Attribute.Value);
	}

	FText SchemaError;
	UGIS_ItemDefinitionSchema::TryPreSaveItemDefinition(this, SchemaError);
	if (!SchemaError.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDefinition PreSave validation warning for %s: %s"), *GetPathName(), *SchemaError.ToString());
	}

	Super::PreSave(SaveContext);
}

EDataValidationResult UGIS_ItemDefinition::IsDataValid(class FDataValidationContext& Context) const
{
	if (ItemTags.IsEmpty())
	{
		Context.AddWarning(FText::FromString(FString::Format(TEXT("Item tags should not be empty for %s, or it can't be queried by external system."), {GetPathName()})));
	}

	// Check for duplicate fragment types
	TSet<TSubclassOf<UGIS_ItemFragment>> FragmentClassSet;
	for (const UGIS_ItemFragment* Fragment : Fragments)
	{
		if (Fragment != nullptr)
		{
			TSubclassOf<UGIS_ItemFragment> FragmentClass = Fragment->GetClass();
			if (FragmentClassSet.Contains(FragmentClass))
			{
				Context.AddError(FText::FromString(FString::Format(TEXT("Duplicate fragment type {0} found in Fragments array for %s."), {FragmentClass->GetName(), GetPathName()})));
				return EDataValidationResult::Invalid;
			}
			FragmentClassSet.Add(FragmentClass);
		}
	}

	FText SchemaError;
	if (!UGIS_ItemDefinitionSchema::TryValidateItemDefinition(this, SchemaError))
	{
		Context.AddError(SchemaError);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif
