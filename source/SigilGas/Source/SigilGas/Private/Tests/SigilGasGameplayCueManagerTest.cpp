// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Globals/SigilGameplayCueManager.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasGameplayCueManagerTest,
	"SigilGas.CueManager.LoadsRuntimeObjectLibrariesOnDemand",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasGameplayCueManagerTest::RunTest(const FString& Parameters)
{
	const USigilGameplayCueManager* Defaults = GetDefault<USigilGameplayCueManager>();
	TestNotNull(TEXT("The cue manager CDO should exist"), Defaults);
	if (!Defaults)
	{
		return false;
	}
	TestFalse(TEXT("By default the Sigil cue manager does not async-load runtime object libraries"), Defaults->ShouldAsyncLoadRuntimeObjectLibraries());

	USigilGameplayCueManager* Manager = NewObject<USigilGameplayCueManager>(GetTransientPackage());
	TestNotNull(TEXT("A cue manager instance should be constructible"), Manager);
	if (!Manager)
	{
		return false;
	}
	TestFalse(TEXT("A fresh instance inherits the on-demand default"), Manager->ShouldAsyncLoadRuntimeObjectLibraries());

	Manager->bAsyncLoadRuntimeObjectLibraries = true;
	TestTrue(TEXT("The config flag restores the engine's up-front async load"), Manager->ShouldAsyncLoadRuntimeObjectLibraries());

	return true;
}

#endif
