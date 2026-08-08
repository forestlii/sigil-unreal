// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GMS_SettingEnumLibrary.generated.h"

UENUM(BlueprintType)
enum class EGMS_InAirRotationMode : uint8
{
	RotateToVelocityOnJump,
	KeepRelativeRotation,
	KeepWorldRotation
};


UENUM(BlueprintType)
enum class EGMS_TurnInPlacePlayMethod :uint8
{
	//Trigger turn in place in animation graph
	Graph,
	//Trigger turn in place as dynamic slot montage.
	Montage,
};

UENUM(BlueprintType)
enum class EGMS_OverlayPlayMode :uint8
{
	SequencePlayer,
	SequenceEvaluator,
};

UENUM(BlueprintType)
enum class EGMS_LayeredBoneBlendMode : uint8
{
	BranchFilter,
	BlendMask,
};


UENUM(BlueprintType)
enum class EGMS_VelocityDirectionMode :uint8
{
	OrientToLastVelocityDirection,
	OrientToInputDirection,
	TurningCircle
};

UENUM(BlueprintType)
enum class EGMS_ViewDirectionMode :uint8
{
	Default,
	Aiming
};