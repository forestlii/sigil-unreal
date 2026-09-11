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
