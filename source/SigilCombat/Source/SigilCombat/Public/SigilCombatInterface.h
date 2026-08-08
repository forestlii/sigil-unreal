// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SigilCombatStructLibrary.h"
#include "UObject/Interface.h"
#include "SigilCombatInterface.generated.h"

class USceneComponent;

/**
 * Interface for actors or components involved in combat.
 * 参与战斗的演员或组件的接口。
 * @note Use helper function "GetCombatInterface" for access.
 * @注意 使用辅助函数"GetCombatInterface"访问。
 */
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class USigilCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Combat interface for handling combat-related functionality.
 * 处理战斗相关功能的接口。
 * @note Implementing objects should group related functionality.
 * @注意 实现对象应分组相关功能。
 */
class SIGILCOMBAT_API ISigilCombatInterface
{
	GENERATED_BODY()

public:
	/**
	 * Gets the current combat target actor.
	 * 获取当前战斗目标演员。
	 * @return The combat target actor. 战斗目标演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	AActor* GetCombatTargetActor() const;

	/**
	 * Gets the current combat target object as a scene component.
	 * 获取当前战斗目标对象的场景组件。
	 * @return The combat target scene component. 战斗目标场景组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat")
	USceneComponent* GetCombatTargetObject() const;

	/**
	 * Queries ability actions based on tags.
	 * 根据标签查询能力动作。
	 * @param AbilityTags Tags for the ability. 能力标签。
	 * @param SourceTags Source tags for filtering. 来源标签。
	 * @param TargetTags Target tags for filtering. 目标标签。
	 * @param AbilityActions The matching ability actions (output). 匹配的能力动作（输出）。
	 * @return True if valid results are found. 如果找到有效结果返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat", meta=(ExpandBoolAsExecs="ReturnValue"))
	bool QueryAbilityActions(FGameplayTagContainer AbilityTags, FGameplayTagContainer SourceTags, FGameplayTagContainer TargetTags, TArray<FSigilAbilityAction>& AbilityActions);

	/**
	 * Queries a weapon based on a tag query.
	 * 根据标签查询武器。
	 * @param Query The tag query for filtering. 标签查询。
	 * @return The object implementing SigilWeaponInterface. 实现武器接口的对象。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Combat", meta=(DisplayName="Query Weapon"))
	UObject* QueryWeapon(const FGameplayTagQuery& Query) const;

	/**
	 * Checks if the block input is held.
	 * 检查是否按住防御键。
	 * @return True if block input is held. 如果按住防御键返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Input")
	bool IsHeldBlockInput() const;

	/**
	 * Sets the character's rotation mode (e.g., strafe).
	 * 设置角色的旋转模式（例如靶向移动）。
	 * @param NewRotationMode The new rotation mode. 新旋转模式。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Movement")
	void SetRotationMode(FGameplayTag NewRotationMode);

	/**
	 * Gets the current rotation mode.
	 * 获取当前旋转模式。
	 * @return The current rotation mode. 当前旋转模式。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Movement")
	FGameplayTag GetRotationMode() const;

	/**
	 * Sets the movement set (e.g., ADS, Guard).
	 * 设置运动集（例如瞄准、防御）。
	 * @param NewMovementSet The new movement set. 新运动集。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Movement")
	void SetMovementSet(FGameplayTag NewMovementSet);

	/**
	 * Gets the current movement set.
	 * 获取当前运动集。
	 * @return The current movement set. 当前运动集。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Movement")
	FGameplayTag GetMovementSet() const;

	/**
	 * Sets the movement state (e.g., walk, jog, sprint).
	 * 设置运动状态（例如走、跑、疾跑）。
	 * @param NewMovementState The new movement state. 新运动状态。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Movement")
	void SetMovementState(FGameplayTag NewMovementState);

	/**
	 * Gets the current movement state.
	 * 获取当前运动状态。
	 * @return The current movement state. 当前运动状态。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Movement")
	FGameplayTag GetMovementState() const;
	/**
	 * Gets the current movement state.
	 * 获取目标运动状态。
	 * @return The current movement state. 当前运动状态。
	 */
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="GCS|Movement")
	FGameplayTag GetDesiredMovementState() const;
	/**
	 * Initiates the death process (e.g., disable collision, drop weapons).
	 * 启动死亡流程（例如禁用碰撞、丢弃武器）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Lifecycle")
	void StartDeath();

	/**
	 * Finalizes the death process (e.g., ragdoll, destroy actor).
	 * 完成死亡流程（例如布娃娃、销毁演员）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Lifecycle")
	void FinishDeath();

	/**
	 * Checks if the character is dead.
	 * 检查角色是否死亡。
	 * @return True if the character is dead. 如果角色死亡返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Lifecycle")
	bool IsDead() const;

	/**
	 * Gets the movement input direction.
	 * 获取移动输入方向。
	 * @return The movement input direction. 移动输入方向。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Input")
	FVector GetMovementInputDirection() const;

	/**
	 * Gets the current weapon.
	 * 获取当前武器。
	 * @param Context Optional context for querying. 可选查询上下文。
	 * @return The object implementing SigilWeaponInterface. 实现武器接口的对象。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Weapon", meta=(DisplayName="Get CurrentWeapon (GCS)"))
	UObject* SigilGetWeapon(UObject* Context) const;

	/**
	 * Gets the relative transform for a mesh attached to a socket.
	 * 获取附加到插槽的网格的相对变换。
	 * @param InSkeletalMeshComponent The skeletal mesh component. 骨骼网格组件。
	 * @param StaticMesh The static mesh. 静态网格。
	 * @param SkeletalMesh The skeletal mesh. 骨骼网格。
	 * @param SocketName The socket name. 插槽名称。
	 * @param OutTransform The relative transform (output). 相对变换（输出）。
	 * @return True if transform is provided. 如果提供变换返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure=false, Category="GCS|Weapon", meta=(ExpandBoolAsExecs="ReturnValue"))
	bool SigilGetRelativeTransformToSocket(const USkeletalMeshComponent* InSkeletalMeshComponent, const UStaticMesh* StaticMesh, const USkeletalMesh* SkeletalMesh, FName SocketName,
	                                      FTransform& OutTransform) const;
};