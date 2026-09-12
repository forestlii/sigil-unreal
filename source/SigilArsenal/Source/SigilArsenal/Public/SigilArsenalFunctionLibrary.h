// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilCombatStructLibrary.h"
#include "SigilArsenalFunctionLibrary.generated.h"

class UGameplayAbility;
class USigilWeaponEquipmentInstance;

/**
 * Helpers to reach the active weapon loadout from the pawn, from an ability or from a weapon actor.
 * 从 Pawn、技能或武器 Actor 找到当前武器装载的辅助函数。
 */
UCLASS()
class SIGILARSENAL_API USigilArsenalFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * A slot query that matches every slot (sigil.inventory treats an empty query as "match nothing").
	 * 匹配所有槽位的查询（sigil.inventory 把空查询当作"什么都不匹配"）。
	 */
	UFUNCTION(BlueprintPure, Category = "Arsenal")
	static FGameplayTagQuery MakeAnySlotQuery();

	/**
	 * The active weapon equipment on the actor's equipment system. An empty SlotQuery means any slot.
	 * Actor 装备系统上当前激活的武器装备。SlotQuery 为空表示任意槽位。
	 */
	UFUNCTION(BlueprintPure, Category = "Arsenal", meta = (DefaultToSelf = "Actor", AutoCreateRefTerm = "SlotQuery"))
	static USigilWeaponEquipmentInstance* GetActiveWeaponEquipment(const AActor* Actor, const FGameplayTagQuery& SlotQuery);

	/** The active weapon's spawned weapon actor, or null. 当前激活武器生成的武器 Actor，无则为 null。 */
	UFUNCTION(BlueprintPure, Category = "Arsenal", meta = (DefaultToSelf = "Actor", AutoCreateRefTerm = "SlotQuery"))
	static AActor* GetActiveWeaponActor(const AActor* Actor, const FGameplayTagQuery& SlotQuery);

	/**
	 * Selects the active weapon's actions for the ability tags: the drop-in implementation for
	 * ISigilCombatInterface::QueryAbilityActions on a pawn whose montages come from its weapon.
	 * 按技能标签选出当前激活武器的动作——蒙太奇来自武器的 Pawn 可直接用它实现 ISigilCombatInterface::QueryAbilityActions。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Arsenal", meta = (DefaultToSelf = "Actor", AutoCreateRefTerm = "SourceTags,TargetTags,SlotQuery", ExpandBoolAsExecs = "ReturnValue"))
	static bool QueryActiveWeaponAbilityActions(const AActor* Actor, const FGameplayTagContainer& AbilityTags, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags,
	                                            TArray<FSigilAbilityAction>& Actions, const FGameplayTagQuery& SlotQuery);

	/**
	 * The weapon equipment that granted the ability (its SourceObject), or null.
	 * 授予该技能的武器装备（其 SourceObject），无则为 null。
	 */
	UFUNCTION(BlueprintPure, Category = "Arsenal", meta = (DefaultToSelf = "Ability"))
	static USigilWeaponEquipmentInstance* GetWeaponEquipmentOfAbility(const UGameplayAbility* Ability);

	/**
	 * The weapon equipment behind a spawned weapon actor: its ISigilWeaponInterface SourceObject, falling back to the owner's
	 * equipment system lookup.
	 * 武器 Actor 背后的武器装备：先取其 ISigilWeaponInterface SourceObject，再回落到拥有者装备系统的查找。
	 */
	UFUNCTION(BlueprintPure, Category = "Arsenal", meta = (DefaultToSelf = "WeaponActor"))
	static USigilWeaponEquipmentInstance* GetWeaponEquipmentOfWeaponActor(AActor* WeaponActor);
};
