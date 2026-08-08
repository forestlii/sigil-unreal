// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Common/SigilListEntryDetailSection.h"


void USigilListEntryDetailSection::SetListItemObject(UObject* ListItemObject)
{
	NativeOnListItemObjectSet(ListItemObject);
}

void USigilListEntryDetailSection::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	OnListItemObjectSet(ListItemObject);
}

