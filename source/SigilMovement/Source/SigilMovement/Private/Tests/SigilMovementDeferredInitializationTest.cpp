// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/ScopeExit.h"
#include "NativeGameplayTags.h"
#include "SigilCharacterMovementSystemComponent.h"
#include "Tests/SigilMovementTestTypes.h"
#include "UObject/Package.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(
	SigilMovementDeferredTestSet,
	"Sigil.Movement.Set.DeferredAutomation");

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilMovementDeferredInitializationTest,
	"SigilMovement.Runtime.DeferredInitialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilMovementDeferredInitializationTest::RunTest(const FString& Parameters)
{
	const USigilCharacterMovementSystemComponent* DefaultComponent =
		GetDefault<USigilCharacterMovementSystemComponent>();
	if (!TestNotNull(TEXT("Default movement component should exist"), DefaultComponent))
	{
		return false;
	}
	TestEqual(
		TEXT("Default initialization mode should remain strict"),
		DefaultComponent->GetRuntimeInitializationMode(),
		ESigilMovementRuntimeInitializationMode::Strict);

	if (!TestNotNull(TEXT("GEngine should exist"), GEngine))
	{
		return false;
	}

	const FName WorldName = MakeUniqueObjectName(
		nullptr,
		UWorld::StaticClass(),
		TEXT("SigilMovementDeferredTestWorld"),
		EUniqueObjectNameOptions::GloballyUnique);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
	if (World)
	{
		World->AddToRoot();
		WorldContext.SetCurrentWorld(World);
	}
	ON_SCOPE_EXIT
	{
		if (World)
		{
			GEngine->ShutdownWorldNetDriver(World);
			World->DestroyWorld(true);
			World->SetPhysicsScene(nullptr);
			GEngine->DestroyWorldContext(World);
			World->RemoveFromRoot();
		}
	};

	if (!TestNotNull(TEXT("Test world should exist"), World))
	{
		return false;
	}
	World->InitializeActorsForPlay(FURL());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASigilMovementDeferredTestCharacter* Character =
		World->SpawnActor<ASigilMovementDeferredTestCharacter>(
			ASigilMovementDeferredTestCharacter::StaticClass(),
			FTransform::Identity,
			SpawnParameters);
	if (!TestNotNull(TEXT("Deferred test character should spawn"), Character))
	{
		return false;
	}
	if (!Character->HasActorBegunPlay())
	{
		Character->DispatchBeginPlay();
	}

	USigilCharacterMovementSystemComponent* Component = Character->GetMovementSystem();
	if (!TestNotNull(TEXT("Deferred movement component should exist"), Component))
	{
		return false;
	}
	TestEqual(
		TEXT("Test character should use deferred initialization"),
		Component->GetRuntimeInitializationMode(),
		ESigilMovementRuntimeInitializationMode::DeferredUntilConfigured);
	Component->SetMovementSet(SigilMovementDeferredTestSet.GetTag());
	TestEqual(
		TEXT("Deferred component should store movement set without configured definitions"),
		Component->GetMovementSet(),
		SigilMovementDeferredTestSet.GetTag());
	TestFalse(
		TEXT("Deferred component should remain inactive without locomotion configuration"),
		Component->IsConfiguredRuntimeActive());
	TestFalse(
		TEXT("Activation should fail without an animation instance while controller yaw is enabled"),
		Component->TryActivateConfiguredRuntime());
	TestFalse(
		TEXT("Failed activation should not change the active state"),
		Component->IsConfiguredRuntimeActive());
	return true;
}

#endif
