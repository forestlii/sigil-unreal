// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Abilities/SigilAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "Runtime/Launch/Resources/Version.h"
#include "AbilitySystemComponent.h"
#include "Utilities/SigilAbilitySystemFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogSigilAbilitySet)

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAbilitySet)

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
		if (!AbilitySet.IsValid())
		{
			AbilitySet.LoadSynchronous();
		}
		AbilitySet->GiveToAbilitySystem(ASC, &GrantedHandles, SourceObject);
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
