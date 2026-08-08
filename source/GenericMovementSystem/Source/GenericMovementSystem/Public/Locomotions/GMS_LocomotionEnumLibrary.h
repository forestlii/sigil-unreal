// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GMS_LocomotionEnumLibrary.generated.h"

UENUM(BlueprintType)
enum class EGMS_MovementDirection : uint8
{
	Forward,
	Backward,
	Left,
	Right
};

UENUM(BlueprintType)
enum class EGMS_MovementDirection_8Way : uint8
{
	Forward,
	ForwardLeft,
	ForwardRight,
	Backward,
	BackwardLeft,
	BackwardRight,
	Left,
	Right
};

