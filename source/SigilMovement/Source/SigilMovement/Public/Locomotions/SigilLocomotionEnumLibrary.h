// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SigilLocomotionEnumLibrary.generated.h"

UENUM(BlueprintType)
enum class ESigilMovementDirection : uint8
{
	Forward,
	Backward,
	Left,
	Right
};

UENUM(BlueprintType)
enum class ESigilMovementDirection_8Way : uint8
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

