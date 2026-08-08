// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GES_ContextEffectsEnumLibrary.generated.h"

/**
 * Enum defining the load state of a context effects library.
 * 定义情景效果库加载状态的枚举。
 */
UENUM()
enum class EGES_ContextEffectsLibraryLoadState : uint8
{
	/**
	 * Library is not loaded.
	 * 库未加载。
	 */
	Unloaded = 0,

	/**
	 * Library is currently loading.
	 * 库正在加载。
	 */
	Loading = 1,

	/**
	 * Library is fully loaded.
	 * 库已完全加载。
	 */
	Loaded = 2
};

/**
 * Enum defining how source context tags are applied.
 * 定义如何应用源情景标签的枚举。
 */
UENUM()
enum class EGES_EffectsContextType : uint8
{
	/**
	 * Merge source context with existing contexts.
	 * 将源情景与现有情景合并。
	 */
	Merge,

	/**
	 * Override existing contexts with source context.
	 * 使用源情景覆盖现有情景。
	 */
	Override
};