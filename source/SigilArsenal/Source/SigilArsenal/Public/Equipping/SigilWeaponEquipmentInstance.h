// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/SigilAbilitySet.h"
#include "Abilities/SigilAbilitySourceInterface.h"
#include "Equipping/SigilEquipmentInstance.h"
#include "GameplayTagContainer.h"
#include "SigilCombatStructLibrary.h"
#include "SigilWeaponEquipmentInstance.generated.h"

class UAbilitySystemComponent;
class UAnimInstance;
class USigilAbilityActionSetSettings;
class USigilItemFragment_WeaponLoadout;
class USkeletalMeshComponent;

/**
 * Equipment instance for items carrying a USigilItemFragment_WeaponLoadout. It is the bridge between sigil.inventory and
 * GAS / sigil.combat that the base packages deliberately leave out:
 *  - grants the loadout's ability set to the owning pawn (SourceObject = this instance) and revokes it on unequip;
 *  - reports IsAbilitySourceActive so USigilGameplayAbility::bRequireSourceObjectActive gates abilities to the active weapon;
 *  - links / unlinks the loadout's animation layer on the owner's main mesh when the weapon activates / deactivates;
 *  - exposes the per-weapon ability action set (montage table) and the spawned weapon actor;
 *  - sets itself as ISigilWeaponInterface::SourceObject on spawned weapon actors.
 * 携带 USigilItemFragment_WeaponLoadout 的物品所用的装备实例，是基础包刻意留白的"库存 ↔ GAS / 战斗"桥：
 * 授予 / 收回技能集（SourceObject = 本实例）；汇报 IsAbilitySourceActive 供 bRequireSourceObjectActive 门控；
 * 激活 / 失活时链接 / 解链动画层；暴露按武器的动作集与武器 Actor；把自己设为武器 Actor 的 SourceObject。
 */
UCLASS(BlueprintType, Blueprintable)
class SIGILARSENAL_API USigilWeaponEquipmentInstance : public USigilEquipmentInstance, public ISigilAbilitySourceInterface
{
	GENERATED_BODY()

public:
	//~ISigilEquipmentInterface
	virtual void OnEquipmentBeginPlay_Implementation() override;
	virtual void OnEquipmentEndPlay_Implementation() override;
	virtual void OnActiveStateChanged_Implementation(bool bNewActiveState) override;
	//~End of ISigilEquipmentInterface

	//~ISigilAbilitySourceInterface
	virtual bool IsAbilitySourceActive_Implementation() const override;
	//~End of ISigilAbilitySourceInterface

	/** The loadout fragment of the source item, or null. 源物品的装载片段，无则为 null。 */
	UFUNCTION(BlueprintPure, Category = "Arsenal|Weapon")
	const USigilItemFragment_WeaponLoadout* GetLoadout() const { return CachedLoadout; }

	/** The resolved ability action set of the loadout, or null. 装载的动作集，无则为 null。 */
	UFUNCTION(BlueprintPure, Category = "Arsenal|Weapon")
	const USigilAbilityActionSetSettings* GetAbilityActionSet() const { return CachedAbilityActionSet; }

	/**
	 * Selects this weapon's actions for the given ability tags (see USigilAbilityActionSetSettings::SelectBestAbilityActions).
	 * 按技能标签选出本武器的动作（见 USigilAbilityActionSetSettings::SelectBestAbilityActions）。
	 * @return False if the loadout has no action set or nothing matched. 无动作集或无匹配则返回 false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Arsenal|Weapon", meta = (AutoCreateRefTerm = "SourceTags,TargetTags"))
	bool QueryAbilityActions(const FGameplayTagContainer& AbilityTags, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, TArray<FSigilAbilityAction>& Actions) const;

	/** First spawned equipment actor implementing ISigilWeaponInterface, or null. 第一个实现 ISigilWeaponInterface 的装备 Actor。 */
	UFUNCTION(BlueprintPure, Category = "Arsenal|Weapon")
	AActor* GetWeaponActor() const;

	/** True while the loadout's ability set is granted to the owner. 装载的技能集当前已授予拥有者时为 true。 */
	UFUNCTION(BlueprintPure, Category = "Arsenal|Weapon")
	bool AreAbilitiesGranted() const { return bAbilitiesGranted; }

	/** Animation layer currently linked by this weapon, or null. 本武器当前链接的动画层，无则为 null。 */
	UFUNCTION(BlueprintPure, Category = "Arsenal|Weapon")
	TSubclassOf<UAnimInstance> GetLinkedAnimLayerClass() const { return LinkedAnimLayerClass; }

	/** Ability system the loadout is granted to (owning pawn, then its player state). 装载授予的技能系统（先 Pawn，再其 PlayerState）。 */
	UFUNCTION(BlueprintPure, Category = "Arsenal|Weapon")
	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;

protected:
	//~USigilEquipmentInstance
	virtual void SetupEquipmentActors_Implementation(const TArray<AActor*>& InActors) const override;
	//~End of USigilEquipmentInstance

	/** Resolves the ability system component the loadout targets. Override for custom owner layouts. */
	virtual UAbilitySystemComponent* ResolveAbilitySystemComponent() const;

	/** Resolves the mesh the animation layer links to. Override for custom mesh layouts. */
	virtual USkeletalMeshComponent* ResolveAnimLayerMesh() const;

	/** Caches the loadout fragment and its soft references from the source item. */
	void CacheLoadout();

	void GrantAbilities();
	void RevokeAbilities();
	void LinkAnimLayer();
	void UnlinkAnimLayer();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Arsenal|Weapon")
	TObjectPtr<const USigilItemFragment_WeaponLoadout> CachedLoadout;

	UPROPERTY(Transient)
	TObjectPtr<const USigilAbilitySet> CachedAbilitySet;

	UPROPERTY(Transient)
	TObjectPtr<const USigilAbilityActionSetSettings> CachedAbilityActionSet;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> CachedAnimLayerClass;

	/** Handles of everything the ability set granted (server only). 技能集授予的全部句柄（仅服务器）。 */
	UPROPERTY(Transient)
	FSigilAbilitySet_GrantedHandles GrantedHandles;

	UPROPERTY(Transient)
	bool bAbilitiesGranted = false;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> LinkedAnimLayerClass;

	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> LinkedAnimLayerMesh;
};
