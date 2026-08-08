// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GIS_DropperComponent.h"
#include "Items/GIS_ItemInfo.h"
#include "GIS_ItemDropperComponent.generated.h"


class UGIS_InventorySystemComponent;

UCLASS(ClassGroup=(GIS), meta=(BlueprintSpawnableComponent))
class GENERICINVENTORYSYSTEM_API UGIS_ItemDropperComponent : public UGIS_DropperComponent
{
	GENERATED_BODY()

public:
	/**
 	 * Get the item infos of this dropper will drops.
 	 * 获取要掉落的道具信息。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|Dropper")
	TArray<FGIS_ItemInfo> GetItemsToDrop() const;

	virtual void Drop() override;

protected:
	virtual void BeginPlay() override;

	virtual TArray<FGIS_ItemInfo> GetItemsToDropInternal() const;
	virtual void DropItemsInternal(const TArray<FGIS_ItemInfo>& ItemInfos);

	virtual void DropInventoryPickup(const TArray<FGIS_ItemInfo>& ItemInfos);
	virtual void DropItemPickup(const FGIS_ItemInfo& ItemInfo);

	/**
	 * Target collection to drop.
	 * 指定要掉落库存中的哪个集合里的道具.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dropper", meta=(Categories="GIS.Collection"))
	FGameplayTag CollectionTag;

	/**
	 * If the drops is inventory pickup or item pickups?
	 * 指定以InventoryPickup的方式掉落还是以ItemPickup
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dropper")
	bool bDropAsInventory{true};
};
