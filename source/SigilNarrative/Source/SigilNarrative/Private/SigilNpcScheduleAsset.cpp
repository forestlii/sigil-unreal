// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilNpcScheduleAsset.h"

bool USigilNpcScheduleAsset::ResolveAtMinute(
	const int32 MinuteOfDay,
	FSigilNpcScheduleEntry& OutEntry) const
{
	OutEntry = FSigilNpcScheduleEntry();
	if (MinuteOfDay < 0 || MinuteOfDay >= 24 * 60)
	{
		return false;
	}

	const FSigilNpcScheduleEntry* CurrentEntry = nullptr;
	const FSigilNpcScheduleEntry* PreviousDayEntry = nullptr;
	for (const FSigilNpcScheduleEntry& Entry : Entries)
	{
		if (Entry.StartMinute < 0 || Entry.StartMinute >= 24 * 60)
		{
			continue;
		}

		if (!PreviousDayEntry || Entry.StartMinute > PreviousDayEntry->StartMinute)
		{
			PreviousDayEntry = &Entry;
		}

		if (Entry.StartMinute <= MinuteOfDay &&
			(!CurrentEntry || Entry.StartMinute > CurrentEntry->StartMinute))
		{
			CurrentEntry = &Entry;
		}
	}

	const FSigilNpcScheduleEntry* ResolvedEntry = CurrentEntry ? CurrentEntry : PreviousDayEntry;
	if (!ResolvedEntry)
	{
		return false;
	}

	OutEntry = *ResolvedEntry;
	return true;
}
