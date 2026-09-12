// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Feedback/SigilAnimNotify_ContextEffects.h"
#include "Tests/SigilEffectsTestTypes.h"
#include "UObject/Package.h"

namespace
{
struct FSigilEffectsTestWorld
{
	FWorldContext* WorldContext = nullptr;
	UWorld* World = nullptr;

	FSigilEffectsTestWorld()
	{
		if (!GEngine)
		{
			return;
		}

		const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("SigilEffectsPerspectiveTestWorld"), EUniqueObjectNameOptions::GloballyUnique);
		WorldContext = &GEngine->CreateNewWorldContext(EWorldType::Game);
		World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
		if (World)
		{
			World->AddToRoot();
			WorldContext->SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}
	}

	~FSigilEffectsTestWorld()
	{
		if (!World || !GEngine)
		{
			return;
		}

		GEngine->ShutdownWorldNetDriver(World);
		World->DestroyWorld(true);
		World->SetPhysicsScene(nullptr);
		GEngine->DestroyWorldContext(World);
		World->RemoveFromRoot();
	}

	template <typename T>
	T* Spawn(AActor* Owner = nullptr) const
	{
		if (!World)
		{
			return nullptr;
		}
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.Owner = Owner;
		return World->SpawnActor<T>(T::StaticClass(), FTransform::Identity, Params);
	}
};

using EFilter = ESigilContextEffectsPerspectiveFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilEffectsPerspectiveFilterTableTest,
	"SigilEffects.ContextEffects.PerspectiveFilterTable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilEffectsPerspectiveFilterTableTest::RunTest(const FString& Parameters)
{
	// Any: always.
	for (const bool bLocal : {false, true})
	{
		for (const bool bProvider : {false, true})
		{
			for (const bool bFirstPerson : {false, true})
			{
				TestTrue(TEXT("Any plays in every state"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::Any, bLocal, bProvider, bFirstPerson));
			}
		}
	}

	// FirstPersonOnly: only the locally controlled viewer that reports first person.
	TestTrue(TEXT("FirstPersonOnly plays for local + provider + first person"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::FirstPersonOnly, true, true, true));
	TestFalse(TEXT("FirstPersonOnly is silent for local + provider + third person"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::FirstPersonOnly, true, true, false));
	TestFalse(TEXT("FirstPersonOnly is silent without a provider"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::FirstPersonOnly, true, false, true));
	TestFalse(TEXT("FirstPersonOnly is silent for remote / simulated pawns"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::FirstPersonOnly, false, true, true));

	// ThirdPersonOnly: the exact complement.
	TestFalse(TEXT("ThirdPersonOnly is silent for local + provider + first person"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::ThirdPersonOnly, true, true, true));
	TestTrue(TEXT("ThirdPersonOnly plays for local + provider + third person"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::ThirdPersonOnly, true, true, false));
	TestTrue(TEXT("ThirdPersonOnly plays without a provider (treated as world body)"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::ThirdPersonOnly, true, false, true));
	TestTrue(TEXT("ThirdPersonOnly plays for remote / simulated pawns"), USigilAnimNotify_ContextEffects::ShouldPlayForPerspectiveFilter(EFilter::ThirdPersonOnly, false, true, true));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilEffectsPerspectiveResolveTest,
	"SigilEffects.ContextEffects.PerspectiveResolvesThroughOwnerChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilEffectsPerspectiveResolveTest::RunTest(const FString& Parameters)
{
	FSigilEffectsTestWorld Fixture;
	if (!Fixture.World)
	{
		AddError(TEXT("The test world could not be created"));
		return false;
	}

	bool bLocal = false;
	bool bProvider = false;
	bool bFirstPerson = false;

	// Null actor: nothing resolves.
	USigilAnimNotify_ContextEffects::ResolvePerspective(nullptr, bLocal, bProvider, bFirstPerson);
	TestFalse(TEXT("Null actor is not locally controlled"), bLocal);
	TestFalse(TEXT("Null actor has no provider"), bProvider);

	// Pawn implementing the interface, not yet possessed.
	ASigilEffectsTestPerspectivePawn* Pawn = Fixture.Spawn<ASigilEffectsTestPerspectivePawn>();
	TestNotNull(TEXT("The perspective pawn should spawn"), Pawn);
	if (!Pawn)
	{
		return false;
	}
	Pawn->bFirstPerson = true;
	USigilAnimNotify_ContextEffects::ResolvePerspective(Pawn, bLocal, bProvider, bFirstPerson);
	TestFalse(TEXT("An unpossessed pawn is not locally controlled"), bLocal);
	TestTrue(TEXT("The pawn itself is found as provider"), bProvider);
	TestTrue(TEXT("The pawn's first person answer is reported"), bFirstPerson);

	// Possess with a controller: in a standalone world the pawn becomes locally controlled.
	ASigilEffectsTestController* Controller = Fixture.Spawn<ASigilEffectsTestController>();
	TestNotNull(TEXT("The controller should spawn"), Controller);
	if (!Controller)
	{
		return false;
	}
	Controller->Possess(Pawn);
	TestTrue(TEXT("The pawn reports local control once possessed in a standalone world"), Pawn->IsLocallyControlled());
	USigilAnimNotify_ContextEffects::ResolvePerspective(Pawn, bLocal, bProvider, bFirstPerson);
	TestTrue(TEXT("Local control is resolved from the pawn"), bLocal);

	// Actor owned by the pawn (weapon-like): resolves through the owner chain.
	AActor* Weapon = Fixture.Spawn<AActor>(Pawn);
	TestNotNull(TEXT("The owned actor should spawn"), Weapon);
	if (!Weapon)
	{
		return false;
	}
	USigilAnimNotify_ContextEffects::ResolvePerspective(Weapon, bLocal, bProvider, bFirstPerson);
	TestTrue(TEXT("An owned actor inherits local control from its owning pawn"), bLocal);
	TestTrue(TEXT("An owned actor finds the pawn's provider"), bProvider);
	TestTrue(TEXT("An owned actor reports the pawn's first person answer"), bFirstPerson);

	Pawn->bFirstPerson = false;
	USigilAnimNotify_ContextEffects::ResolvePerspective(Weapon, bLocal, bProvider, bFirstPerson);
	TestFalse(TEXT("Perspective changes propagate to owned actors"), bFirstPerson);

	// Plain pawn with a component provider.
	ASigilEffectsTestPlainPawn* PlainPawn = Fixture.Spawn<ASigilEffectsTestPlainPawn>();
	TestNotNull(TEXT("The plain pawn should spawn"), PlainPawn);
	if (!PlainPawn)
	{
		return false;
	}
	USigilAnimNotify_ContextEffects::ResolvePerspective(PlainPawn, bLocal, bProvider, bFirstPerson);
	TestFalse(TEXT("A pawn without any provider reports no provider"), bProvider);

	USigilEffectsTestPerspectiveComponent* Component = NewObject<USigilEffectsTestPerspectiveComponent>(PlainPawn);
	Component->RegisterComponent();
	Component->bFirstPerson = true;
	USigilAnimNotify_ContextEffects::ResolvePerspective(PlainPawn, bLocal, bProvider, bFirstPerson);
	TestTrue(TEXT("A component provider is found"), bProvider);
	TestTrue(TEXT("The component's first person answer is reported"), bFirstPerson);

	// Whole-notify decision in a game world.
	USigilAnimNotify_ContextEffects* Notify = NewObject<USigilAnimNotify_ContextEffects>(GetTransientPackage());
	Notify->PerspectiveFilter = EFilter::FirstPersonOnly;
	Pawn->bFirstPerson = true;
	TestTrue(TEXT("FirstPersonOnly plays for the possessed first person pawn"), Notify->ShouldPlayForPerspective(Pawn));
	TestTrue(TEXT("FirstPersonOnly plays for the weapon owned by that pawn"), Notify->ShouldPlayForPerspective(Weapon));
	Pawn->bFirstPerson = false;
	TestFalse(TEXT("FirstPersonOnly is silent once the pawn is in third person"), Notify->ShouldPlayForPerspective(Pawn));
	Notify->PerspectiveFilter = EFilter::ThirdPersonOnly;
	TestTrue(TEXT("ThirdPersonOnly plays once the pawn is in third person"), Notify->ShouldPlayForPerspective(Pawn));
	Notify->PerspectiveFilter = EFilter::Any;
	TestTrue(TEXT("Any always plays"), Notify->ShouldPlayForPerspective(Pawn));

	// Outside a game world (editor preview) the filter never suppresses.
	AActor* Orphan = NewObject<AActor>(GetTransientPackage());
	Notify->PerspectiveFilter = EFilter::FirstPersonOnly;
	TestTrue(TEXT("Without a game world the notify always plays"), Notify->ShouldPlayForPerspective(Orphan));

	return true;
}

#endif
