// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Abilities/SigilAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "Runtime/Launch/Resources/Version.h"
#include "AbilitySystemComponent.h"
#include "Misc/SecureHash.h"
#include "UObject/SoftObjectPath.h"
#include "Utilities/SigilAbilitySystemFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogSigilAbilitySet)

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAbilitySet)

namespace
{
	void AppendCanonicalField(FString& Target, const FString& Field)
	{
		Target.Appendf(TEXT("%d:"), Field.Len());
		Target.Append(Field);
		Target.AppendChar(TEXT(';'));
	}

	FString HashCanonicalString(const FString& Canonical)
	{
		const FTCHARToUTF8 Utf8(*Canonical);
		return FSHA1::HashBuffer(Utf8.Get(), static_cast<uint64>(Utf8.Length())).ToString();
	}
}

void FSigilAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FSigilAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FSigilAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}

void FSigilAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	if (!ASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		ASC->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

USigilAbilitySet::USigilAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool USigilAbilitySet::BuildAbilityOnlyEntitlementGrantPlan(int32 OverrideLevel, FAbilityOnlyGrantPlan& OutPlan, FString& OutError) const
{
	OutPlan = FAbilityOnlyGrantPlan();
	OutError.Reset();

	if (OverrideLevel != INDEX_NONE && OverrideLevel < 1)
	{
		OutError = TEXT("OverrideLevel must be INDEX_NONE or at least one.");
		return false;
	}

	if (!GrantedGameplayEffects.IsEmpty() || !GrantedAttributes.IsEmpty())
	{
		OutError = TEXT("Entitlement projection accepts ability-only sets.");
		return false;
	}

	if (GrantedGameplayAbilities.IsEmpty())
	{
		OutError = TEXT("Entitlement projection does not accept an empty ability set.");
		return false;
	}

	TSet<FString> SeenAbilityClasses;
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FSigilAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

#if WITH_EDITORONLY_DATA
		if (!AbilityToGrant.bAbilityEnabled)
		{
			OutError = FString::Printf(TEXT("GrantedGameplayAbilities[%d] is editor-disabled and cannot produce a build-stable entitlement identity."), AbilityIndex);
			return false;
		}
#endif

		if (AbilityToGrant.Ability.IsNull() || !AbilityToGrant.Ability.IsValid())
		{
			OutError = FString::Printf(TEXT("GrantedGameplayAbilities[%d] must already be loaded."), AbilityIndex);
			return false;
		}

		const TSubclassOf<UGameplayAbility> AbilityClass = AbilityToGrant.Ability.Get();
		if (!AbilityClass || AbilityClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			OutError = FString::Printf(TEXT("GrantedGameplayAbilities[%d] is not a concrete, current ability class."), AbilityIndex);
			return false;
		}

		const int32 EffectiveLevel = OverrideLevel > 0 ? OverrideLevel : AbilityToGrant.AbilityLevel;
		if (EffectiveLevel < 1)
		{
			OutError = FString::Printf(TEXT("GrantedGameplayAbilities[%d] has an invalid effective level."), AbilityIndex);
			return false;
		}

		const FString AbilityClassPath = AbilityClass->GetPathName();
		if (SeenAbilityClasses.Contains(AbilityClassPath))
		{
			OutError = FString::Printf(TEXT("Ability class [%s] appears more than once in one entitlement identity."), *AbilityClassPath);
			return false;
		}
		SeenAbilityClasses.Add(AbilityClassPath);

		TArray<FString> DynamicTagStrings;
		for (const FGameplayTag& DynamicTag : AbilityToGrant.DynamicTags)
		{
			if (!DynamicTag.IsValid())
			{
				OutError = FString::Printf(TEXT("GrantedGameplayAbilities[%d] contains an invalid dynamic tag."), AbilityIndex);
				return false;
			}
			DynamicTagStrings.Add(DynamicTag.ToString());
		}
		DynamicTagStrings.Sort();

		FString CanonicalEntry;
		AppendCanonicalField(CanonicalEntry, AbilityClassPath);
		AppendCanonicalField(CanonicalEntry, LexToString(EffectiveLevel));
		AppendCanonicalField(CanonicalEntry, LexToString(AbilityToGrant.InputID > 0 ? AbilityToGrant.InputID : INDEX_NONE));
		for (const FString& DynamicTagString : DynamicTagStrings)
		{
			AppendCanonicalField(CanonicalEntry, DynamicTagString);
		}

		FAbilityOnlyGrantPlanEntry& PlanEntry = OutPlan.Entries.AddDefaulted_GetRef();
		PlanEntry.AbilityClass = AbilityClass;
		PlanEntry.AbilityLevel = EffectiveLevel;
		PlanEntry.InputID = AbilityToGrant.InputID > 0 ? AbilityToGrant.InputID : INDEX_NONE;
		PlanEntry.DynamicTags = AbilityToGrant.DynamicTags;
		PlanEntry.AbilityClassPath = AbilityClassPath;
		PlanEntry.CanonicalEntry = MoveTemp(CanonicalEntry);
		OutPlan.AbilityClassPaths.Add(AbilityClassPath);
	}

	OutPlan.Entries.Sort([](const FAbilityOnlyGrantPlanEntry& Left, const FAbilityOnlyGrantPlanEntry& Right)
	{
		return Left.CanonicalEntry < Right.CanonicalEntry;
	});

	const FPrimaryAssetId PrimaryAssetId = GetPrimaryAssetId();
	const FString AssetIdentity = PrimaryAssetId.IsValid() ? PrimaryAssetId.ToString() : FSoftObjectPath(this).ToString();
	FString CanonicalIdentity;
	AppendCanonicalField(CanonicalIdentity, TEXT("SigilAbilityEntitlementIdentityV1"));
	AppendCanonicalField(CanonicalIdentity, AssetIdentity);
	for (const FAbilityOnlyGrantPlanEntry& PlanEntry : OutPlan.Entries)
	{
		AppendCanonicalField(CanonicalIdentity, PlanEntry.CanonicalEntry);
	}
	OutPlan.CanonicalIdentity = HashCanonicalString(CanonicalIdentity);
	return true;
}

void USigilAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, FSigilAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject, int32 OverrideLevel) const
{
	check(ASC);

	if (!ASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	// Grant the attribute sets.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FSigilAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		const TSubclassOf<UAttributeSet> AttributeSetClass = SetToGrant.AttributeSet.LoadSynchronous();

		if (!AttributeSetClass)
		{
			UE_LOG(LogSigilAbilitySet, Error, TEXT("GrantedAttributes[%d] on ability set [%s]: AttributeSet is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		if (UAttributeSet* ExistingOne = USigilAbilitySystemFunctionLibrary::GetAttributeSetByClass(ASC, AttributeSetClass))
		{
			UE_LOG(LogSigilAbilitySet, Error, TEXT("GrantedAttributes[%d] on ability set [%s]: AttributeSet already exists."), SetIndex, *GetNameSafe(this));
			continue;
		}


#if WITH_EDITORONLY_DATA
		if (!SetToGrant.bAttributeSetEnabled)
		{
			UE_LOG(LogSigilAbilitySet, Display, TEXT( "GrantedAttributes[%d] on ability set [%s]:skipped for debugging."), SetIndex, *GetNameSafe(this));
			continue;
		}
#endif

		UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), AttributeSetClass);
		ASC->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}

	// Grant the gameplay abilities.
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FSigilAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		const TSubclassOf<UGameplayAbility> AbilityClass = AbilityToGrant.Ability.LoadSynchronous();

		if (!AbilityClass)
		{
			UE_LOG(LogSigilAbilitySet, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s]: Ability class is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

#if WITH_EDITORONLY_DATA
		if (!AbilityToGrant.bAbilityEnabled)
		{
			UE_LOG(LogSigilAbilitySet, Display, TEXT( "GrantedGameplayAbilities[%d] on ability set [%s]: Skipped for debugging."), AbilityIndex, *GetNameSafe(this));
			continue;
		}
#endif


		UGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, OverrideLevel > 0 ? OverrideLevel : AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;

		if (AbilityToGrant.InputID > 0)
		{
			AbilitySpec.InputID = AbilityToGrant.InputID;
		}

		if (!AbilityToGrant.DynamicTags.IsEmpty())
		{
#if ENGINE_MINOR_VERSION > 4
			AbilitySpec.GetDynamicSpecSourceTags().AppendTags(AbilityToGrant.DynamicTags);
#else
			AbilitySpec.DynamicAbilityTags.AppendTags(AbilityToGrant.DynamicTags);
#endif
		}

		const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}

	// Grant the gameplay effects.
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FSigilAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];
		const TSubclassOf<UGameplayEffect> EffectClass = EffectToGrant.GameplayEffect.LoadSynchronous();

		if (!EffectClass)
		{
			UE_LOG(LogSigilAbilitySet, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s]:Effect Class  is not valid"), EffectIndex, *GetNameSafe(this));
			continue;
		}

#if WITH_EDITORONLY_DATA
		if (!EffectToGrant.bEffectEnabled)
		{
			UE_LOG(LogSigilAbilitySet, Display, TEXT( "GrantedGameplayEffects[%d] on ability set [%s]:Skipped for debugging."), EffectIndex, *GetNameSafe(this));
			continue;
		}
#endif

		const UGameplayEffect* GameplayEffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();

		const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->
			ApplyGameplayEffectToSelf(GameplayEffectCDO, OverrideLevel > 0 ? OverrideLevel : EffectToGrant.EffectLevel, ASC->MakeEffectContext());
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
			if (GameplayEffectCDO->DurationPolicy == EGameplayEffectDurationType::Infinite && !GameplayEffectHandle.IsValid())
			{
				UE_LOG(LogSigilAbilitySet, Warning, TEXT("Granted Infinite GameplayEffects[%d] on ability set [%s] failed to apply"), EffectIndex, *GetNameSafe(this));
			}
		}
	}
}

FSigilAbilitySet_GrantedHandles USigilAbilitySet::GiveAbilitySetToAbilitySystem(TSoftObjectPtr<USigilAbilitySet> AbilitySet, UAbilitySystemComponent* ASC, UObject* SourceObject, int32 OverrideLevel)
{
	FSigilAbilitySet_GrantedHandles GrantedHandles;
	if (IsValid(ASC) && !AbilitySet.IsNull())
	{
		USigilAbilitySet* ResolvedAbilitySet = AbilitySet.Get();
		if (!IsValid(ResolvedAbilitySet))
		{
			ResolvedAbilitySet = AbilitySet.LoadSynchronous();
		}
		if (IsValid(ResolvedAbilitySet))
		{
			ResolvedAbilitySet->GiveToAbilitySystem(ASC, &GrantedHandles, SourceObject, OverrideLevel);
		}
	}
	return GrantedHandles;
}

void USigilAbilitySet::TakeAbilitySetFromAbilitySystem(FSigilAbilitySet_GrantedHandles& GrantedHandles, UAbilitySystemComponent* ASC)
{
	if (IsValid(ASC))
	{
		GrantedHandles.TakeFromAbilitySystem(ASC);
	}
}

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

void FSigilAbilitySet_GameplayAbility::MakeEditorFriendlyName()
{
	EditorFriendlyName = "Empty Ability";

	if (!Ability.IsNull())
	{
		if (TSubclassOf<UGameplayAbility> Loaded = Ability.LoadSynchronous())
		{
			EditorFriendlyName = Loaded->GetDisplayNameText().ToString();
		}
	}
}

void FSigilAbilitySet_GameplayEffect::MakeEditorFriendlyName()
{
	EditorFriendlyName = "Empty Effect";

	if (!GameplayEffect.IsNull())
	{
		if (TSubclassOf<UGameplayEffect> Loaded = GameplayEffect.LoadSynchronous())
		{
			EditorFriendlyName = Loaded->GetDisplayNameText().ToString();
		}
	}
}

void FSigilAbilitySet_AttributeSet::MakeEditorFriendlyName()
{
	EditorFriendlyName = "Empty Attribute Set";

	if (!AttributeSet.IsNull())
	{
		if (TSubclassOf<UAttributeSet> Loaded = AttributeSet.LoadSynchronous())
		{
			EditorFriendlyName = Loaded->GetDisplayNameText().ToString();
		}
	}
}

void USigilAbilitySet::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
	if (!IsRunningCommandlet())
	{
		for (auto& Ability : GrantedGameplayAbilities)
		{
			Ability.MakeEditorFriendlyName();
		}
		for (auto& Effect : GrantedGameplayEffects)
		{
			Effect.MakeEditorFriendlyName();
		}

		for (auto& Attribute : GrantedAttributes)
		{
			Attribute.MakeEditorFriendlyName();
		}
	}
}

void USigilAbilitySet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void USigilAbilitySet::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	FName MemberName = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue()->GetFName();
	if (PropertyChangedEvent.GetPropertyName() == TEXT("Ability") && MemberName == GET_MEMBER_NAME_CHECKED(USigilAbilitySet, GrantedGameplayAbilities))
	{
		const int32 Index = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_CHECKED(USigilAbilitySet, GrantedGameplayAbilities).ToString());
		if (Index != INDEX_NONE)
		{
			GrantedGameplayAbilities[Index].MakeEditorFriendlyName();
		}
	}

	if (PropertyChangedEvent.GetPropertyName() == TEXT("GameplayEffect") && MemberName == GET_MEMBER_NAME_CHECKED(USigilAbilitySet, GrantedGameplayEffects))
	{
		const int32 Index = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_CHECKED(USigilAbilitySet, GrantedGameplayEffects).ToString());
		if (Index != INDEX_NONE)
		{
			GrantedGameplayEffects[Index].MakeEditorFriendlyName();
		}
	}

	if (PropertyChangedEvent.GetPropertyName() == TEXT("AttributeSet") && MemberName == GET_MEMBER_NAME_CHECKED(USigilAbilitySet, GrantedAttributes))
	{
		const int32 Index = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_CHECKED(USigilAbilitySet, GrantedGameplayEffects).ToString());
		if (Index != INDEX_NONE)
		{
			GrantedAttributes[Index].MakeEditorFriendlyName();
		}
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif
