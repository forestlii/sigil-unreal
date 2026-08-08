// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utilities/SigilGameplayAbilityFunctionLibrary.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Abilities/GameplayAbility.h"

bool USigilGameplayAbilityFunctionLibrary::IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle)
{
	return Handle.IsValid();
}

const UGameplayAbility* USigilGameplayAbilityFunctionLibrary::GetAbilityCDOFromClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (IsValid(AbilityClass))
	{
		return AbilityClass->GetDefaultObject<UGameplayAbility>();
	}
	return nullptr;
}

FGameplayAbilitySpecHandle USigilGameplayAbilityFunctionLibrary::GetCurrentAbilitySpecHandle(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->GetCurrentAbilitySpecHandle() : FGameplayAbilitySpecHandle();
}

bool USigilGameplayAbilityFunctionLibrary::IsAbilityActive(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->IsActive() : false;
}

EGameplayAbilityReplicationPolicy::Type USigilGameplayAbilityFunctionLibrary::GetReplicationPolicy(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->GetReplicationPolicy() : EGameplayAbilityReplicationPolicy::ReplicateNo;
}

EGameplayAbilityInstancingPolicy::Type USigilGameplayAbilityFunctionLibrary::GetInstancingPolicy(const UGameplayAbility* Ability)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	return IsValid(Ability) ? Ability->GetInstancingPolicy() : EGameplayAbilityInstancingPolicy::NonInstanced;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FGameplayTagContainer USigilGameplayAbilityFunctionLibrary::GetAbilityTags(const UGameplayAbility* Ability)
{
#if ENGINE_MINOR_VERSION > 4
	return IsValid(Ability) ? Ability->GetAssetTags() : FGameplayTagContainer::EmptyContainer;
#else
	return IsValid(Ability) ? Ability->AbilityTags : FGameplayTagContainer::EmptyContainer;
#endif
}

bool USigilGameplayAbilityFunctionLibrary::IsPredictingClient(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->IsPredictingClient() : false;
}

bool USigilGameplayAbilityFunctionLibrary::IsForRemoteClient(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->IsForRemoteClient() : false;
}

bool USigilGameplayAbilityFunctionLibrary::HasAuthorityOrPredictionKey(const UGameplayAbility* Ability)
{
	if (IsValid(Ability))
	{
		const FGameplayAbilityActivationInfo& ActivationInfo = Ability->GetCurrentActivationInfo();
		return Ability->HasAuthorityOrPredictionKey(Ability->GetCurrentActorInfo(), &ActivationInfo);
	}
	return IsValid(Ability) ? Ability->IsForRemoteClient() : false;
}
