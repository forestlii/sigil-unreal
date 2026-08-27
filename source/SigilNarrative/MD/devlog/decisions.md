# SigilNarrative 决策日志

> 本文件只记录 `SigilNarrative` 插件内部的运行时和编辑器取舍。ProjectSpecter 的 Smart Object、行为树、HUD、镜头、Level Sequence 和内容资产记录在游戏工程自己的 devlog。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-08-27。

### [2026-08-24] 用数据资产表达 Dialogue / Quest / Story

- 阶段: 脚手架
- 面临的选择: 建一套项目专属叙事 Actor，或用可验证的数据资产和通用运行时状态表达三类叙事内容。
- 定了什么: Dialogue、Quest、Story 分别使用独立 DataAsset Schema；`USigilNarrativeSubsystem` 管理 Quest / Story 状态，`USigilDialogueSession` 管理单次对话会话。
- 否掉了什么 + 为什么: 否掉把三类内容压进一个巨型通用节点模型；它们的生命周期和作者语义不同，强行统一会让校验和编辑变复杂。
- 复用层🔑: ② 引擎相关
- 来源: Sigil commit `7a51824`；`SigilDialogueAsset.h`、`SigilQuestAsset.h`、`SigilStoryAsset.h`、`SigilNarrativeSubsystem.h`。

### [2026-08-24] Snapshot 只恢复状态，不重放叙事事件

- 阶段: 垂直切片
- 面临的选择: 导入存档时重新执行 Entry / Transition / Complete Event，或直接恢复已验证的运行时状态。
- 定了什么: Snapshot 导出稳定 JSON；导入先完整校验并构造临时状态，成功后原子替换，不重放 Event。
- 否掉了什么 + 为什么: 否掉导入时重放副作用；事件可能发奖励、改 Flag 或触发演出，重放会造成重复行为。
- 复用层🔑: ② 引擎相关
- 来源: `SigilNarrativeSnapshot.cpp`；SigilNarrative Snapshot Automation；Sigil runtime core 累计提交。

### [2026-08-25] NPC Schedule 只解析活动，不负责移动

- 阶段: 垂直切片
- 面临的选择: 让插件直接驱动 NPC 移动，或只提供按分钟解析当前日程项的资产能力。
- 定了什么: `USigilNpcScheduleAsset` 保存日程项并通过 `ResolveAtMinute` 返回当前活动与目标；实际寻路和动作由消费项目负责。
- 否掉了什么 + 为什么: 否掉插件依赖行为树、StateTree 或项目角色类；通用叙事数据不应绑定某一种 AI 执行方案。
- 复用层🔑: ② 引擎相关
- 来源: Sigil commit `f8a033d`；`SigilNpcScheduleAsset.h/.cpp`。

### [2026-08-25] Condition / Event 回调期间拒绝同一叙事对象重入

- 阶段: 迭代
- 面临的选择: 允许回调中递归推进同一 Dialogue / Quest / Story，或锁住当前对象并在回调后重新核对状态。
- 定了什么: 对话会话和 Subsystem 使用回调 guard；回调后重新查找并验证资产、状态和节点，不跨 Blueprint 回调持有容器元素指针。
- 否掉了什么 + 为什么: 否掉无保护重入；回调可再次调用公开 API，可能导致重复事件、状态覆盖或 `TMap` 扩容后的悬空指针。
- 复用层🔑: ② 引擎相关
- 来源: `SigilDialogueSession.cpp`、`SigilNarrativeSubsystem.h/.cpp`；相关 Automation 用例。

### [2026-08-26] 异步演出通过抽象 Host 接入

- 阶段: 迭代
- 面临的选择: 让插件直接依赖 Level Sequence 和项目相机，或只定义演出资产、句柄、结果和 Host 接口。
- 定了什么: `SigilNarrative` 提供 Presentation Definition、带 Generation 的 Handle 和 `Completed / Skipped / Cancelled / Failed` 结果；消费项目实现具体 Host。
- 否掉了什么 + 为什么: 否掉插件直接依赖 Level Sequence、输入和项目相机；这些是项目表现层，不属于通用叙事状态合同。
- 复用层🔑: ② 引擎相关
- 来源: Sigil commit `b06f26f`；`SigilNarrativePresentation.h`。

### [2026-08-26] 专用编辑器直接编辑 Runtime 资产，预览不执行副作用

- 阶段: 迭代
- 面临的选择: 继续使用默认 Details、另建一套 Editor Graph Schema，或围绕真实 Runtime 资产提供专用 Toolkit。
- 定了什么: Dialogue / Quest / Story 专用编辑器直接修改真实 DataAsset，使用事务、校验和安全预览；预览只记录 Event，不调用 `Run`。
- 否掉了什么 + 为什么: 否掉影子 Schema 和会执行真实副作用的预览；前者会产生双真源，后者可能在编辑器里改写游戏状态。
- 复用层🔑: ② 引擎相关
- 来源: Sigil commits `79f7343`、`0700b4c`、`d97293e`、`477634c`、`5de6f90`、`d4f4710`。

### [2026-08-27] NPC Schedule 专用编辑器暂缓

- 阶段: 迭代
- 面临的选择: 继续为 NPC Schedule 开发专用编辑器，或先保留 DataAsset 默认编辑方式。
- 定了什么: NPC Schedule 专用编辑器列为 `DEFERRED / PENDING`，当前不排期；现有 Runtime Asset API 保持不变。
- 否掉了什么 + 为什么: 暂不继续开发；当前切片已有日程解析与项目行为树执行路径，深入投入的即时价值不足。
- 复用层🔑: ② 引擎相关
- 来源: Likeon 2026-08-27 原话“列为待定吧，这一块不想深入做太多”。
