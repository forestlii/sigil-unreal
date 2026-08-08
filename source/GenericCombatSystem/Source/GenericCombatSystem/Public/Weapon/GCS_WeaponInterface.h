// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "GCS_WeaponInterface.generated.h"

class APawn;
class AActor;

/**
 * Interface for objects acting as weapons.
 * 作为武器的对象的接口。
 */
UINTERFACE()
class UGCS_WeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for weapon-related functionality.
 * 武器相关功能的接口。
 */
class GENERICCOMBATSYSTEM_API IGCS_WeaponInterface
{
	GENERATED_BODY()

public:
	/**
	 * Gets the pawn owning this weapon.
	 * 获取拥有此武器的Pawn。
	 * @return The owning pawn. 所属Pawn。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Weapon")
	APawn* GetWeaponOwner() const;

	/**
	 * Returns the actor if implemented by an actor, or the component's owner.
	 * 如果由Actor实现，返回Actor；如果由组件实现，返回组件的Owner。
	 * @return The actor. Actor。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Weapon")
	AActor* AsActor() const;

	/**
	 * Gets the optional source object associated with the weapon.
	 * 获取与武器关联的可选源对象。
	 * @return The source object (e.g., equipment or data asset). 源对象（例如装备或数据资产）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Weapon")
	UObject* GetSourceObject() const;

	/**
	 * Sets the source object for the weapon.
	 * 设置武器的源对象。
	 * @param NewSourceObject The new source object. 新源对象。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Weapon")
	void SetSourceObject(UObject* NewSourceObject);

	/**
	 * Gets the gameplay tags associated with the weapon.
	 * 获取与武器关联的游戏标签。
	 * @return The weapon's gameplay tags. 武器游戏标签。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Weapon")
	const FGameplayTagContainer GetWeaponTags() const;

	/**
	 * Sets the weapon's active state.
	 * 设置武器的激活状态。
	 * @param bNewActive The new active state. 新激活状态。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Weapon")
	void SetWeaponActive(bool bNewActive);

	/**
	 * Checks if the weapon is active.
	 * 检查武器是否激活。
	 * @return True if the weapon is active. 如果武器激活返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Weapon")
	bool IsWeaponActive() const;

	/**
	 * Gets the main primitive component of the weapon.
	 * 获取武器的主要原始组件。
	 * @return The primitive component. 原始组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Weapon")
	UPrimitiveComponent* GetPrimitiveComponent();
	virtual UPrimitiveComponent* GetPrimitiveComponent_Implementation() = 0;

	/**
	 * Gets the targeting start transform for ranged weapons.
	 * 获取远程武器的目标起始变换。
	 * @return The targeting start transform. 目标起始变换。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Weapon", meta=(DisplayName="Get Targeting Start Transform"))
	FTransform GCS_GetTargetingStartTransform() const;

	/**
	 * Toggles targeting for the weapon.
	 * 切换武器的目标状态。
	 * @param bEnable Whether to enable targeting. 是否启用目标。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Weapon", meta=(DisplayName="Toggle Targeting"))
	void GCS_ToggleTargeting(bool bEnable);
};