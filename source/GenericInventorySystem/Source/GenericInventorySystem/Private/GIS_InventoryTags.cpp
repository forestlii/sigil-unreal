// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_InventoryTags.h"

namespace GIS_CollectionTags
{
	UE_DEFINE_GAMEPLAY_TAG(Main, "GIS.Collection.Main");
	UE_DEFINE_GAMEPLAY_TAG(Equipped, "GIS.Collection.Equipped");
	UE_DEFINE_GAMEPLAY_TAG(Hidden, "GIS.Collection.Hidden");
	UE_DEFINE_GAMEPLAY_TAG(QuickBar, "GIS.Collection.QuickBar");
}

namespace GIS_InventoryInitState
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Spawned, "GIS.InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DataAvailable, "GIS.InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DataInitialized, "GIS.InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayReady, "GIS.InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");
}

// namespace GIS_MessageTags
// {
// 	UE_DEFINE_GAMEPLAY_TAG(ItemStackUpdate, "GIS.Message.StackUpdate")
// 	UE_DEFINE_GAMEPLAY_TAG(InventoryUpdate, "GIS.Message.Inventory.Update")
// 	UE_DEFINE_GAMEPLAY_TAG(CollectionUpdate, "GIS.Message.Inventory.Collection.Update")
// 	UE_DEFINE_GAMEPLAY_TAG(InventoryAddItemInfo, "GIS.Message.Inventory.AddItemInfo")
// 	UE_DEFINE_GAMEPLAY_TAG(InventoryAddItemInfoRejected, "GIS.Message.Inventory.AddItemInfo.Rejected")
// 	UE_DEFINE_GAMEPLAY_TAG(InventoryRemoveItemInfo, "GIS.Message.Inventory.RemoveItemInfo")
// 	UE_DEFINE_GAMEPLAY_TAG(QuickBarSlotsChanged, "GIS.Message.QuickBar.SlotsChanged")
// 	UE_DEFINE_GAMEPLAY_TAG(QuickBarActiveIndexChanged, "GIS.Message.QuickBar.ActiveIndexChanged")
// }

namespace GIS_AttributeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Dummy, "GIS.Attribute.Dummy");
	// UE_DEFINE_GAMEPLAY_TAG(EnhancedLevel, "GIS.Attribute.EnhancedLevel");
	// UE_DEFINE_GAMEPLAY_TAG(MaxEnhancedLevel, "GIS.Attribute.MaxEnhancedLevel");
	UE_DEFINE_GAMEPLAY_TAG(StackSizeLimit, "GIS.Attribute.StackSizeLimit");
}
