// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIS_CollectionContainer.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"
#include "GIS_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CollectionContainer)


bool FGIS_CollectionEntry::IsValidEntry() const
{
	return Id.IsValid() && IsValid(Instance) && IsValid(Definition);
}

FGIS_CollectionContainer::FGIS_CollectionContainer(UGIS_InventorySystemComponent* InInventory)
{
	OwningComponent = InInventory;
}

void FGIS_CollectionContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		const FGIS_CollectionEntry& Entry = Entries[Index];

		// already in the list.
		if (Entry.IsValidEntry() && OwningComponent->CollectionIdToInstanceMap.Contains(Entry.Instance->GetCollectionId()))
		{
			OwningComponent->OnCollectionRemoved(Entry);
		}
		else if (OwningComponent->PendingCollections.Contains(Entry.Id))
		{
			GIS_OWNED_CLOG(OwningComponent, Warning, "Discard pending collection(%s).", *GetNameSafe(OwningComponent->PendingCollections[Entry.Id].Definition))
			OwningComponent->PendingCollections.Remove(Entry.Id);
		}
	}
}

void FGIS_CollectionContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FGIS_CollectionEntry& Entry = Entries[Index];

		if (OwningComponent->GetOwner() && Entry.IsValidEntry())
		{
			OwningComponent->OnCollectionAdded(Entry);
		}
		else
		{
			OwningComponent->PendingCollections.Add(Entry.Id, Entry);
		}
	}
}

void FGIS_CollectionContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FGIS_CollectionEntry& Entry = Entries[Index];

		if (Entry.IsValidEntry() && OwningComponent->CollectionIdToInstanceMap.Contains(Entry.Instance->GetCollectionId())) //Already Added.
		{
			OwningComponent->OnCollectionUpdated(Entry);
		}
		else if (OwningComponent->PendingCollections.Contains(Entry.Id)) //In pending list.
		{
			OwningComponent->PendingCollections.Emplace(Entry.Id, Entry); // Updated to pending.
		}
		else
		{
			OwningComponent->PendingCollections.Add(Entry.Id, Entry); //Add to pending list.
		}
	}
}
