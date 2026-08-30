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
		FSigilItemInfo AddableItem = PickupItem;
		AddableItem.ItemCollection = nullptr;
		FSigilItemInfo AllowedToAdd;
		if (!DestCollection->CanAddItem(AddableItem, AllowedToAdd))
		{
			continue;
		}

		FSigilItemInfo AllowedToRemove;
		if (!SourceCollection->RemoveItemCondition(PickupItem, AllowedToRemove))
		{
			continue;
		}

		const int32 RequestedAmount = FMath::Min(
			PickupItem.Amount,
			FMath::Min(AllowedToAdd.Amount, AllowedToRemove.Amount));
		if (RequestedAmount <= 0)
		{
			continue;
		}

		const int32 SourceAmountBefore = SourceCollection->GetItemAmount(AllowedToRemove.Item.Get());
		FSigilItemInfo ItemToRemove = AllowedToRemove;
		ItemToRemove.Amount = RequestedAmount;
		SourceCollection->RemoveItem(ItemToRemove);
		const int32 ActualRemoved = FMath::Clamp(
			SourceAmountBefore - SourceCollection->GetItemAmount(AllowedToRemove.Item.Get()),
			0,
			RequestedAmount);
		if (ActualRemoved <= 0)
		{
			continue;
		}

		const int32 DestinationAmountBefore = DestCollection->GetItemAmount(AllowedToAdd.Item.Get());
		FSigilItemInfo ItemToAdd = AllowedToAdd;
		ItemToAdd.Amount = ActualRemoved;
		ItemToAdd.ItemCollection = nullptr;
		DestCollection->AddItem(ItemToAdd);
		const int32 ActualAdded = FMath::Clamp(
			DestCollection->GetItemAmount(AllowedToAdd.Item.Get()) - DestinationAmountBefore,
			0,
			ActualRemoved);

		const int32 RejectedAmount = ActualRemoved - ActualAdded;
		int32 ActualRestored = 0;
		if (RejectedAmount > 0)
		{
			const int32 SourceAmountBeforeRestore = SourceCollection->GetItemAmount(AllowedToRemove.Item.Get());
			FSigilItemInfo ItemToRestore = AllowedToRemove;
			ItemToRestore.Amount = RejectedAmount;
			ItemToRestore.ItemCollection = nullptr;
			SourceCollection->AddItem(ItemToRestore);
			ActualRestored = FMath::Clamp(
				SourceCollection->GetItemAmount(AllowedToRemove.Item.Get()) - SourceAmountBeforeRestore,
				0,
				RejectedAmount);
		}

		const int32 SourceNetRemoved = ActualRemoved - ActualRestored;
		if (SourceNetRemoved == ActualAdded)
		{
			TotalAdded += ActualAdded;
		}
	}

	if (TotalAdded == 0)
	{
		NotifyPickupFailed();
		return false;
	}

	NotifyPickupSuccess();
	return true;
}
