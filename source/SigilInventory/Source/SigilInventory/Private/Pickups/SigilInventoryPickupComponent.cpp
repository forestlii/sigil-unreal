// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Pickups/SigilInventoryPickupComponent.h"
#include "GameFramework/Actor.h"
#include "SigilInventorySystemComponent.h"
#include "SigilInventoryTags.h"
#include "SigilItemCollection.h"
#include "SigilInventoryLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilInventoryPickupComponent)

void USigilInventoryPickupComponent::BeginPlay()
{
	if (!CollectionTag.IsValid())
	{
		CollectionTag = SigilCollectionTags::Main;
	}

	Inventory = USigilInventorySystemComponent::FindInventorySystemComponent(GetOwner());
	if (!Inventory)
	{
		SIGIL_INVENTORY_CLOG(Warning, "InventoryPickup requries an inventory system component on the same actor!")
	}

	Super::BeginPlay();
}

bool USigilInventoryPickupComponent::Pickup(USigilInventorySystemComponent* Picker)
{
	if (!GetOwner()->HasAuthority())
	{
		SIGIL_INVENTORY_CLOG(Warning, "has no authority!");
		return false;
	}
	if (Inventory == nullptr || !IsValid(Inventory))
	{
		SIGIL_INVENTORY_CLOG(Warning, "doesn't have an inventory system component to function.")
		return false;
	}
	if (!CollectionTag.IsValid() || !IsValid(Picker))
	{
		SIGIL_INVENTORY_CLOG(Warning, "doesn't have valid picker to function.")
		return false;
	}

	USigilItemCollection* DestCollection = Picker->GetCollectionByTag(CollectionTag);
	if (DestCollection == nullptr)
	{
		SIGIL_INVENTORY_CLOG(Warning, "picker(%s) doesn't have valid collection named:%s", *Picker->GetOwner()->GetName(), *CollectionTag.ToString());
		return false;
	}

	return AddPickupToCollection(DestCollection);
}

USigilInventorySystemComponent* USigilInventoryPickupComponent::GetOwningInventory() const
{
	return Inventory;
}

bool USigilInventoryPickupComponent::AddPickupToCollection(USigilItemCollection* DestCollection)
{
	USigilItemCollection* SourceCollection = Inventory->GetDefaultCollection();
	const TArray<FSigilItemInfo> PickupItems = SourceCollection->GetAllItemInfos();
	int32 TotalAdded = 0;
	for (const FSigilItemInfo& PickupItem : PickupItems)
	{
		FSigilItemInfo ItemToAdd = PickupItem;
		ItemToAdd.ItemCollection = nullptr;
		const FSigilItemInfo AddedItem = DestCollection->AddItem(ItemToAdd);
		if (AddedItem.Amount <= 0)
		{
			continue;
		}

		SourceCollection->RemoveItem(FSigilItemInfo(AddedItem.Amount, PickupItem));
		TotalAdded += AddedItem.Amount;
	}

	if (TotalAdded == 0)
	{
		NotifyPickupFailed();
		return false;
	}

	NotifyPickupSuccess();
	return true;
}
