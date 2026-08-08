// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utilities/GGA_GameplayCueFunctionLibrary.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"

void UGGA_GameplayCueFunctionLibrary::ExecuteGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		Actor, GameplayCueTag, EGameplayCueEvent::Type::Executed,
		GameplayCueParameters);
}

void UGGA_GameplayCueFunctionLibrary::AddGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(Actor, GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(Actor, GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void UGGA_GameplayCueFunctionLibrary::RemoveGameplayCueLocal(AActor* Actor, const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
	Actor, GameplayCueTag, EGameplayCueEvent::Type::Removed,
	GameplayCueParameters);
}
