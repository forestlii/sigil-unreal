// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIS_CurrencyContainer.h"

#include "GIS_CurrencySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CurrencyContainer)

void FGIS_CurrencyContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FGIS_CurrencyEntry& Entry = Entries[Index];
		if (OwningComponent)
		{
			OwningComponent->OnCurrencyEntryRemoved(Entry, Index);
		}
		Entry.PrevAmount = 0;
	}
}

void FGIS_CurrencyContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FGIS_CurrencyEntry& Entry = Entries[Index];
		if (OwningComponent)
		{
			OwningComponent->OnCurrencyEntryAdded(Entry, Index);
		}
		Entry.PrevAmount = Entry.Amount;
	}
}

void FGIS_CurrencyContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FGIS_CurrencyEntry& Entry = Entries[Index];
		if (OwningComponent)
		{
			OwningComponent->OnCurrencyEntryUpdated(Entry, Index, Entry.PrevAmount, Entry.Amount);
		}
		Entry.PrevAmount = Entry.Amount;
	}
}
