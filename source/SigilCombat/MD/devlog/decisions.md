# SigilCombat 决策日志

> 本文件只记录 `SigilCombat` 插件内部的设计取舍，不记录消费项目的玩法、资产或产品范围。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-09-11。

### [2026-09-11] 武器 Actor 桥接 SigilGas 的"技能来源激活"接口

- 阶段: 迭代
- 面临的选择: 让 SigilGas 直接认识 `ISigilWeaponInterface::IsWeaponActive`，或由 SigilCombat 的 `ASigilWeaponActor` 实现 SigilGas 定义的通用接口。
- 定了什么: `ASigilWeaponActor` 额外实现 `ISigilAbilitySourceInterface`，`IsAbilitySourceActive` 直接返回 `Execute_IsWeaponActive(this)`。以武器 Actor 为 SourceObject 授予的技能可打开 `USigilGameplayAbility::bRequireSourceObjectActive`。
- 否掉了什么 + 为什么: 否掉反向依赖（gas→combat）；依赖方向只能 combat→gas。
- 复用层🔑: ② 引擎相关
- 来源: 审计 `gasshooter-sigil-borrow-audit.md` §2 A2；`SigilWeaponActor.h/.cpp`；门禁本体的测试在 SigilGas（`SigilGas.Ability.RequireSourceObjectActive`）。

### [2026-09-13] 武器 Actor 找网格：拥有者优先，再回落到自身

- 阶段: 迭代
- 面临的选择: `ASigilWeaponActor::GetPrimitiveComponent` 原来只在 **Owner（Pawn）** 身上按 `WeaponMeshTagName` 找组件（审计 §4.1-1 指出的坑）——装备系统生成、自带 Mesh 的武器 Actor（枪）永远找不到自己的网格。选项：改成只找自身（改语义）、或保持 Owner 优先再回落自身。
- 定了什么: 保持 Owner 优先（既有近战布局不变），找不到再按标签找本 Actor，再取本 Actor 第一个 `UPrimitiveComponent`；三处都没有才 Warning。纯增量，旧配置行为不变。
- 否掉了什么 + 为什么: 否掉"自身优先"——若 Pawn 与武器上恰好都有同名标签组件，会悄悄改掉现有近战武器的 Trace 宿主。
- 复用层🔑: ② 引擎相关
- 来源: 审计 `gasshooter-sigil-borrow-audit.md` §4.1-1；`SigilWeaponActor.cpp`；Automation `SigilCombat.Weapon.FindsMeshOnWeaponActor`。
