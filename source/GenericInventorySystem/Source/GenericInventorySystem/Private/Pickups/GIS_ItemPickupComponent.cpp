// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Pickups/GIS_ItemPickupComponent.h"
#include "GameFramework/Actor.h"
#include "GIS_InventoryTags.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"
#include "Items/GIS_ItemInstance.h"
#include "GIS_LogChannels.h"
#include "Pickups/GIS_WorldItemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemPickupComponent)

bool UGIS_ItemPickupComponent::Pickup(UGIS_InventorySystemComponent* Picker)
{
	if (!GetOwner()->HasAuthority())
	{
		GIS_CLOG(Warning, "has no authority!");
		return false;
	}
	if (!CollectionTag.IsValid() || !IsValid(Picker))
	{
		GIS_CLOG(Warning, "passed-in invalid picker.");
		return false;
	}
	if (WorldItemComponent == nullptr && WorldItemComponent->GetItemInstance()->IsItemValid())
	{
		GIS_CLOG(Warning, "doesn't have valid WordItem component attached or it has invalid item instance reference.");
		return false;
	}

	return TryAddToCollection(Picker);
}

UGIS_WorldItemComponent* UGIS_ItemPickupComponent::GetWorldItem() const
{
	return WorldItemComponent;
}

// Called when the game starts
void UGIS_ItemPickupComponent::BeginPlay()
{
	if (!CollectionTag.IsValid())
	{
		CollectionTag = GIS_CollectionTags::Main;
	}

	Super::BeginPlay();
	WorldItemComponent = GetOwner()->FindComponentByClass<UGIS_WorldItemComponent>();

	if (WorldItemComponent == nullptr)
	{
		GIS_CLOG(Error, "requires GIS_WorldItemComponent to function!")
	}
}

bool UGIS_ItemPickupComponent::TryAddToCollection(UGIS_InventorySystemComponent* Picker)
{
	UGIS_ItemInstance* NewItemInstance = WorldItemComponent->GetDuplicatedItemInstance(Picker->GetOwner());

	if (NewItemInstance == nullptr)
	{
		GIS_CLOG(Error, "referenced invalid item! Pickup failed!");
		NotifyPickupFailed();
		return false;
	}

	FGIS_ItemInfo NewItemInfo;
	NewItemInfo.Item = NewItemInstance;
	NewItemInfo.Amount = WorldItemComponent->GetItemAmount();
	const FGameplayTag TargetCollection = CollectionTag.IsValid() ? CollectionTag : GIS_CollectionTags::Main;
	NewItemInfo.CollectionTag = TargetCollection;

	FGIS_ItemInfo CanAddedItemInfo;
	const bool bResult = Picker->CanAddItem(NewItemInfo, CanAddedItemInfo);

	if (!bResult || CanAddedItemInfo.Amount == 0 || (bFailIfFullAmountNotFit && CanAddedItemInfo.Amount != NewItemInfo.Amount))
	{
		NotifyPickupFailed();
		return false;
	}

	Picker->AddItem(NewItemInfo);
	NotifyPickupSuccess();
	return true;
}
