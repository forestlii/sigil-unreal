// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemRestriction_UniqueOnly.h"

#include "Items/GIS_ItemInstance.h"
#include "GIS_ItemCollection.h"
#include "Items/GIS_ItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemRestriction_UniqueOnly)

bool UGIS_ItemRestriction_UniqueOnly::CanAddItemInternal_Implementation(FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ReceivingCollection) const
{
	// this restriction will not modify the item info will be added, so just set it to OutItemInfo
	return ItemInfo.Item->IsUnique();
}
