// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Common/GUIS_ListEntryDetailSection.h"


void UGUIS_ListEntryDetailSection::SetListItemObject(UObject* ListItemObject)
{
	NativeOnListItemObjectSet(ListItemObject);
}

void UGUIS_ListEntryDetailSection::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	OnListItemObjectSet(ListItemObject);
}

