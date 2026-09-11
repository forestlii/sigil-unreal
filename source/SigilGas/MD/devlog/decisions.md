# SigilGas 决策日志

> 本文件只记录 `SigilGas` 插件内部的设计取舍，不记录消费项目（如 ProjectSpecter）的玩法、资产或产品范围。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-09-11。
>
> 本轮（2026-09-11）条目来源：外部审计 `D:\P4\Code_UE5.6\MD\analysis\gasshooter-sigil-borrow-audit.md`（EXT-20260911-GASSHOOTER-BORROW-AUDIT-001，R2），只实施其中 Sigil 侧、不依赖枪械产品门的项。抄自 GASShooter / GASDocumentation 的代码保留 `Copyright 2020 Dan Kestranek`（MIT）署名。

### [2026-09-11] A2：技能"来源对象激活"门禁走接口，不 Cast 具体角色类

- 阶段: 迭代
- 面临的选择: 照抄 GASShooter `bSourceObjectMustEqualCurrentWeaponToActivate`（`Cast<AGSHeroCharacter>` 后比较 `GetCurrentWeapon()`），或让 SourceObject 自己汇报激活态。
- 定了什么: SigilGas 新增 `ISigilAbilitySourceInterface::IsAbilitySourceActive()`（Blueprintable）；`USigilGameplayAbility` 新增 `bRequireSourceObjectActive`（默认 false，语义不变），开启时 `CanActivateAbility` 在 `Super` 之前查 `GetSourceObject()`，SourceObject 缺失 / 未实现接口 / 汇报未激活 → 以新失败标签 `Sigil.Ability.ActivateFail.SourceObjectInactive` 拒绝。`ASigilWeaponActor`（SigilCombat）实现该接口并桥接到 `IsWeaponActive`。
- 否掉了什么 + 为什么: 否掉在 SigilGas 里引用 `ISigilWeaponInterface` / `USigilEquipmentInstance`——依赖图只允许 combat→gas，SigilInventory 与 GAS 零耦合是刻意设计（`docs/sigil-inventory.zh-CN.md`），装备实例的桥接留给消费项目在 `OnEquipmentBeginPlay` 里做。否掉"未实现接口时放行"——"require" 语义应严格，误配时宁可拒绝并给出失败标签。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 A2；GASShooter `GSGameplayAbility.h:64-66`、`.cpp:180-191`；Automation `SigilGas.Ability.RequireSourceObjectActive`。

### [2026-09-11] A7：按类 + SourceObject 查技能句柄放函数库，不放 ASC

- 阶段: 迭代
- 面临的选择: 照 GASShooter 把 `FindAbilitySpecHandleForClass` 加在 ASC 上，或放进 `USigilAbilitySystemFunctionLibrary`。
- 定了什么: 加在函数库：`FindAbilitySpecHandleForClass(ASC, AbilityClass, OptionalSourceObject)`，精确类匹配 + 可选 SourceObject 过滤，用公开的 `GetActivatableAbilities()` 遍历（5.8 已公开，无需 `ABILITYLIST_SCOPE_LOCK`），无匹配返回无效句柄。
- 否掉了什么 + 为什么: 否掉挂在 ASC 上——Sigil 现有"按 Tag / Query 找技能"全在函数库（`FindAbilityWithTags` 等），保持同一入口；否掉子类匹配（`IsChildOf`）——GS 原语义是精确类，装备授予/回收场景需要精确定位。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 A7；GASShooter `GSAbilitySystemComponent.cpp:140-156`；Automation `SigilGas.Library.FindAbilitySpecHandleForClass`。
