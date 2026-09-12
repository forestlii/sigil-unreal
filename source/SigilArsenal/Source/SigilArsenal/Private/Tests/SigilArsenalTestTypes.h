// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Abilities/SigilAbilitySet.h"
#include "Abilities/SigilGameplayAbility.h"
#include "Equipping/SigilEquipmentSystemComponent.h"
#include "GameFramework/Pawn.h"
#include "NativeGameplayTags.h"
#include "SigilAbilitySystemComponent.h"
#include "SigilInventorySystemComponent.h"
#include "Weapon/SigilWeaponActor.h"
#include "SigilArsenalTestTypes.generated.h"

class USigilItemCollectionDefinition;
class USkeletalMeshComponent;

namespace SigilArsenalTestTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemGun)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SlotWeaponGroup)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SlotPrimary)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SlotSecondary)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityFire)
}

/** Inventory component whose collection definitions can be set by a test. 可由测试设置集合定义的库存组件。 */
UCLASS(Transient)
class USigilArsenalTestInventoryComponent final : public USigilInventorySystemComponent
{
	GENERATED_BODY()

public:
	void AddCollectionDefinitionForTest(const USigilItemCollectionDefinition* Definition) { CollectionDefinitions.Add(Definition); }
};

/** Equipment component whose target collection tag can be set by a test. 可由测试设置目标集合标签的装备组件。 */
UCLASS(Transient)
class USigilArsenalTestEquipmentComponent final : public USigilEquipmentSystemComponent
{
	GENERATED_BODY()

public:
	void SetTargetCollectionTagForTest(const FGameplayTag& Tag) { TargetCollectionTag = Tag; }
};

/**
 * Pawn with a Sigil ability system, inventory + equipment components and a main skeletal mesh.
 * 带 Sigil 技能系统、库存 + 装备组件和主骨骼网格的 Pawn。
 */
UCLASS(Transient, NotPlaceable)
class ASigilArsenalTestPawn final : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASigilArsenalTestPawn();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }
	USigilAbilitySystemComponent* GetSigilAbilitySystem() const { return AbilitySystem; }
	USigilArsenalTestInventoryComponent* GetInventory() const { return Inventory; }
	USigilArsenalTestEquipmentComponent* GetEquipment() const { return Equipment; }
	USkeletalMeshComponent* GetMainMesh() const { return MainMesh; }

private:
	UPROPERTY()
	TObjectPtr<USigilAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<USigilArsenalTestInventoryComponent> Inventory;

	UPROPERTY()
	TObjectPtr<USigilArsenalTestEquipmentComponent> Equipment;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> MainMesh;
};

/**
 * Weapon-granted ability that requires its source (the weapon equipment) to be active and counts activations.
 * 要求来源（武器装备）处于激活态并计数激活次数的武器技能。
 */
UCLASS(Transient)
class USigilArsenalTestFireAbility final : public USigilGameplayAbility
{
	GENERATED_BODY()

public:
	USigilArsenalTestFireAbility();

	int32 ActivationCount = 0;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
};

/** Ability set granting the fire ability. 授予射击技能的技能集。 */
UCLASS(Transient)
class USigilArsenalTestAbilitySet final : public USigilAbilitySet
{
	GENERATED_BODY()

public:
	USigilArsenalTestAbilitySet();
};

/** Concrete weapon actor (the base class is abstract). 具体武器 Actor（基类是抽象的）。 */
UCLASS(Transient, NotPlaceable)
class ASigilArsenalTestWeaponActor final : public ASigilWeaponActor
{
	GENERATED_BODY()
};
