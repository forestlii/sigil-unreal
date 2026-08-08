// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Bullet/GCS_BulletDefinition.h"
#include "GCS_AttackDefinition.h"
#include "UObject/Object.h"
#include "GCS_AttackRequest.generated.h"

/**
 * Base class for all attack request types.
 * 所有攻击请求类型的基类。
 */
UCLASS(Abstract, Blueprintable, BlueprintType, Const, DefaultToInstanced, EditInlineNew, meta=(DisplayName="GCS Attack Request"))
class UGCS_AttackRequest_Base : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Gets the attack definition handle.
	 * 获取攻击定义句柄。
	 * @return The attack definition handle. 攻击定义句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GCS|Attack")
	FDataTableRowHandle GetAttackDefinitionHandle() const;

	/**
	 * Gets the attack definition.
	 * 获取攻击定义。
	 * @return The attack definition. 攻击定义。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS|Attack")
	FGCS_AttackDefinition GetAttackDefinition() const;
};

/**
 * Attack request for melee attacks.
 * 近战攻击请求。
 */
UCLASS(meta=(DisplayName="GCS Attack Request (Melee)"))
class GENERICCOMBATSYSTEM_API UGCS_AttackRequest_Melee : public UGCS_AttackRequest_Base
{
	GENERATED_BODY()

public:
	/**
	 * Gets the attack definition handle.
	 * 获取攻击定义句柄。
	 * @return The attack definition handle. 攻击定义句柄。
	 */
	virtual FDataTableRowHandle GetAttackDefinitionHandle_Implementation() const override;

	/**
	 * Tags for traces activated during the notify state.
	 * 在通知状态期间激活的追踪标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	FGameplayTagContainer TracesToControl;

protected:
	/**
	 * Handle to the attack definition.
	 * 攻击定义的句柄。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack", meta=(RowType="/Script/GenericCombatSystem.GCS_AttackDefinition"))
	FDataTableRowHandle AttackDefinitionHandle;
};

/**
 * Enum for ability targeting source types.
 * 能力目标来源类型的枚举。
 */
UENUM(BlueprintType)
enum class EGCS_AbilityTargetingSourceType : uint8
{
	/**
	 * From the player's camera towards camera focus.
	 * 从玩家相机朝向相机焦点。
	 */
	CameraTowardsFocus,

	/**
	 * From the pawn's location/socket, in the pawn's orientation.
	 * 从Pawn的位置/插槽，沿Pawn的朝向。
	 */
	PawnForward,

	/**
	 * From the pawn's location/socket, oriented towards camera focus.
	 * 从Pawn的位置/插槽，朝向相机焦点。
	 */
	PawnTowardsFocus,

	/**
	 * From the weapon's location/socket, in the pawn's orientation.
	 * 从武器的位置/插槽，沿Pawn的朝向。
	 */
	WeaponForward,

	/**
	 * From the weapon's location/socket, towards camera focus.
	 * 从武器的位置/插槽，朝向相机焦点。
	 */
	WeaponTowardsFocus,

	/**
	 * Custom targeting, requires overriding GetTargetingTransform.
	 * 自定义目标，需重写GetTargetingTransform。
	 */
	Custom
};

/**
 * Attack request for firing bullets.
 * 发射子弹的攻击请求。
 */
UCLASS(Blueprintable, BlueprintType, Const, EditInlineNew, meta=(DisplayName="GCS Attack Request (Bullet)"))
class GENERICCOMBATSYSTEM_API UGCS_AttackRequest_Bullet : public UGCS_AttackRequest_Base
{
	GENERATED_BODY()

public:
	/**
	 * Gets the bullet definition.
	 * 获取子弹定义。
	 * @return The bullet definition. 子弹定义。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Attack")
	FGCS_BulletDefinition GetBulletDefinition() const;

	/**
	 * Gets the attack definition handle.
	 * 获取攻击定义句柄。
	 * @return The attack definition handle. 攻击定义句柄。
	 */
	virtual FDataTableRowHandle GetAttackDefinitionHandle_Implementation() const override;

	/**
	 * Gets the targeting transform for the attack.
	 * 获取攻击的目标变换。
	 * @param SourcePawn The source pawn. 来源Pawn。
	 * @param Source The targeting source type. 目标来源类型。
	 * @return The targeting transform. 目标变换。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "GCS|Attack")
	FTransform GetTargetingTransform(APawn* SourcePawn, EGCS_AbilityTargetingSourceType Source) const;

	/**
	 * Gets the weapon targeting source location.
	 * 获取武器目标来源位置。
	 * @param SourcePawn The source pawn. 来源Pawn。
	 * @return The weapon source location. 武器来源位置。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "GCS|Attack")
	FVector GetWeaponTargetingSourceLocation(APawn* SourcePawn) const;

	/**
	 * Gets the pawn targeting source location.
	 * 获取Pawn目标来源位置。
	 * @param SourcePawn The source pawn. 来源Pawn。
	 * @return The pawn source location. Pawn来源位置。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "GCS|Attack")
	FVector GetPawnTargetingSourceLocation(APawn* SourcePawn) const;

	/**
	 * Handle to the bullet definition.
	 * 子弹定义的句柄。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta=(RowType="/Script/GenericCombatSystem.GCS_BulletDefinition"))
	FDataTableRowHandle BulletDefinitionHandle;

	/**
	 * Type of targeting source for the attack.
	 * 攻击的目标来源类型。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = Parameters)
	EGCS_AbilityTargetingSourceType TargetingSourceType{EGCS_AbilityTargetingSourceType::PawnForward};

	/**
	 * Tag name for looking up the source component.
	 * 用于查找来源组件的标签名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = Parameters, meta=(EditCondition="TargetingSourceType != EGCS_AbilityTargetingSourceType::CameraTowardsFocus"))
	FName SourceComponentLookupTagName{NAME_None};

	/**
	 * Source socket name, falls back to source location if not found.
	 * 来源插槽名称，如果未找到则回退到来源位置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = Parameters, meta=(EditCondition="TargetingSourceType != EGCS_AbilityTargetingSourceType::CameraTowardsFocus"))
	FName SourceSocketName{NAME_None};

	/**
	 * Weapon socket name, falls back to source location if not found.
	 * 武器插槽名称，如果未找到则回退到来源位置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = Parameters, meta=(EditCondition="TargetingSourceType != EGCS_AbilityTargetingSourceType::CameraTowardsFocus"))
	FName SourceWeaponSocketName{NAME_None};

	/**
	 * Additional offset to the source location.
	 * 来源位置的附加偏移。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = Parameters)
	FVector LocationOffset{FVector::Zero()};

	/**
	 * Whether targeting is required for the attack.
	 * 攻击是否需要目标。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters)
	bool bRequireTargeting{false};

	/**
	 * Targeting preset for the attack.
	 * 攻击的目标预设。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters)
	TObjectPtr<UTargetingPreset> TargetingPreset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters)
	bool bShowDebugLine = false;
};
