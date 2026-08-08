// Copyright 2024 RedMoonGames All Rights Reserved.

#include "GenericMovementSystem.h"

#include "GameFramework/HUD.h"

#define LOCTEXT_NAMESPACE "FGenericMovementSystemModule"

void FGenericMovementSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

}

void FGenericMovementSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGenericMovementSystemModule, GenericMovementSystem)
