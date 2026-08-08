// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIS_PickupComponent.h"
#include "GIS_CurrencyPickupComponent.generated.h"

class UGIS_CurrencySystemComponent;

/**
 * Component for picking up currencies into the currency system.
 * 用于将货币拾取到货币系统的组件。
 */
UCLASS(ClassGroup=(GIS), meta=(BlueprintSpawnableComponent))
class GENERICINVENTORYSYSTEM_API UGIS_CurrencyPickupComponent : public UGIS_PickupComponent
{
	GENERATED_BODY()

public:
	/**
	 * Called when the game starts to initialize the component.
	 * 游戏开始时调用以初始化组件。
	 */
	virtual void BeginPlay() override;

	/**
	 * Performs the pickup logic, adding currencies to the picker's currency system.
	 * 执行拾取逻辑，将货币添加到拾取者的货币系统。
	 * @param Picker The inventory system component of the actor performing the pickup. 执行拾取的演员的库存系统组件。
	 * @return True if the pickup was successful, false otherwise. 如果拾取成功则返回true，否则返回false。
	 */
	virtual bool Pickup(UGIS_InventorySystemComponent* Picker) override;

protected:
	/**
	 * The currency system component associated with this pickup.
	 * 与此拾取关联的货币系统组件。
	 */
	UPROPERTY()
	UGIS_CurrencySystemComponent* OwningCurrencySystem;
};