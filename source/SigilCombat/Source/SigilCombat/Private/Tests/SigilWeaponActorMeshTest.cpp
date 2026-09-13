// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UObject/Package.h"
#include "Tests/SigilCombatTestTypes.h"
#include "Weapon/SigilWeaponInterface.h"

namespace
{
struct FSigilCombatTestWorld
{
	FWorldContext* WorldContext = nullptr;
	UWorld* World = nullptr;

	FSigilCombatTestWorld()
	{
		if (!GEngine)
		{
			return;
		}
		const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("SigilCombatWeaponMeshTestWorld"), EUniqueObjectNameOptions::GloballyUnique);
		WorldContext = &GEngine->CreateNewWorldContext(EWorldType::Game);
		World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
		if (World)
		{
			World->AddToRoot();
			WorldContext->SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}
	}

	~FSigilCombatTestWorld()
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
};

UStaticMeshComponent* AddMesh(AActor* Actor, const TCHAR* Name, FName Tag)
{
	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Actor, Name);
	if (Tag != NAME_None)
	{
		Mesh->ComponentTags.Add(Tag);
	}
	Mesh->RegisterComponent();
	return Mesh;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilWeaponActorMeshTest,
	"SigilCombat.Weapon.FindsMeshOnWeaponActor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilWeaponActorMeshTest::RunTest(const FString& Parameters)
{
	FSigilCombatTestWorld Fixture;
	if (!Fixture.World)
	{
		AddError(TEXT("The test world could not be created"));
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APawn* Pawn = Fixture.World->SpawnActor<APawn>(APawn::StaticClass(), FTransform::Identity, Params);
	TestNotNull(TEXT("The pawn should spawn"), Pawn);
	if (!Pawn)
	{
		return false;
	}

	const FName WeaponMeshTag(TEXT("WeaponMesh"));
	Params.Owner = Pawn;

	// Legacy layout: the tagged mesh lives on the owning pawn and still wins.
	{
		ASigilCombatTestWeaponActor* Weapon = Fixture.World->SpawnActor<ASigilCombatTestWeaponActor>(ASigilCombatTestWeaponActor::StaticClass(), FTransform::Identity, Params);
		UStaticMeshComponent* PawnMesh = AddMesh(Pawn, TEXT("PawnBlade"), WeaponMeshTag);
		AddMesh(Weapon, TEXT("WeaponOwnMesh"), WeaponMeshTag);
		TestEqual(TEXT("A tagged mesh on the owning pawn is preferred (legacy melee layout)"), ISigilWeaponInterface::Execute_GetPrimitiveComponent(Weapon), static_cast<UPrimitiveComponent*>(PawnMesh));
		PawnMesh->DestroyComponent();
		Weapon->Destroy();
	}

	// Equipment-spawned weapon: nothing tagged on the pawn, the weapon's own tagged mesh is found.
	{
		ASigilCombatTestWeaponActor* Weapon = Fixture.World->SpawnActor<ASigilCombatTestWeaponActor>(ASigilCombatTestWeaponActor::StaticClass(), FTransform::Identity, Params);
		UStaticMeshComponent* OwnMesh = AddMesh(Weapon, TEXT("WeaponOwnMeshTagged"), WeaponMeshTag);
		TestEqual(TEXT("With nothing tagged on the pawn, the weapon's own tagged mesh is found"), ISigilWeaponInterface::Execute_GetPrimitiveComponent(Weapon), static_cast<UPrimitiveComponent*>(OwnMesh));
		Weapon->Destroy();
	}

	// No tag anywhere: the weapon's first primitive component is the fallback.
	{
		ASigilCombatTestWeaponActor* Weapon = Fixture.World->SpawnActor<ASigilCombatTestWeaponActor>(ASigilCombatTestWeaponActor::StaticClass(), FTransform::Identity, Params);
		UStaticMeshComponent* Untagged = AddMesh(Weapon, TEXT("WeaponUntaggedMesh"), NAME_None);
		TestEqual(TEXT("Without any tag the weapon's first primitive component is used"), ISigilWeaponInterface::Execute_GetPrimitiveComponent(Weapon), static_cast<UPrimitiveComponent*>(Untagged));
		Weapon->Destroy();
	}

	return true;
}

#endif
