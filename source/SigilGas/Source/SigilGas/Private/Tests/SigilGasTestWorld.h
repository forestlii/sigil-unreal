// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Tests/SigilGasTestTypes.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Creates a throwaway game world for SigilGas automation tests and tears it down on scope exit.
 * 为 SigilGas 自动化测试创建一次性游戏世界，作用域结束时销毁。
 */
struct FSigilGasTestWorld
{
	FWorldContext* WorldContext = nullptr;
	UWorld* World = nullptr;

	explicit FSigilGasTestWorld(const TCHAR* BaseName = TEXT("SigilGasTestWorld"))
	{
		if (!GEngine)
		{
			return;
		}

		const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), BaseName, EUniqueObjectNameOptions::GloballyUnique);
		WorldContext = &GEngine->CreateNewWorldContext(EWorldType::Game);
		World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
		if (World)
		{
			World->AddToRoot();
			WorldContext->SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}
	}

	~FSigilGasTestWorld()
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

	/** Spawns the test actor and initializes its ability system with itself as owner and avatar. */
	ASigilGasTestAbilityActor* SpawnAbilityActor() const
	{
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASigilGasTestAbilityActor* Actor = World->SpawnActor<ASigilGasTestAbilityActor>(ASigilGasTestAbilityActor::StaticClass(), FTransform::Identity, SpawnParameters);
		if (Actor && Actor->GetAbilitySystem())
		{
			Actor->GetAbilitySystem()->InitAbilityActorInfo(Actor, Actor);
		}
		return Actor;
	}
};

#endif
