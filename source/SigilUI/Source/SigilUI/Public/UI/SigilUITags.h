// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once
#include "NativeGameplayTags.h"

/**
 * Namespace for modal action gameplay tags.
 * 模态动作游戏标签的命名空间。
 */
namespace SigilGameModalActionTags
{
	/**
	 * Tag for "Ok" modal action.
	 * "确定"模态动作的标签。
	 */
	SIGILUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ok);

	/**
	 * Tag for "Cancel" modal action.
	 * "取消"模态动作的标签。
	 */
	SIGILUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cancel);

	/**
	 * Tag for "Yes" modal action.
	 * "是"模态动作的标签。
	 */
	SIGILUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Yes);

	/**
	 * Tag for "No" modal action.
	 * "否"模态动作的标签。
	 */
	SIGILUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(No);

	/**
	 * Tag for unknown modal action.
	 * 未知模态动作的标签。
	 */
	SIGILUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unknown);
}

/**
 * Namespace for UI layer gameplay tags.
 * UI层游戏标签的命名空间。
 */
namespace SigilGameUILayerTags
{
	/**
	 * Tag for modal UI layer.
	 * 模态UI层的标签。
	 */
	SIGILUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Modal);
}