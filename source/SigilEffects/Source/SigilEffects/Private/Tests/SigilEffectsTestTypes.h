// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Perspective/SigilViewPerspectiveInterface.h"
#include "SigilEffectsTestTypes.generated.h"

/**
 * Pawn that reports its perspective through ISigilViewPerspectiveInterface.
 * 通过 ISigilViewPerspectiveInterface 汇报视角的 Pawn。
 */
UCLASS(Transient, NotPlaceable)
class ASigilEffectsTestPerspectivePawn final : public APawn, public ISigilViewPerspectiveInterface
{
	GENERATED_BODY()

public:
	bool bFirstPerson = false;

	virtual bool IsInFirstPersonPerspective_Implementation() const override { return bFirstPerson; }
};

/**
 * Component that reports its perspective, for pawns that delegate the answer to a component.
 * 通过组件汇报视角，用于把答案委托给组件的 Pawn。
 */
UCLASS(Transient)
class USigilEffectsTestPerspectiveComponent final : public UActorComponent, public ISigilViewPerspectiveInterface
{
	GENERATED_BODY()

public:
	bool bFirstPerson = false;

	virtual bool IsInFirstPersonPerspective_Implementation() const override { return bFirstPerson; }
};

/**
 * Plain pawn without any perspective provider.
 * 没有任何视角实现者的普通 Pawn。
 */
UCLASS(Transient, NotPlaceable)
class ASigilEffectsTestPlainPawn final : public APawn
{
	GENERATED_BODY()
};

/**
 * Concrete controller (AController is abstract) so a test can make a pawn locally controlled in a standalone world.
 * 具体控制器（AController 是抽象类），让测试能在单机世界里把 Pawn 变成本地控制。
 */
UCLASS(Transient, NotPlaceable)
class ASigilEffectsTestController final : public AController
{
	GENERATED_BODY()
};
