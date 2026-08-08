// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilGas.h"
#include "Misc/Paths.h"
#include "GameplayTagsManager.h"

#define LOCTEXT_NAMESPACE "FGGameplayAbilitiesModule"


void FSigilGasModule::StartupModule()
{
}

void FSigilGasModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSigilGasModule, SigilGas)
