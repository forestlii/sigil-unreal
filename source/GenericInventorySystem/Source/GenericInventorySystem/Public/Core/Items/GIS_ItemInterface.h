// // Copyright 2025 RedMoonGames All Rights Reserved.
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "GameplayTagContainer.h"
// #include "UObject/Interface.h"
// #include "GIS_ItemInterface.generated.h"
//
//
// // This class does not need to be modified.
// UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
// class GENERICINVENTORYSYSTEM_API UGIS_EnhancedItemInterface : public UInterface
// {
// 	GENERATED_BODY()
// };
//
// /**
//  * 具备数值修正的道具
//  */
// class GENERICINVENTORYSYSTEM_API IGIS_EnhancedItemInterface
// {
// 	GENERATED_BODY()
//
// public:
// 	UFUNCTION(BlueprintCallable, Category="GIS|ItemInstance")
// 	virtual TMap<FGameplayTag, float> GetAdditionAttributes() const = 0;
// 	// virtual TMap<FGameplayTag, float> GetAdditionAttributes_Implementation() const = 0;
// 	virtual void OverrideAdditionAttribute(FGameplayTag AttributeTag, float Value) = 0;
//
// 	UFUNCTION(BlueprintCallable, Category="GIS|ItemInstance")
// 	virtual TMap<FGameplayTag, float> GetMultiplierAttributes() const = 0;
// 	// virtual TMap<FGameplayTag, float> GetMultiplierAttributes_Implementation() const =0;
// 	virtual void OverrideMultiplierAttribute(FGameplayTag AttributeTag, float Value) = 0;
//
// 	/**强化等级*/
// 	UFUNCTION(BlueprintCallable, Category="GIS|ItemInstance")
// 	virtual int32 GetEnhancedLevel() const = 0;
// 	virtual void SetEnhancedLevel(int32 Value) = 0;
// 	/**最大强化次数*/
// 	UFUNCTION(BlueprintCallable, Category="GIS|ItemInstance")
// 	virtual int32 GetMaxEnhancedNumber() const = 0;
// 	virtual void SetMaxEnhancedNumber(int32 Value) = 0;
// 	/**当前强化次数*/
// 	UFUNCTION(BlueprintCallable, Category="GIS|ItemInstance")
// 	virtual int32 GetCurrentEnhancedNumber() const = 0;
// 	virtual void SetCurrentEnhancedNumber(int32 Value) = 0;
// };
