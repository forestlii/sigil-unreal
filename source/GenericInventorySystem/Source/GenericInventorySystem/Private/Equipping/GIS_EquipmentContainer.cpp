// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIS_EquipmentContainer.h"
#include "GIS_EquipmentInstance.h"
#include "GIS_EquipmentSystemComponent.h"
#include "GIS_LogChannels.h"
#include "Items/GIS_ItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_EquipmentContainer)

FString FGIS_EquipmentEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s"), *GetNameSafe(Instance));
}

bool FGIS_EquipmentEntry::IsValid() const
{
	return Instance != nullptr && ItemInstance != nullptr && EquippedSlot.IsValid();
}

void FGIS_EquipmentContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FGIS_EquipmentEntry& Entry = Entries[Index];

		// already in the list.
		if (OwningComponent->SlotToIdxMap.Contains(Entry.EquippedSlot))
		{
			OwningComponent->OnEquipmentEntryRemoved(Entry, Index);
		}
		else if (OwningComponent->PendingEquipmentEntries.Contains(Index))
		{
			GIS_OWNED_CLOG(OwningComponent, Warning, "Discard pending equipment(%s).", *OwningComponent->PendingEquipmentEntries[Index].GetDebugString())
			OwningComponent->PendingEquipmentEntries.Remove(Index);
		}
		Entry.bPrevActive = Entry.bActive;
	}
}

void FGIS_EquipmentContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FGIS_EquipmentEntry& Entry = Entries[Index];
		if (OwningComponent && OwningComponent->GetOwner() && OwningComponent->IsEquipmentSystemInitialized() && Entry.IsValid())
		{
			OwningComponent->OnEquipmentEntryAdded(Entry, Index);
		}
		else
		{
			OwningComponent->PendingEquipmentEntries.Add(Index, Entry);
		}
		Entry.bPrevActive = Entry.bActive;
	}
}

void FGIS_EquipmentContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FGIS_EquipmentEntry& Entry = Entries[Index];
		if (OwningComponent->SlotToIdxMap.Contains(Entry.EquippedSlot)) //Already Added.
		{
			OwningComponent->OnEquipmentEntryChanged(Entry, Index);
		}
		else if (OwningComponent->PendingEquipmentEntries.Contains(Index)) //In pending list.
		{
			OwningComponent->PendingEquipmentEntries.Emplace(Index, Entry);
		}
		else
		{
			OwningComponent->PendingEquipmentEntries.Add(Index, Entry); //Add to pending list.
		}
		Entry.bPrevActive = Entry.bActive;
	}
}

int32 FGIS_EquipmentContainer::IndexOfBySlot(const FGameplayTag& Slot) const
{
	if (!Slot.IsValid())
	{
		return INDEX_NONE;
	}
	return Entries.IndexOfByPredicate([Slot](const FGIS_EquipmentEntry& Entry)
	{
		return Entry.EquippedSlot == Slot;
	});
}

int32 FGIS_EquipmentContainer::IndexOfByItem(const UGIS_ItemInstance* Item) const
{
	if (!IsValid(Item))
	{
		return INDEX_NONE;
	}
	return Entries.IndexOfByPredicate([Item](const FGIS_EquipmentEntry& Entry)
	{
		return Entry.ItemInstance && Entry.ItemInstance == Item;
	});
}

int32 FGIS_EquipmentContainer::IndexOfByItemId(const FGuid& ItemId) const
{
	if (!ItemId.IsValid())
	{
		return INDEX_NONE;
	}
	return Entries.IndexOfByPredicate([ItemId](const FGIS_EquipmentEntry& Entry)
	{
		return Entry.ItemInstance && Entry.ItemInstance->GetItemId() == ItemId;
	});
}
