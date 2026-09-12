[English](sigil-arsenal.md) | [简体中文](sigil-arsenal.zh-CN.md)

# sigil.arsenal

`sigil.arsenal`（插件 `SigilArsenal`）把一个背包物品变成**武器装载**：武器授予的技能、技能读取的按武器蒙太奇表、以及武器激活期间链接的动画层。它是 `sigil.inventory`（刻意零 GAS 耦合）和 `sigil.combat`（不认识背包）有意留给项目的那座桥。

**依赖：** `sigil.gas`、`sigil.inventory`、`sigil.combat`。三个基础包彼此仍然独立；只有需要"物品驱动武器"的项目才加载本包。

## 模型

```
USigilItemDefinition
 ├─ USigilItemFragment_Equippable      InstanceType = USigilWeaponEquipmentInstance，ActorsToSpawn = 武器 Actor
 └─ USigilItemFragment_WeaponLoadout   AbilitySet · GrantPolicy · AbilityActionSet · AnimLayerClass
            │ 装备进槽位（USigilEquipmentSystemComponent）
            ▼
USigilWeaponEquipmentInstance  ── 把 AbilitySet 授予 Pawn 的 ASC（SourceObject = 本实例）
            │                  ── 激活期间把 AnimLayerClass 链接到主网格
            │                  ── 用 AbilityActionSet 回答 QueryAbilityActions
            ▼
武器 Actor（ISigilWeaponInterface）   SourceObject = 装备实例
```

- **物品 → 技能。** `USigilItemFragment_WeaponLoadout::AbilitySet` 是一个 `USigilAbilitySet`。`GrantPolicy` 决定何时授予：`WhileEquipped`（默认——装备在任一槽位就授予；技能用 `USigilGameplayAbility::bRequireSourceObjectActive` 门控，只有当前激活的武器能放，换枪零延迟）或 `WhileActive`（激活时授予、失活时收回）。
- **每把武器自己的"技能 → 蒙太奇"。** `AbilityActionSet` 是 `USigilAbilityActionSetSettings`：一个"射击"技能，每把枪各自的蒙太奇。`USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions` 找到 Pawn 当前激活的武器并选出它的动作——角色的 `ISigilCombatInterface::QueryAbilityActions` 一行转发即可。
- **动画层。** 武器激活时把 `AnimLayerClass` 链接（`LinkAnimClassLayers`）到 Pawn 主网格（`USigilCombatSystemSettings::CharacterMeshLookupTag`，其次 `ACharacter::GetMesh`，可用 `AnimLayerMeshLookupTag` 覆盖），失活或卸下时解链；每台机器各自执行。
- **从技能找到武器。** 授予的 Spec 以装备实例为 SourceObject：`USigilArsenalFunctionLibrary::GetWeaponEquipmentOfAbility` 取到实例后，`GetSourceItem()`（弹药等物品态）、`GetWeaponActor()`（枪口 / Trace）、`GetAbilityActionSet()` 都是一步之遥。生成的武器 Actor 也通过 `ISigilWeaponInterface::GetSourceObject` 反指回来。
- **换枪**走背包的槽位组机制：`USigilEquipmentSystemComponent::SetGroupActiveIndex` / `CycleGroupActiveIndex`。激活与失活经 `OnActiveStateChanged` 到达装备实例。

## 快速开始

1. 创建槽位标签，以及带武器槽位组的已装备集合 `USigilItemSlotCollectionDefinition`（见 sigil.inventory 指南）。
2. 创建武器的 `USigilAbilitySet`。只能从当前武器施放的技能打开 `bRequireSourceObjectActive`。
3. 每把武器一个 `USigilAbilityActionSetSettings`，把共用的技能标签（如 `Ability.Fire`）映射到该武器的蒙太奇。
4. 创建武器的 `USigilItemDefinition`：加 **Equippable Settings**（`InstanceType` = `USigilWeaponEquipmentInstance` 或子类，`ActorsToSpawn` = 你的武器 Actor）和 **Weapon Loadout Settings**（技能集、动作集、可选动画层）。
5. 角色的 `ISigilCombatInterface::QueryAbilityActions` 调用 `USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions`。
6. **先**初始化技能系统，**再**初始化装备系统（装载在 `OnEquipmentBeginPlay` 授予；ASC 未初始化只记 Warning、不授予）。

## 弹匣成本

将 `USigilAbilityCost_ItemIntegerAttribute` 加到单发技能的 `AdditionalCosts`，并在产生射击前调用 `CommitAbility`。`Tag` 默认 `Sigil.Arsenal.Ammo.Magazine`；`Quantity` 为正整数，默认 1；`FailureTag` 默认 `Sigil.Arsenal.Ability.Fail.Ammo`。项目可以替换标签。

在 `USigilItemFragment_DynamicAttributes::InitialIntegerAttributes` 初始化弹匣数；容量放物品定义的 `StaticIntegerAttributes`，使用 `Sigil.Arsenal.Ammo.MagazineCapacity`。Cost 不限制补弹上限；换弹逻辑由消费项目负责。

Cost 从被检查的技能 Spec 的 `SourceObject` 找到武器装备，再取源物品。缺装备、物品或属性，标签无效，消耗量非正或弹药不足，均拒绝并在配置有效时返回 `FailureTag`。仅装备所属 Pawn 的 authority 可以扣弹；执行扣除时再次核对余额，避免直接调用或重复扣除变成负数。客户端只检查，不预测扣弹。

备弹、换弹技能、物品 AttributeSet 和 Tag→Attribute 映射不在本批范围内。网络回补与真实玩法【未验证】。

## 已知缺口

- 弹匣 Cost 已提供；射击节奏待补。备弹与换弹逻辑留消费项目。
- 动画层只链接一个主网格，第一人称次要网格不在范围内。
- sigil.inventory 里对已激活条目调用 `USigilEquipmentSystemComponent::SetEquipmentActiveState(slot, false)` 是无操作；请用槽位组索引 API 切换。
