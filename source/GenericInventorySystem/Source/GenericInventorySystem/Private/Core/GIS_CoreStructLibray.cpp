// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_CoreStructLibray.h"

#include "Items/GIS_ItemDefinition.h"
#include "Items/GIS_ItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CoreStructLibray)


FGIS_ItemDefinitionAmount::FGIS_ItemDefinitionAmount()
{
	Definition = nullptr;
	Amount = 1;
}

FGIS_ItemDefinitionAmount::FGIS_ItemDefinitionAmount(TSoftObjectPtr<UGIS_ItemDefinition> InDefinition, int32 InAmount)
{
	Definition = InDefinition;
	Amount = InAmount;
}

bool FGIS_ItemSlotDefinition::MatchItem(const UGIS_ItemInstance* Item) const
{
	if (!IsValid(Item))
	{
		return false;
	}

	if (TagQuery.IsEmpty())
	{
		return false;
	}

	if (TagQuery.Matches(Item->GetDefinition()->ItemTags))
	{
		return true;
	}

	return false;
}
