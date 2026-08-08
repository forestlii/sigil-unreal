// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemRestriction_TagRequirements.h"

#include "Items/GIS_ItemInstance.h"
#include "GIS_ItemCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemRestriction_TagRequirements)

bool UGIS_ItemRestriction_TagRequirements::CanAddItemInternal_Implementation(FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ReceivingCollection) const
{
	if (TagQuery.IsEmpty())
	{
		return true;
	}
	return TagQuery.Matches(ItemInfo.Item->GetItemTags());
}
