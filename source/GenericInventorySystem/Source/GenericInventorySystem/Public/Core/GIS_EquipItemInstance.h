// // Copyright 2025 RedMoonGames All Rights Reserved.
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "Items/GIS_ItemInstance.h"
// #include "Items/GIS_ItemInterface.h"
// #include "GIS_EquipItemInstance.generated.h"
//
//
// struct FGIS_EquipmentActorToSpawn;
// /**
//  * An equipment instance is a UObject tasked with managing the internal logic of the equipment.
//  * 装备实例是一个UObject，负责装备的内部逻辑
//  * @deprecated Using GIS_EquipmentInstance 弃用了，请使用GIS_EquipmentInstance.
//  */
// UCLASS(NotBlueprintable, HideDropdown)
// class GENERICINVENTORYSYSTEM_API UGIS_EquipItemInstance : public UGIS_ItemInstance, public IGIS_EnhancedItemInterface
// {
// 	GENERATED_BODY()
//
// public:
// 	virtual TMap<FGameplayTag, float> GetAdditionAttributes() const override;
// 	virtual void OverrideAdditionAttribute(FGameplayTag AttributeTag, float Value) override;
//
// 	virtual TMap<FGameplayTag, float> GetMultiplierAttributes() const override;
// 	virtual void OverrideMultiplierAttribute(FGameplayTag AttributeTag, float Value) override;
//
// 	virtual int32 GetEnhancedLevel() const override;
// 	virtual int32 GetMaxEnhancedNumber() const override;
// 	virtual int32 GetCurrentEnhancedNumber() const override;
// 	virtual void SetEnhancedLevel(int32 Value) override;
// 	virtual void SetMaxEnhancedNumber(int32 Value) override;
// 	virtual void SetCurrentEnhancedNumber(int32 Value) override;
//
// 	virtual float GetFloatAttribute(FGameplayTag AttributeTag) const override;
//
// protected:
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="ItemInstance")
// 	TMap<FGameplayTag, float> AdditionAttributes;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="ItemInstance")
// 	TMap<FGameplayTag, float> MultiplierAttributes;
//
// 	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="ItemInstance")
// 	int32 EnhancedLevel;
// 	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="ItemInstance")
// 	int32 MaxEnhancedNumber;
// 	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="ItemInstance")
// 	int32 CurrentEnhancedNumber;
// };
