// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemRestriction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemRestriction)

bool UGIS_ItemRestriction::CanAddItem(FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ReceivingCollection) const
{
	return CanAddItemInternal(ItemInfo, ReceivingCollection);
}

bool UGIS_ItemRestriction::CanRemoveItem(FGIS_ItemInfo& ItemInfo) const
{
	return CanRemoveItemInternal(ItemInfo);
}

bool UGIS_ItemRestriction::CanAddItemInternal_Implementation(FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ReceivingCollection) const
{
	return true;
}

bool UGIS_ItemRestriction::CanRemoveItemInternal_Implementation(FGIS_ItemInfo& ItemInfo) const
{
	return true;
}
