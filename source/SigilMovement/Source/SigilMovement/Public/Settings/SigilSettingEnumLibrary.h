// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SigilSettingEnumLibrary.generated.h"

UENUM(BlueprintType)
enum class ESigilInAirRotationMode : uint8
{
	RotateToVelocityOnJump,
	KeepRelativeRotation,
	KeepWorldRotation
};


UENUM(BlueprintType)
enum class ESigilTurnInPlacePlayMethod :uint8
{
	//Trigger turn in place in animation graph
	Graph,
	//Trigger turn in place as dynamic slot montage.
	Montage,
};

UENUM(BlueprintType)
enum class ESigilOverlayPlayMode :uint8
{
	SequencePlayer,
	SequenceEvaluator,
};

UENUM(BlueprintType)
enum class ESigilLayeredBoneBlendMode : uint8
{
	BranchFilter,
	BlendMask,
};


UENUM(BlueprintType)
enum class ESigilVelocityDirectionMode :uint8
{
	OrientToLastVelocityDirection,
	OrientToInputDirection,
	TurningCircle
};

UENUM(BlueprintType)
enum class ESigilViewDirectionMode :uint8
{
	Default,
	Aiming
};