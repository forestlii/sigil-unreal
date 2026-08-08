// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIS_CurrencyEntry.h"
#include "GIS_InventoryTags.h"
#include "Items/GIS_ItemInfo.h"
#include "Components/ActorComponent.h"
#include "GIS_ShopSystemComponent.generated.h"

class UGIS_CurrencySystemComponent;
class IGIS_ShopSellCondition;
class IGIS_ShopBuyCondition;

/**
 * Component responsible for managing shop functionality, including buying and selling items.
 * 负责管理商店功能的组件，包括购买和出售道具。
 */
UCLASS(ClassGroup=(GIS), meta=(BlueprintSpawnableComponent))
class GENERICINVENTORYSYSTEM_API UGIS_ShopSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the shop system component.
	 * 商店系统组件的构造函数。
	 */
	UGIS_ShopSystemComponent();

	/**
	 * Gets the shop system component from an actor.
	 * 从一个演员获取商店系统组件。
	 * @param Actor The actor to query for the shop system component. 要查询商店系统组件的演员。
	 * @return The shop system component, or nullptr if not found. 商店系统组件，如果未找到则返回nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIS|ShopSystem", Meta = (DefaultToSelf="Actor"))
	static UGIS_ShopSystemComponent* GetShopSystemComponent(const AActor* Actor);

	/**
	 * Gets the shop's inventory.
	 * 获取商店的库存。
	 * @return The shop's inventory component, or nullptr if not set. 商店的库存组件，如果未设置则返回nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIS|ShopSystem")
	virtual UGIS_InventorySystemComponent* GetInventory() const;

	/**
	 * Attempts to buy an item from the shop.
	 * 尝试从商店购买道具。
	 * @param BuyerInventory The buyer's inventory component. 买家的库存组件。
	 * @param CurrencySystem The buyer's currency system component (to deduct currency). 买家的货币系统组件（用于扣除货币）。
	 * @param ItemInfo The item information to buy. 要购买的道具信息。
	 * @return True if the purchase was successful, false otherwise. 如果购买成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|ShopSystem")
	virtual bool BuyItem(UGIS_InventorySystemComponent* BuyerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo);

	/**
	 * Attempts to sell an item to the shop.
	 * 尝试向商店出售道具。
	 * @param SellerInventory The seller's inventory component. 卖家的库存组件。
	 * @param CurrencySystem The seller's currency system component (to add currency). 卖家的货币系统组件（用于添加货币）。
	 * @param ItemInfo The item information to sell. 要出售的道具信息。
	 * @return True if the sale was successful, false otherwise. 如果出售成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|ShopSystem")
	virtual bool SellItem(UGIS_InventorySystemComponent* SellerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo);

	/**
	 * Checks if a buyer can purchase an item from the shop.
	 * 检查买家是否可以从商店购买道具。
	 * @param BuyerInventory The buyer's inventory component. 买家的库存组件。
	 * @param CurrencySystem The buyer's currency system component (to check currency availability). 买家的货币系统组件（用于检查货币是否足够）。
	 * @param ItemInfo The item information to buy. 要购买的道具信息。
	 * @return True if the buyer can purchase the item, false otherwise. 如果买家可以购买道具则返回true，否则返回false。
	 * @details Used for custom checks such as content locking or other game mechanics. 可用于自定义检查，如内容锁定或其他游戏机制。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|ShopSystem")
	virtual bool CanBuyerBuyItem(UGIS_InventorySystemComponent* BuyerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo) const;

	/**
	 * Checks if a seller can sell an item to the shop.
	 * 检查卖家是否可以向商店出售道具。
	 * @param SellerInventory The seller's inventory component. 卖家的库存组件。
	 * @param CurrencySystem The seller's currency system component. 卖家的货币系统组件。
	 * @param ItemInfo The item information to sell. 要出售的道具信息。
	 * @return True if the seller can sell the item, false otherwise. 如果卖家可以出售道具则返回true，否则返回false。
	 * @details Used for custom checks such as content locking or other game mechanics. 可用于自定义检查，如内容锁定或其他游戏机制。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|ShopSystem")
	virtual bool CanSellerSellItem(UGIS_InventorySystemComponent* SellerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo) const;

	/**
	 * Checks if the specified item is available for purchase from the shop.
	 * 检查指定道具是否可从商店购买。
	 * @param ItemInfo The item information to check. 要检查的道具信息。
	 * @return True if the item is buyable, false otherwise. 如果道具可购买则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|ShopSystem")
	virtual bool IsItemBuyable(const FGIS_ItemInfo& ItemInfo) const;

	/**
	 * Checks if the shop can buy the specified item from a seller.
	 * 检查商店是否可以从卖家购买指定道具。
	 * @param ItemInfo The item information to check. 要检查的道具信息。
	 * @return True if the item is sellable, false otherwise. 如果道具可出售则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|ShopSystem")
	virtual bool IsItemSellable(const FGIS_ItemInfo& ItemInfo) const;

	/**
	 * Gets the buy price modifier for the buyer.
	 * 获取买家的购买价格浮动。
	 * @param BuyerInventory The buyer's inventory component. 买家的库存组件。
	 * @return The buy price modifier. 购买价格浮动值。
	 * @details Can be overridden to implement a dynamic pricing system based on game mechanics. 可覆写以基于游戏机制实现动态价格系统。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GIS|ShopSystem")
	float GetBuyModifierForBuyer(UGIS_InventorySystemComponent* BuyerInventory) const;

	/**
	 * Gets the sell price modifier for the seller.
	 * 获取卖家的出售价格浮动。
	 * @param SellerInventory The seller's inventory component. 卖家的库存组件。
	 * @return The sell price modifier. 出售价格浮动值。
	 * @details Can be overridden to implement a dynamic pricing system based on game mechanics, such as supply shortages. 可覆写以基于游戏机制（如缺货）实现动态价格系统。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GIS|ShopSystem")
	float GetSellModifierForSeller(UGIS_InventorySystemComponent* SellerInventory) const;

	/**
	 * Gets the buy value (currency cost) of an item for the buyer.
	 * 获取买家购买道具的货币价格。
	 * @param Buyer The buyer's inventory component. 买家的库存组件。
	 * @param ItemInfo The item information to buy. 要购买的道具信息。
	 * @param BuyValue The required currency for the purchase (output). 购买所需的货币（输出）。
	 * @return True if the buyer can afford the item, false otherwise. 如果买家买得起则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintNativeEvent, Category="GIS|ShopSystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	bool TryGetBuyValueForBuyer(UGIS_InventorySystemComponent* Buyer, const FGIS_ItemInfo& ItemInfo, TArray<FGIS_CurrencyEntry>& BuyValue) const;

	/**
	 * Gets the sell value (currency gained) of an item for the seller.
	 * 获取卖家出售道具的货币价格。
	 * @param Seller The seller's inventory component. 卖家的库存组件。
	 * @param ItemInfo The item information to sell. 要出售的道具信息。
	 * @param SellValue The currency gained from the sale (output). 出售获得的货币（输出）。
	 * @return True if the currency value is valid, false otherwise. 如果货币价格有效则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintNativeEvent, Category="GIS|ShopSystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	bool TryGetSellValueForSeller(UGIS_InventorySystemComponent* Seller, const FGIS_ItemInfo& ItemInfo, TArray<FGIS_CurrencyEntry>& SellValue) const;

	/**
	 * Called when the actor begins play.
	 * 演员开始播放时调用。
	 */
	virtual void BeginPlay() override;

	/**
	 * Called when the actor ends play.
	 * 演员结束播放时调用。
	 * @param EndPlayReason The reason for ending play. 结束播放的原因。
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/**
	 * Internal check for whether a seller can sell an item to the shop.
	 * 检查卖家是否可以向商店出售道具的内部函数。
	 * @param SellerInventory The seller's inventory component. 卖家的库存组件。
	 * @param CurrencySystem The seller's currency system component. 卖家的货币系统组件。
	 * @param ItemInfo The item information to sell. 要出售的道具信息。
	 * @return True if the seller can sell the item, false otherwise. 如果卖家可以出售道具则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GIS|ShopSystem")
	bool CanSellerSellItemInternal(UGIS_InventorySystemComponent* SellerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo) const;

	/**
	 * Internal check for whether a buyer can purchase an item from the shop.
	 * 检查买家是否可以从商店购买道具的内部函数。
	 * @param BuyerInventory The buyer's inventory component. 买家的库存组件。
	 * @param CurrencySystem The buyer's currency system component. 买家的货币系统组件。
	 * @param ItemInfo The item information to buy. 要购买的道具信息。
	 * @return True if the buyer can purchase the item, false otherwise. 如果买家可以购买道具则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GIS|ShopSystem")
	bool CanBuyerBuyItemInternal(UGIS_InventorySystemComponent* BuyerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo) const;

	/**
	 * Internal function to handle selling an item to the shop.
	 * 处理向商店出售道具的内部函数。
	 * @param SellerInventory The seller's inventory component. 卖家的库存组件。
	 * @param CurrencySystem The seller's currency system component. 卖家的货币系统组件。
	 * @param ItemInfo The item information to sell. 要出售的道具信息。
	 * @return True if the sale was successful, false otherwise. 如果出售成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GIS|ShopSystem")
	bool SellItemInternal(UGIS_InventorySystemComponent* SellerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo);

	/**
	 * Internal function to handle buying an item from the shop.
	 * 处理从商店购买道具的内部函数。
	 * @param BuyerInventory The buyer's inventory component. 买家的库存组件。
	 * @param CurrencySystem The buyer's currency system component. 买家的货币系统组件。
	 * @param ItemInfo The item information to buy. 要购买的道具信息。
	 * @return True if the purchase was successful, false otherwise. 如果购买成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GIS|ShopSystem")
	bool BuyItemInternal(UGIS_InventorySystemComponent* BuyerInventory, UGIS_CurrencySystemComponent* CurrencySystem, const FGIS_ItemInfo& ItemInfo);

	/**
	 * The target item collection to add items to when purchased.
	 * 购买时道具添加到的目标道具集合。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop", meta=(Categories="GIS.ItemCollection"))
	FGameplayTag TargetItemCollectionToAddOnBuy = GIS_CollectionTags::Main;

	/**
	 * Controls the buy price modifier for this shop.
	 * 控制此商店的购买价格浮动。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop", meta=(ClampMin=0))
	float BuyPriceModifier = 0;

	/**
	 * Controls the sell price modifier for this shop.
	 * 控制此商店的出售价格浮动。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop", meta=(ClampMin=0))
	float SellPriceModifer = 0;

	/**
	 * The inventory component associated with the shop.
	 * 与商店关联的库存组件。
	 */
	UPROPERTY()
	TObjectPtr<UGIS_InventorySystemComponent> OwningInventory;

	/**
	 * List of conditions for buying items from the shop.
	 * 商店购买道具的条件列表。
	 */
	UPROPERTY()
	TArray<TScriptInterface<IGIS_ShopBuyCondition>> BuyConditions;

	/**
	 * List of conditions for selling items to the shop.
	 * 向商店出售道具的条件列表。
	 */
	UPROPERTY()
	TArray<TScriptInterface<IGIS_ShopSellCondition>> SellConditions;
};
