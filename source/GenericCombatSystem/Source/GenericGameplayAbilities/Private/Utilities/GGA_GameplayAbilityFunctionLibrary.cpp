// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utilities/GGA_GameplayAbilityFunctionLibrary.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Abilities/GameplayAbility.h"

bool UGGA_GameplayAbilityFunctionLibrary::IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle)
{
	return Handle.IsValid();
}

const UGameplayAbility* UGGA_GameplayAbilityFunctionLibrary::GetAbilityCDOFromClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (IsValid(AbilityClass))
	{
		return AbilityClass->GetDefaultObject<UGameplayAbility>();
	}
	return nullptr;
}

FGameplayAbilitySpecHandle UGGA_GameplayAbilityFunctionLibrary::GetCurrentAbilitySpecHandle(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->GetCurrentAbilitySpecHandle() : FGameplayAbilitySpecHandle();
}

bool UGGA_GameplayAbilityFunctionLibrary::IsAbilityActive(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->IsActive() : false;
}

EGameplayAbilityReplicationPolicy::Type UGGA_GameplayAbilityFunctionLibrary::GetReplicationPolicy(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->GetReplicationPolicy() : EGameplayAbilityReplicationPolicy::ReplicateNo;
}

EGameplayAbilityInstancingPolicy::Type UGGA_GameplayAbilityFunctionLibrary::GetInstancingPolicy(const UGameplayAbility* Ability)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	return IsValid(Ability) ? Ability->GetInstancingPolicy() : EGameplayAbilityInstancingPolicy::NonInstanced;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FGameplayTagContainer UGGA_GameplayAbilityFunctionLibrary::GetAbilityTags(const UGameplayAbility* Ability)
{
#if ENGINE_MINOR_VERSION > 4
	return IsValid(Ability) ? Ability->GetAssetTags() : FGameplayTagContainer::EmptyContainer;
#else
	return IsValid(Ability) ? Ability->AbilityTags : FGameplayTagContainer::EmptyContainer;
#endif
}

bool UGGA_GameplayAbilityFunctionLibrary::IsPredictingClient(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->IsPredictingClient() : false;
}

bool UGGA_GameplayAbilityFunctionLibrary::IsForRemoteClient(const UGameplayAbility* Ability)
{
	return IsValid(Ability) ? Ability->IsForRemoteClient() : false;
}

bool UGGA_GameplayAbilityFunctionLibrary::HasAuthorityOrPredictionKey(const UGameplayAbility* Ability)
{
	if (IsValid(Ability))
	{
		const FGameplayAbilityActivationInfo& ActivationInfo = Ability->GetCurrentActivationInfo();
		return Ability->HasAuthorityOrPredictionKey(Ability->GetCurrentActorInfo(), &ActivationInfo);
	}
	return IsValid(Ability) ? Ability->IsForRemoteClient() : false;
}
