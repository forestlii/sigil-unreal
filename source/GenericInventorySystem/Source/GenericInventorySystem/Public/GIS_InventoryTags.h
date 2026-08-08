// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once
#include "NativeGameplayTags.h"

/**
 * Namespace for collection-related gameplay tags.
 * 集合相关游戏标签的命名空间。
 */
namespace GIS_CollectionTags
{
	/** Main inventory collection tag. 主要库存集合标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Main)
	/** Hidden inventory collection tag. 隐藏库存集合标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hidden)
	/** Equipped items collection tag. 已装备道具集合标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipped)
	/** Quick bar collection tag. 快捷栏集合标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(QuickBar);
}

/**
 * Namespace for inventory initialization state tags.
 * 库存初始化状态标签的命名空间。
 */
namespace GIS_InventoryInitState
{
	/** Tag indicating the inventory has been spawned. 指示库存已生成的标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Spawned);
	/** Tag indicating data is available for the inventory. 指示库存数据可用的标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DataAvailable);
	/** Tag indicating the inventory data is initialized. 指示库存数据已初始化的标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DataInitialized);
	/** Tag indicating the inventory is ready for gameplay. 指示库存已准备好用于游戏的标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayReady);
}

// namespace GIS_MessageTags
// {
// 	/** Tag for item stack updates. 道具堆叠更新标签。 */
// 	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemStackUpdate);
// 	/** Tag for inventory updates. 库存更新标签。 */
// 	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InventoryUpdate);
// 	/** Tag for collection updates. 集合更新标签。 */
// 	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CollectionUpdate);
// 	/** Tag for adding item information to the inventory. 添加道具信息到库存的标签。 */
// 	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InventoryAddItemInfo);
// 	/** Tag for rejected item addition to the inventory. 拒绝添加道具到库存的标签。 */
// 	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InventoryAddItemInfoRejected)
// 	/** Tag for removing item information from the inventory. 从库存移除道具信息的标签。 */
// 	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InventoryRemoveItemInfo);
// 	/** Tag for quick bar slots changes. 快捷栏槽位更改标签。 */
// 	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(QuickBarSlotsChanged);
// 	/** Tag for quick bar active index changes. 快捷栏激活索引更改标签。 */
// 	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(QuickBarActiveIndexChanged);
// }

/**
 * Namespace for attribute-related gameplay tags.
 * 属性相关游戏标签的命名空间。
 */
namespace GIS_AttributeTags
{
	/** Tag for the testing attribute. 用于测试属性的标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dummy);
	// /** Tag for the current enhancement level of an item. 道具当前强化等级标签。 */
	// GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(EnhancedLevel);
	// /** Tag for the maximum enhancement level of an item. 道具最大强化等级标签。 */
	// GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxEnhancedLevel);
	/** Tag for the stack size limit of an item. 道具堆叠数量限制标签。 */
	GENERICINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(StackSizeLimit);
}
