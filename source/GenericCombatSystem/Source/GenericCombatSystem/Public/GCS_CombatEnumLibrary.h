// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GCS_CombatEnumLibrary.generated.h"

/**
 * Enum for basic directional inputs.
 * 基本方向输入的枚举。
 */
UENUM(BlueprintType)
enum class EGCS_Direction : uint8
{
	/**
	 * Forward direction.
	 * 前进方向。
	 */
	Forward,

	/**
	 * Backward direction.
	 * 后退方向。
	 */
	Backward,

	/**
	 * Left direction.
	 * 左方向。
	 */
	Left,

	/**
	 * Right direction.
	 * 右方向。
	 */
	Right
};

/**
 * Enum for eight-directional inputs.
 * 八方向输入的枚举。
 */
UENUM(BlueprintType)
enum class EGCS_Direction_8 : uint8
{
	/**
	 * Forward direction.
	 * 前进方向。
	 */
	Forward,

	/**
	 * Forward-left direction.
	 * 前左方向。
	 */
	Forward_Left,

	/**
	 * Forward-right direction.
	 * 前右方向。
	 */
	Forward_Right,

	/**
	 * Backward direction.
	 * 后退方向。
	 */
	Backward,

	/**
	 * Backward-left direction.
	 * 后左方向。
	 */
	Backward_Left,

	/**
	 * Backward-right direction.
	 * 后右方向。
	 */
	Backward_Right,

	/**
	 * Left direction.
	 * 左方向。
	 */
	Left,

	/**
	 * Right direction.
	 * 右方向。
	 */
	Right
};