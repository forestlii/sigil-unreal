# SigilInteraction 决策日志

> 本文件只记录 `SigilInteraction` 插件内部的交互合同与修复取舍。具体对话内容、输入资产和场景 Actor 属于消费项目自己的 devlog。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-08-27。

### [2026-08-26] 复用 Smart Object + GAS 交互链，不新建对话专用交互框架

- 阶段: 选型
- 面临的选择: 为对话另写一套“按键 → Actor → 对话”通道，或复用现有 Smart Object 槽位、Gameplay Behavior 与 Gameplay Ability 链路。
- 定了什么: `SigilInteraction` 继续以 Smart Object 表达发现、认领、占用和失效，以 GAS Ability / AbilityTask 执行交互行为；具体交互结果由消费项目连接。
- 否掉了什么 + 为什么: 否掉对话专用的平行交互框架；它会绕开槽位竞争、生命周期和能力门禁，后续难以扩展其他交互类型。
- 复用层🔑: ② 引擎相关
- 来源: Likeon 对“为什么不用smart object”“gas不能接入smart object吗”的追问及后续确认；`SigilInteraction.Build.cs`、`SigilAbilityTask_UseSmartObjectWithGameplayBehavior.h/.cpp`。

### [2026-08-26] 候选 Actor 未变化时也刷新交互选项

- 阶段: 迭代
- 面临的选择: 只在选中的 Actor 发生变化时刷新选项，或在同一 Actor 的 Smart Object 槽位状态变化后也重新读取选项。
- 定了什么: `OnInteractableActorsChanged` 在仍保留同一有效 Actor 时调用 `RefreshOptionsForActor`；认领前复制当前 Option，避免认领流程中的容器变化使引用失效。
- 否掉了什么 + 为什么: 否掉“Actor 相同就跳过刷新”；槽位可从 Free 变为 Claimed / Occupied，旧选项会让 UI 和认领判断过期。
- 复用层🔑: ② 引擎相关
- 来源: Sigil commit `9452ef1`；`SigilInteractionSystemComponent.cpp`、`SigilGameplayAbility_Interaction.cpp`、`SigilInteraction.System.RefreshesRetainedActor` Automation。
