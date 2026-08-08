// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Drops/GIS_ItemDropperComponent.h"
#include "GIS_InventoryTags.h"
#include "GameFramework/Actor.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"
#include "GIS_LogChannels.h"
#include "Pickups/GIS_InventoryPickupComponent.h"
#include "Pickups/GIS_ItemPickupComponent.h"
#include "Pickups/GIS_WorldItemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemDropperComponent)

void UGIS_ItemDropperComponent::Drop()
{
	TArray<FGIS_ItemInfo> ItemsToDrop = GetItemsToDrop();
	
	DropItemsInternal(ItemsToDrop);
}

void UGIS_ItemDropperComponent::BeginPlay()
{
	if (!CollectionTag.IsValid())
	{
		CollectionTag = GIS_CollectionTags::Main;
	}

	Super::BeginPlay();
}

TArray<FGIS_ItemInfo> UGIS_ItemDropperComponent::GetItemsToDrop() const
{
	return GetItemsToDropInternal();
}

TArray<FGIS_ItemInfo> UGIS_ItemDropperComponent::GetItemsToDropInternal() const
{
	TArray<FGIS_ItemInfo> Items;

	UGIS_InventorySystemComponent* Inventory = UGIS_InventorySystemComponent::FindInventorySystemComponent(GetOwner());
	if (Inventory == nullptr)
	{
		GIS_CLOG(Error, "requires inventory system component to drop items.")
		return Items;
	}
	UGIS_ItemCollection* Collection = Inventory->GetCollectionByTag(CollectionTag);
	if (Collection == nullptr)
	{
		GIS_CLOG(Error, " inventory missing collection with tag:%s'", *CollectionTag.ToString())
		return Items;
	}

	Items = Collection->GetAllItemInfos();
	return Items;
}

void UGIS_ItemDropperComponent::DropItemsInternal(const TArray<FGIS_ItemInfo>& ItemInfos)
{
	if (bDropAsInventory)
	{
		DropInventoryPickup(ItemInfos);
	}
	else
	{
		for (int32 i = 0; i < ItemInfos.Num(); i++)
		{
			DropItemPickup(ItemInfos[i]);
		}
	}
}

void UGIS_ItemDropperComponent::DropInventoryPickup(const TArray<FGIS_ItemInfo>& ItemInfos)
{
	if (AActor* PickupActor = CreatePickupActorInstance())
	{
		UGIS_InventorySystemComponent* Inventory = PickupActor->FindComponentByClass<UGIS_InventorySystemComponent>();
		UGIS_InventoryPickupComponent* Pickup = PickupActor->FindComponentByClass<UGIS_InventoryPickupComponent>();
		if (Inventory == nullptr || Pickup == nullptr)
		{
			GIS_CLOG(Error, "Spawned pickup(%s) missing either inventory component or inventory pickup component.", *PickupActor->GetName());
			return;
		}

		UGIS_ItemCollection* Collection = Inventory->GetDefaultCollection();
		if (Collection == nullptr)
		{
			GIS_CLOG(Error, "Spawned pickup(%s)'s inventory doesn't have default collection.", *PickupActor->GetName());
			return;
		}
		Collection->RemoveAll();
		Collection->AddItems(ItemInfos);
	}
}

void UGIS_ItemDropperComponent::DropItemPickup(const FGIS_ItemInfo& ItemInfo)
{
	if (AActor* Pickup = CreatePickupActorInstance())
	{
		UGIS_ItemPickupComponent* ItemPickup = Pickup->FindComponentByClass<UGIS_ItemPickupComponent>();
		UGIS_WorldItemComponent* WorldItem = Pickup->FindComponentByClass<UGIS_WorldItemComponent>();
		if (ItemPickup == nullptr || WorldItem == nullptr)
		{
			GIS_CLOG(Error, "Spawned pickup(%s) missing either ItemPickup component or WorldItem component.", *Pickup->GetName());
		}
		WorldItem->SetItemInfo(ItemInfo.Item, ItemInfo.Amount);
	}
}
