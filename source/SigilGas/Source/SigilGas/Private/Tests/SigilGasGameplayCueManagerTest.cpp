// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemGlobals.h"
#include "GameplayAbilitiesDeveloperSettings.h"
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

	// Wiring: Host/Config/DefaultGame.ini sets GlobalGameplayCueManagerClass under [/Script/GameplayAbilities.AbilitySystemGlobals].
	// UE 5.8 reads the class from UGameplayAbilitiesDeveloperSettings, whose OverrideConfigSection maps onto that legacy
	// section; a [/Script/GameplayAbilities.GameplayAbilitiesDeveloperSettings] section is ignored (tried, assertion failed).
	const UGameplayAbilitiesDeveloperSettings* Settings = GetDefault<UGameplayAbilitiesDeveloperSettings>();
	TestNotNull(TEXT("The developer settings should exist"), Settings);
	if (Settings)
	{
		TestEqual(TEXT("Host wires GlobalGameplayCueManagerClass through GameplayAbilitiesDeveloperSettings"),
		          Settings->GlobalGameplayCueManagerClass.ToString(), FString(TEXT("/Script/SigilGas.SigilGameplayCueManager")));
	}

	UGameplayCueManager* GlobalManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	TestNotNull(TEXT("The global cue manager should exist"), GlobalManager);
	TestTrue(TEXT("The global cue manager instantiated by the engine is the Sigil subclass"), GlobalManager && GlobalManager->IsA<USigilGameplayCueManager>());
	// ShouldAsyncLoadRuntimeObjectLibraries is protected on the engine base class; query it through the Sigil subclass.
	if (const USigilGameplayCueManager* SigilGlobalManager = Cast<USigilGameplayCueManager>(GlobalManager))
	{
		TestFalse(TEXT("The live global cue manager loads runtime object libraries on demand"), SigilGlobalManager->ShouldAsyncLoadRuntimeObjectLibraries());
	}

	return true;
}

#endif
