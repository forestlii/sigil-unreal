// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SigilContextEffectsEnumLibrary.generated.h"

/**
 * Enum defining the load state of a context effects library.
 * 定义情景效果库加载状态的枚举。
 */
UENUM()
enum class ESigilContextEffectsLibraryLoadState : uint8
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
enum class ESigilEffectsContextType : uint8
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
/**
 * Which camera perspective a context effects notify plays for.
 * 情景效果通知在哪种视角下播放。
 */
UENUM(BlueprintType)
enum class ESigilContextEffectsPerspectiveFilter : uint8
{
	/** Always play (default; previous behaviour). 总是播放（默认，与原行为一致）。 */
	Any,

	/**
	 * Only when the owning pawn is locally controlled and reports first person through ISigilViewPerspectiveInterface.
	 * Use on first-person body / arms animations.
	 * 仅当拥有者 Pawn 由本地控制且通过 ISigilViewPerspectiveInterface 汇报第一人称时播放。用于第一人称身体 / 手臂动画。
	 */
	FirstPersonOnly,

	/**
	 * Everyone except the locally controlled first-person viewer: simulated pawns, other players, and the local player
	 * while in third person. Use on world body animations.
	 * 除"本地控制且处于第一人称的观看者"以外都播放：模拟 Pawn、其他玩家、以及第三人称下的本地玩家。用于世界身体动画。
	 */
	ThirdPersonOnly
};
