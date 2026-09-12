// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fragments/SigilItemFragment.h"
#include "SigilItemFragment_WeaponLoadout.generated.h"

class UAnimInstance;
class USigilAbilitySet;
class USigilAbilityActionSetSettings;

/**
 * When the abilities of a weapon loadout are granted to the owning pawn.
 * 武器装载的技能何时授予拥有者 Pawn。
 */
UENUM(BlueprintType)
enum class ESigilWeaponAbilityGrantPolicy : uint8
{
	/**
	 * Granted while the item is equipped in any slot; activation is meant to be gated by
	 * USigilGameplayAbility::bRequireSourceObjectActive so only the active weapon's abilities can fire. Zero-latency swaps.
	 * 只要装备在任一槽位就授予；激活由 USigilGameplayAbility::bRequireSourceObjectActive 门控，只有当前激活武器的技能能放。切换零延迟。
	 */
	WhileEquipped,

	/**
	 * Granted only while the equipment is the active one in its slot group, revoked when it deactivates.
	 * 仅在装备为槽位组内当前激活项时授予，失活即收回。
	 */
	WhileActive
};

/**
 * Item fragment that turns an equippable item into a weapon loadout: the abilities it grants, the per-weapon montage table
 * its abilities read (pistol and rifle share one "fire" ability but play different montages), and the animation layer linked
 * to the owner's main mesh while the weapon is active. Pair it with a USigilItemFragment_Equippable whose InstanceType is
 * USigilWeaponEquipmentInstance (or a subclass).
 * 把可装备物品变成"武器装载"的片段：它授予的技能、技能读取的按武器蒙太奇表（手枪与步枪共用一个"射击"技能但播不同蒙太奇）、
 * 以及武器激活期间链接到拥有者主网格的动画层。需与 InstanceType 为 USigilWeaponEquipmentInstance（或其子类）的
 * USigilItemFragment_Equippable 搭配使用。
 */
UCLASS(DisplayName = "Weapon Loadout Settings", Category = "Arsenal")
class SIGILARSENAL_API USigilItemFragment_WeaponLoadout : public USigilItemFragment
{
	GENERATED_BODY()

public:
	/**
	 * Abilities / effects / attribute sets granted to the owning pawn's ability system. The granted specs use the
	 * USigilWeaponEquipmentInstance as SourceObject, so abilities can reach the item, the weapon actor and the action set.
	 * 授予拥有者 Pawn 技能系统的技能 / 效果 / 属性集。授予的 Spec 以 USigilWeaponEquipmentInstance 为 SourceObject，
	 * 技能由此能拿到物品、武器 Actor 与动作集。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Abilities)
	TSoftObjectPtr<const USigilAbilitySet> AbilitySet;

	/** When the ability set is granted. 技能集何时授予。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Abilities)
	ESigilWeaponAbilityGrantPolicy GrantPolicy = ESigilWeaponAbilityGrantPolicy::WhileEquipped;

	/**
	 * Ability tag -> montage table for this weapon (USigilAbilityActionSetSettings). Query it through
	 * USigilWeaponEquipmentInstance::QueryAbilityActions or USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions.
	 * 本武器的"技能标签 → 蒙太奇"表。通过 USigilWeaponEquipmentInstance::QueryAbilityActions 或
	 * USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions 查询。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	TSoftObjectPtr<const USigilAbilityActionSetSettings> AbilityActionSet;

	/**
	 * Animation layer class linked to the owner's main mesh while this weapon is active (unlinked when it deactivates or
	 * is unequipped). Leave empty for no layer.
	 * 武器激活期间链接到拥有者主网格的动画层类（失活或卸下时解链）。留空则不链接。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	TSoftClassPtr<UAnimInstance> AnimLayerClass;

	/**
	 * Optional component tag used to find the mesh the animation layer links to. None: the sigil.combat main mesh lookup
	 * (USigilCombatSystemSettings::CharacterMeshLookupTag, then ACharacter::GetMesh, then the first skeletal mesh).
	 * 可选：查找动画层所链接网格用的组件标签。None 则用 sigil.combat 的主网格查找规则。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	FName AnimLayerMeshLookupTag = NAME_None;
};
