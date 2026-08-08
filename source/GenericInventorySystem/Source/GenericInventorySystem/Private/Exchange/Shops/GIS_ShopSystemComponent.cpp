// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Exchange/Shops/GIS_ShopSystemComponent.h"
#include "UObject/Object.h"
#include "GameFramework/Actor.h"
#include "GIS_CurrencySystemComponent.h"
#include "GIS_InventoryFunctionLibrary.h"
#include "GIS_InventorySubsystem.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"
#include "GIS_ItemDefinition.h"
#include "GIS_ItemFragment_Shoppable.h"
#include "Items/GIS_ItemInstance.h"
#include "GIS_LogChannels.h"
#include "Exchange/Shops/GIS_ShopCondition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ShopSystemComponent)


UGIS_ShopSystemComponent::UGIS_ShopSystemComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
}

UGIS_ShopSystemComponent* UGIS_ShopSystemComponent::GetShopSystemComponent(const AActor* Actor)
{
	return IsValid(Actor) ? Actor->FindComponentByClass<UGIS_ShopSystemComponent>() : nullptr;
}

UGIS_InventorySystemComponent* UGIS_ShopSystemComponent::GetInventory() const
{
	return OwningInventory;
}

bool UGIS_ShopSystemComponent::BuyItem(UGIS_InventorySystemComponent* BuyerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo)
{
	return BuyItemInternal(BuyerInventory, CurrencySystem, ItemInfo);
}

bool UGIS_ShopSystemComponent::SellItem(UGIS_InventorySystemComponent* SellerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo)
{
	return SellItemInternal(SellerInventory, CurrencySystem, ItemInfo);
}

bool UGIS_ShopSystemComponent::CanBuyerBuyItem(UGIS_InventorySystemComponent* BuyerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo) const
{
	UGIS_ItemCollection* TargetCollection = BuyerInventory->GetCollectionByTag(TargetItemCollectionToAddOnBuy);
	if (TargetCollection == nullptr)
	{
		GIS_CLOG(Warning, "buyer's inventory missing collection named:%s", *TargetItemCollectionToAddOnBuy.ToString());
		return false;
	}

	FGIS_ItemInfo CanAddItemInfo;
	if (!TargetCollection->CanAddItem(ItemInfo, CanAddItemInfo))
	{
		GIS_CLOG(Warning, "buyer's collection can't add this item(%s)", *ItemInfo.GetDebugString());
		return false;
	}

	for (int i = 0; i < BuyConditions.Num(); i++)
	{
		if (BuyConditions[i]->CanBuy(this, BuyerInventory, CurrencySystem, ItemInfo)) { continue; }
		GIS_CLOG(Warning, "buy collection(%s) reject buying item(%s)", *BuyConditions[i].GetObject()->GetClass()->GetName(), *ItemInfo.GetDebugString());
		return false;
	}

	return CanBuyerBuyItemInternal(BuyerInventory, CurrencySystem, ItemInfo);
}

bool UGIS_ShopSystemComponent::CanSellerSellItem(UGIS_InventorySystemComponent* SellerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo) const
{
	for (int i = 0; i < SellConditions.Num(); i++)
	{
		if (SellConditions[i]->CanSell(this, SellerInventory, CurrencySystem, ItemInfo)) { continue; }
		GIS_CLOG(Warning, "sell collection(%s) reject selling item(%s)", *SellConditions[i].GetObject()->GetClass()->GetName(), *ItemInfo.GetDebugString());
		return false;
	}
	return CanSellerSellItemInternal(SellerInventory, CurrencySystem, ItemInfo);
}

bool UGIS_ShopSystemComponent::IsItemBuyable(const FGIS_ItemInfo& ItemInfo) const
{
	if (OwningInventory == nullptr) { return false; }

	if (!ItemInfo.IsValid())
	{
		GIS_CLOG(Warning, "invalid item to buy.");
		return false;
	}

	UGIS_ItemCollection* ItemCollection = ItemInfo.Item->GetOwningCollection();
	if (ItemCollection == nullptr) { ItemCollection = OwningInventory->GetDefaultCollection(); }

	if (!ItemCollection->HasItem(ItemInfo.Item, 1))
	{
		GIS_CLOG(Warning, "shop's inventory doesn't have item:%s", *ItemInfo.Item->GetDefinition()->GetName());
		return false;
	}
	const UGIS_ItemFragment_Shoppable* Shoppable = ItemInfo.Item->FindFragmentByClass<UGIS_ItemFragment_Shoppable>();
	if (Shoppable == nullptr)
	{
		GIS_CLOG(Warning, "item(%s) is not buyable, missing Shoppable fragment!", *ItemInfo.GetDebugString());
		return false;
	}
	if (Shoppable->BuyCurrencyAmounts.IsEmpty())
	{
		GIS_CLOG(Warning, "item(%s) is not buyable, missing BuyCurrencyAmounts in shoppable fragment!", *ItemInfo.GetDebugString());
		return false;
	}
	return true;
}

bool UGIS_ShopSystemComponent::IsItemSellable(const FGIS_ItemInfo& ItemInfo) const
{
	if (!ItemInfo.IsValid())
	{
		GIS_CLOG(Warning, "invalid item to sell.");
		return false;
	}

	const UGIS_ItemFragment_Shoppable* Shoppable = ItemInfo.Item->FindFragmentByClass<UGIS_ItemFragment_Shoppable>();

	if (Shoppable == nullptr)
	{
		GIS_CLOG(Warning, "item(%s) is not sellable, missing Shoppable fragment!", *ItemInfo.GetDebugString());
		return false;
	}

	if (Shoppable->SellCurrencyAmounts.IsEmpty())
	{
		GIS_CLOG(Warning, "item(%s) is not sellable, missing SellCurrencyAmounts in shoppable fragment!", *ItemInfo.GetDebugString());
		return false;
	}
	return true;
}

float UGIS_ShopSystemComponent::GetBuyModifierForBuyer_Implementation(UGIS_InventorySystemComponent* BuyerInventory) const
{
	return 1 + BuyPriceModifier;
}

// float UGIS_ShopSystemComponent::GetBuyModifierForItem(UGIS_InventorySystemComponent* BuyerInventory, FGIS_ItemInfo ItemInfo) const
// {
// 	return 1;
// }

float UGIS_ShopSystemComponent::GetSellModifierForSeller_Implementation(UGIS_InventorySystemComponent* SellerInventory) const
{
	return 1 + SellPriceModifer;
}

// float UGIS_ShopSystemComponent::GetSellModifierForItem(UGIS_InventorySystemComponent* SellerInventory, const FGIS_ItemInfo& ItemInfo) const
// {
// 	return 1;
// }

bool UGIS_ShopSystemComponent::TryGetBuyValueForBuyer_Implementation(UGIS_InventorySystemComponent* Buyer, const FGIS_ItemInfo& ItemInfo, TArray<FGIS_CurrencyEntry>& BuyValue) const
{
	if (!IsValid(ItemInfo.Item))
	{
		GIS_CLOG(Warning, "invalid item to buy.");
		return false;
	}
	const UGIS_ItemFragment_Shoppable* Shoppable = ItemInfo.Item->FindFragmentByClass<UGIS_ItemFragment_Shoppable>();
	if (!IsValid(Shoppable))
	{
		GIS_CLOG(Warning, "missing Shoppable fragment for item:%s!", *GetNameSafe(ItemInfo.Item->GetDefinition()));
		return false;
	}

	float Modifier = GetBuyModifierForBuyer(Buyer);
	BuyValue = UGIS_InventoryFunctionLibrary::MultiplyCurrencies(Shoppable->BuyCurrencyAmounts, Modifier * ItemInfo.Amount);

	return BuyValue.IsEmpty() == false;
}

bool UGIS_ShopSystemComponent::TryGetSellValueForSeller_Implementation(UGIS_InventorySystemComponent* Seller, const FGIS_ItemInfo& ItemInfo, TArray<FGIS_CurrencyEntry>& SellValue) const
{
	if (!IsValid(ItemInfo.Item))
	{
		GIS_CLOG(Warning, "invalid item to sell.");
		return false;
	}

	const UGIS_ItemFragment_Shoppable* Shoppable = ItemInfo.Item->FindFragmentByClass<UGIS_ItemFragment_Shoppable>();
	if (!IsValid(Shoppable))
	{
		GIS_CLOG(Warning, "missing Shoppable fragment for item:%s!", *GetNameSafe(ItemInfo.Item->GetDefinition()));
		return false;
	}

	float Modifier = GetSellModifierForSeller(Seller);

	SellValue = UGIS_InventoryFunctionLibrary::MultiplyCurrencies(Shoppable->SellCurrencyAmounts, Modifier * ItemInfo.Amount);

	return SellValue.IsEmpty() == false;
}

void UGIS_ShopSystemComponent::BeginPlay()
{
	OwningInventory = UGIS_InventorySystemComponent::FindInventorySystemComponent(GetOwner());
	if (!OwningInventory)
	{
		GIS_CLOG(Error, "Requires inventory system component!");
	}
	{
		TArray<UActorComponent*> Components = GetOwner()->GetComponentsByInterface(UGIS_ShopBuyCondition::StaticClass());
		BuyConditions.Empty();
		for (const auto Component : Components)
		{
			BuyConditions.Add(Component);
		}
	}

	{
		TArray<UActorComponent*> Components = GetOwner()->GetComponentsByInterface(UGIS_ShopSellCondition::StaticClass());
		SellConditions.Empty();
		for (const auto Component : Components)
		{
			SellConditions.Add(Component);
		}
	}

	Super::BeginPlay();
}

void UGIS_ShopSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

bool UGIS_ShopSystemComponent::CanSellerSellItemInternal_Implementation(UGIS_InventorySystemComponent* SellerInventory, UGIS_CurrencySystemComponent* CurrencySystem,
                                                                        const FGIS_ItemInfo& ItemInfo) const
{
	UGIS_ItemCollection* ItemCollection = ItemInfo.ItemCollection;
	if (ItemCollection == nullptr || ItemCollection->GetOwningInventory() == SellerInventory)
	{
		ItemCollection = SellerInventory->GetDefaultCollection();
	}

	if (ItemCollection == nullptr)
	{
		GIS_CLOG(Warning, "seller:%s doesn't have valid default collection.", *GetNameSafe(SellerInventory));
		return false;
	}

	//atleast has one.
	if (!ItemCollection->HasItem(ItemInfo.Item, 1))
	{
		return false;
	}

	return true;
}

bool UGIS_ShopSystemComponent::CanBuyerBuyItemInternal_Implementation(UGIS_InventorySystemComponent* BuyerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo) const
{
	TArray<FGIS_CurrencyEntry> BuyPrice;
	if (TryGetBuyValueForBuyer(BuyerInventory, ItemInfo, BuyPrice))
	{
		return CurrencySystem->HasCurrencies(BuyPrice);
	}
	return false;
}

bool UGIS_ShopSystemComponent::SellItemInternal_Implementation(UGIS_InventorySystemComponent* Seller, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo)
{
	if (!IsValid(Seller) || !ItemInfo.IsValid() || !IsValid(CurrencySystem))
	{
		GIS_CLOG(Warning, "passed invalid parameters!");
		return false;
	}
	if (!IsItemSellable(ItemInfo))
	{
		GIS_CLOG(Warning, "item:%s is not sellable", *ItemInfo.GetDebugString());
		return false;
	}
	if (!CanSellerSellItem(Seller, CurrencySystem, ItemInfo))
	{
		GIS_CLOG(Warning, "seller can sell this item:%s", *ItemInfo.GetDebugString());
		return false;
	}
	UGIS_ItemCollection* ItemCollection = ItemInfo.ItemCollection;
	if (ItemCollection == nullptr || ItemCollection->GetOwningInventory() == Seller)
	{
		ItemCollection = Seller->GetDefaultCollection();
	}
	if (ItemCollection->RemoveItem(ItemInfo).Amount != ItemInfo.Amount)
	{
		GIS_CLOG(Error, "Failed to remove item(%s) from inventory!", *ItemInfo.GetDebugString());
		return false;
	}

	TArray<FGIS_CurrencyEntry> SellCurrencyAmount;
	if (!TryGetSellValueForSeller(Seller, ItemInfo, SellCurrencyAmount))
	{
		GIS_CLOG(Error, "can't get sell value for item:%s", *ItemInfo.GetDebugString());
		return false;
	}

	CurrencySystem->AddCurrencies(SellCurrencyAmount);

	OwningInventory->AddItem(ItemInfo);
	return true;
}

bool UGIS_ShopSystemComponent::BuyItemInternal_Implementation(UGIS_InventorySystemComponent* BuyerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo)
{
	if (!IsValid(BuyerInventory) || !ItemInfo.IsValid() || !IsValid(CurrencySystem))
	{
		return false;
	}
	if (!IsItemBuyable(ItemInfo))
	{
		return false;
	}
	if (!CanBuyerBuyItem(BuyerInventory, CurrencySystem, ItemInfo))
	{
		return false;
	}

	UGIS_ItemCollection* TargetCollection = BuyerInventory->GetCollectionByTag(TargetItemCollectionToAddOnBuy);
	if (TargetCollection == nullptr)
	{
		return false;
	}

	//remove currency from buyer's currency system.
	TArray<FGIS_CurrencyEntry> BuyPrice;
	if (TryGetBuyValueForBuyer(BuyerInventory, ItemInfo, BuyPrice))
	{
		CurrencySystem->RemoveCurrencies(BuyPrice);
	}
	else
	{
		return false;
	}

	if (ItemInfo.Item->IsUnique())
	{
		for (int32 i = 0; i < ItemInfo.Amount; ++i)
		{
			UGIS_ItemInstance* NewItem = UGIS_InventorySubsystem::Get(this)->CreateItem(BuyerInventory->GetOwner(), ItemInfo.Item->GetDefinition());
			TargetCollection->AddItem(NewItem, 1);
		}
	}
	else
	{
		TargetCollection->AddItem(ItemInfo);
	}

	// Remove item from shop's inventory.
	OwningInventory->RemoveItem(ItemInfo);

	// Add currency to shop's currency system.
	if (UGIS_CurrencySystemComponent* OwningCurrencySystem = UGIS_CurrencySystemComponent::GetCurrencySystemComponent(GetOwner()))
	{
		OwningCurrencySystem->AddCurrencies(BuyPrice);
	}

	return true;
}
