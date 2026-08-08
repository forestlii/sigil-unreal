// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Pickups/GIS_InventoryPickupComponent.h"
#include "GameFramework/Actor.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_InventoryTags.h"
#include "GIS_ItemCollection.h"
#include "GIS_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_InventoryPickupComponent)

void UGIS_InventoryPickupComponent::BeginPlay()
{
	if (!CollectionTag.IsValid())
	{
		CollectionTag = GIS_CollectionTags::Main;
	}

	Inventory = UGIS_InventorySystemComponent::FindInventorySystemComponent(GetOwner());
	if (!Inventory)
	{
		GIS_CLOG(Warning, "InventoryPickup requries an inventory system component on the same actor!")
	}

	Super::BeginPlay();
}

bool UGIS_InventoryPickupComponent::Pickup(UGIS_InventorySystemComponent* Picker)
{
	if (!GetOwner()->HasAuthority())
	{
		GIS_CLOG(Warning, "has no authority!");
		return false;
	}
	if (Inventory == nullptr || !IsValid(Inventory))
	{
		GIS_CLOG(Warning, "doesn't have an inventory system component to function.")
		return false;
	}
	if (!CollectionTag.IsValid() || !IsValid(Picker))
	{
		GIS_CLOG(Warning, "doesn't have valid picker to function.")
		return false;
	}

	UGIS_ItemCollection* DestCollection = Picker->GetCollectionByTag(CollectionTag);
	if (DestCollection == nullptr)
	{
		GIS_CLOG(Warning, "picker(%s) doesn't have valid collection named:%s", *Picker->GetOwner()->GetName(), *CollectionTag.ToString());
		return false;
	}

	return AddPickupToCollection(DestCollection);
}

UGIS_InventorySystemComponent* UGIS_InventoryPickupComponent::GetOwningInventory() const
{
	return Inventory;
}

bool UGIS_InventoryPickupComponent::AddPickupToCollection(UGIS_ItemCollection* DestCollection)
{
	TArray<FGIS_ItemInfo> PickupItems = Inventory->GetDefaultCollection()->GetAllItemInfos();
	bool bAtLeastOneCanBeAdded = false;
	for (int32 i = 0; i < PickupItems.Num(); i++)
	{
		FGIS_ItemInfo ItemInfo = PickupItems[i];
		FGIS_ItemInfo CanAddedItemInfo;
		if (DestCollection->CanAddItem(ItemInfo, CanAddedItemInfo))
		{
			if (CanAddedItemInfo.Amount != 0)
			{
				bAtLeastOneCanBeAdded = true;
			}
		}
	}
	if (bAtLeastOneCanBeAdded == false)
	{
		NotifyPickupFailed();
		return false;
	}

	for (int32 i = 0; i < PickupItems.Num(); i++)
	{
		DestCollection->AddItem(PickupItems[i]);
	}
	Inventory->GetDefaultCollection()->RemoveAll();
	NotifyPickupSuccess();
	return true;
}
