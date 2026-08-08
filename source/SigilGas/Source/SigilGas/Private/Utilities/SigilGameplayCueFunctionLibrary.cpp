// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utilities/SigilGameplayCueFunctionLibrary.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"

void USigilGameplayCueFunctionLibrary::ExecuteGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		Actor, GameplayCueTag, EGameplayCueEvent::Type::Executed,
		GameplayCueParameters);
}

void USigilGameplayCueFunctionLibrary::AddGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(Actor, GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(Actor, GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void USigilGameplayCueFunctionLibrary::RemoveGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
	Actor, GameplayCueTag, EGameplayCueEvent::Type::Removed,
	GameplayCueParameters);
}
