// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GenericGameplayAbilities.h"
#include "Misc/Paths.h"
#include "GameplayTagsManager.h"

#define LOCTEXT_NAMESPACE "FGGameplayAbilitiesModule"


void FGenericGameplayAbilitiesModule::StartupModule()
{
}

void FGenericGameplayAbilitiesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGenericGameplayAbilitiesModule, GenericGameplayAbilities)
