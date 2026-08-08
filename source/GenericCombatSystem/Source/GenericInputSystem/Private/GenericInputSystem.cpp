// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GenericInputSystem.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "GIPS_GameplayDebugger.h"
#endif // WITH_GAMEPLAY_DEBUGGER

#define LOCTEXT_NAMESPACE "FGenericInputSystemModule"

void FGenericInputSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("GenericInputSystem", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGIPS_GameplayDebuggerCategory_Input::MakeInstance));
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif // WITH_GAMEPLAY_DEBUGGER
}

void FGenericInputSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("GenericInputSystem");
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif // WITH_GAMEPLAY_DEBUGGER
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGenericInputSystemModule, GenericInputSystem)