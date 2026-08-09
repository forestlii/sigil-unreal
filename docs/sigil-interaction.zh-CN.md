[English](sigil-interaction.md) | [简体中文](sigil-interaction.zh-CN.md)

# sigil.interaction

**插件：** `SigilInteraction` · **模块：** `SigilInteraction`（Runtime） · **依赖：** SmartObjects、GameplayBehaviors、GameplayBehaviorSmartObjects、GameplayAbilities、TargetingSystem、ModularGameplay（引擎插件）

sigil.interaction 是一套以 SmartObject 为核心、桥接 Gameplay Behaviors 与 GAS 的交互系统。它不给每个可交互 Actor 写交互代码，而是把"交互入口"作为数据挂在 SmartObject 槽位定义里；玩家侧由一个组件统一管理候选列表、当前选中目标和同步下来的交互选项。真正的执行走 GAS，有两条路径可选——两条路径之间的串联需要项目用蓝图自己完成。

## 概述（Overview）

### 交互入口 = SmartObject 槽位上的一条数据

一个 Actor 可交互的**唯一判据**：它的某个 SmartObject 槽位定义数据里挂了 `FSigilSmartObjectInteractionEntranceData`（编辑器里显示为 **Interaction Entrance**）。这个结构只有一个软引用字段 `DefinitionDA`，指向 `USigilInteractionDefinition` 数据资产，里面是静态展示数据：`Text`、`SubText`、`TriggeringInputAction`（`FDataTableRowHandle`，行类型锁定为 CommonUI 的 `CommonInputActionDataBase`）。

查询原语是 `USigilSmartObjectFunctionLibrary::FindSmartObjectsWithInteractionEntranceInActor`，用于扫描某个 Actor 的 SmartObject 槽位里带入口的候选。

### 组件负责候选、选中与选项

`USigilInteractionSystemComponent`（`ActorComponent`）是挂在玩家 Pawn 上的"大脑"：

- **候选列表** —— `InteractableActors` 存潜在目标。插件自己**不会**去填这个数组，见"前置条件"。`SetInteractableActors`（仅 Authority）整体替换数组；同步给拥有客户端的只有数量 `NumsOfInteractableActors`，不是完整数组。
- **当前选中** —— `InteractableActor` 是当前目标（仅对 Owner 同步）。`CycleInteractableActors(bNext)` 是 Reliable Server RPC，用于在候选间轮切；`SetInteractableActor` 在服务端直接指定。勾选 `bNewActorHasPriority` 后，候选列表变化时始终自动选中第一个。
- **交互选项** —— 服务端的 `RefreshOptionsForActor` 把选中 Actor 的槽位解析成 `FSigilInteractionOption` 数组（`Definition`、`SlotIndex`、`SlotState`，以及不同步的 `RequestResult`、`BehaviorDefinition`），`InteractionOptions` 仅对 Owner 同步。搜索时使用 `GetSmartObjectRequestFilter`（`BlueprintNativeEvent`，默认返回 `DefaultRequestFilter`）。
- **交互会话** —— `StartInteraction(Index)`、`EndInteraction`、`InstantInteraction(Index)`（均为 `BlueprintAuthorityOnly`）驱动 `InteractingOption` / `bInteracting`；查询用 `IsInteracting`、`GetInteractingOption`。
- **事件** —— 可在蓝图绑定的委托：`OnInteractableActorChangedEvent`、`OnInteractableActorNumChangedEvent`、`OnInteractingStateChangedEvent`、`OnInteractionOptionsChangedEvent`、`OnSearchInteractableActorsEvent`。

注意：`SearchInteractableActors` 本身**不做任何搜索**，它只是广播 `OnSearchInteractableActorsEvent`，让真正负责候选发现的外部系统去响应。

### 两条执行路径，串联靠项目蓝图

插件提供了两条 GAS 执行路径的零件，但"按下交互键 → 技能跑起来"这段胶水刻意留给项目：

1. **通用交互技能（玩家侧）。** 继承 `USigilGameplayAbility_Interaction`，在蓝图里先调 `TryClaimInteraction(Index, out ClaimedHandle)` 认领当前选项对应的 SmartObject 槽位，再执行技能任务 `USigilAbilityTask_UseSmartObjectWithGameplayBehavior::UseSmartObjectWithGameplayBehavior(ClaimHandle, ClaimPriority)` 触发槽位上的 Gameplay Behavior，结果走 `OnSucceeded` / `OnFailed`。
2. **SmartObject 反向授予技能（物件侧）。** 槽位的行为定义里配 `USigilGameplayBehaviorConfig_InteractionWithAbility`（`AbilityToGrant`、`AbilityLevel`），配套 `USigilGameplayBehavior_InteractionWithAbility`。行为触发时把技能授予交互者的 ASC 并激活，技能结束时行为随之结束。被授予的技能必须是实例化、非 LocalOnly 的技能，且不支持事件触发型技能（`IsDataValid` 会在编辑器里校验）。

### 配套零件

- `ISigilInteractableInterface` —— 可交互 Actor 上的可选表现接口：`GetInteractionDisplayName`、`OnInteractionSelected` / `OnInteractionDeselected`、`OnInteractionStarted` / `OnInteractionEnded`、`OnInteractionOptionSelected`。
- `USigilTargetingFilterTask_InteractionSmartObjects` —— Targeting System 过滤任务（显示名 *(GGS)FilterTask:InteractionSmartObject*），把不带交互入口的目标滤掉，可直接放进候选发现用的 Targeting Preset。
- `USigilSmartObjectFunctionLibrary` —— 蓝图工具：`GetGameplayBehaviorConfig`、`FindGameplayBehaviorConfig`、`FindSmartObjectsWithInteractionEntranceInActor`、`FindInteractionDefinitionFromSmartObjectSlot`。
- `USigilSocketRelationshipMapping` —— 独立的数据资产，按骨骼/网格记录插槽变换修正（`FSigilSocketRelationship` / `FSigilSocketAdjustment`），交互中挂接道具时可用。系统其余部分并不引用它。

## 前置条件（Prerequisites）

这个插件给的是机制零件，不是开箱即用的完整功能。跑通之前需要：

- [ ] **启用引擎插件** —— SmartObjects、GameplayBehaviors、GameplayBehaviorSmartObjects、GameplayAbilities、TargetingSystem、ModularGameplay（`.uplugin` 已声明依赖，启用本插件会自动带起）。
- [ ] **候选发现系统。** 组件从不自己扫描世界。必须由外部在服务端调用 `SetInteractableActors` 喂候选——通常是 Targeting System 预设（可搭配 `USigilTargetingFilterTask_InteractionSmartObjects`）、Overlap 体积或任意蓝图逻辑，建议挂在 `OnSearchInteractableActorsEvent` 上响应。
- [ ] **CommonUI 输入动作 DataTable。** `USigilInteractionDefinition::TriggeringInputAction` 要求行类型为 `CommonInputActionDataBase`；想显示按键提示，项目里得先有配好的 CommonUI 和对应 DataTable。
- [ ] **GAS 环境。** 两条执行路径都假定交互 Pawn 有 `AbilitySystemComponent`。项目要自己创建 `USigilGameplayAbility_Interaction` 的蓝图子类（路径 1）和/或 SmartObject 授予的交互技能（路径 2），并自行授予、触发。
- [ ] **开启 Push Model 同步**（`net.IsPushModelEnabled=1`）——组件的全部同步属性均注册为 push-based，不开的话 Owner 侧状态不会更新。

## 快速上手（Quick Start）

1. **做交互定义。** 新建 `USigilInteractionDefinition` 数据资产，填 `Text`、`SubText`，需要按键提示就配 `TriggeringInputAction`。
2. **把 SmartObject 标记为可交互。** 在目标 Actor 的 SmartObject Definition 资产里，给某个槽位的定义数据加一条 **Interaction Entrance**（`FSigilSmartObjectInteractionEntranceData`），`DefinitionDA` 指向上一步的资产；同时给槽位配行为定义（走路径 2 的话，行为定义里放 `USigilGameplayBehaviorConfig_InteractionWithAbility`）。
3. **挂组件。** 给玩家 Pawn（或其它归玩家所有的同步 Actor）加 `USigilInteractionSystemComponent`，按需配 `DefaultRequestFilter`、`bNewActorHasPriority`。
4. **喂候选。** 服务端跑你的发现逻辑（Targeting 预设、Overlap 等）后调用 `SetInteractableActors`；也可以在玩法代码里调 `SearchInteractableActors`，让发现逻辑绑定 `OnSearchInteractableActorsEvent` 被动触发。
5. **搭交互技能。** 蓝图继承 `USigilGameplayAbility_Interaction`：激活时先 `TryClaimInteraction` 拿认领句柄，再接 `UseSmartObjectWithGameplayBehavior`。把技能授予 Pawn，并从你的输入层触发。
6. **接 UI。** 拥有客户端上绑定组件的各变更委托，读 `GetInteractionOptions` / `GetNumOfInteractableActors` 渲染提示。

## 关键类型（Key Types）

| 类型 | 说明 |
| --- | --- |
| `USigilInteractionSystemComponent` | 玩家侧组件：候选、选中、选项同步与交互会话状态，全部以服务端为准。 |
| `FSigilSmartObjectInteractionEntranceData` | SmartObject 槽位定义数据（"Interaction Entrance"）；它的存在就是可交互的唯一标记，持有 `DefinitionDA`。 |
| `USigilInteractionDefinition` | 数据资产：`Text`、`SubText`、`TriggeringInputAction`（CommonUI 输入动作行）。 |
| `FSigilInteractionOption` | 解析出的交互选项：`Definition`、`SlotIndex`、`SlotState`，以及不同步的 `RequestResult`、`BehaviorDefinition`。 |
| `USigilGameplayAbility_Interaction` | 玩家侧路径的技能基类；提供 `TryClaimInteraction`，并持有 `InteractionSystem` 引用。 |
| `USigilAbilityTask_UseSmartObjectWithGameplayBehavior` | 用认领句柄执行槽位 Gameplay Behavior 的技能任务；`OnSucceeded` / `OnFailed`。 |
| `USigilGameplayBehavior_InteractionWithAbility` | 把技能授予交互者 ASC 并激活的 Gameplay Behavior，技能结束即行为结束。 |
| `USigilGameplayBehaviorConfig_InteractionWithAbility` | 上者的配置：`AbilityToGrant`（须实例化、非 LocalOnly、不可事件触发）、`AbilityLevel`。 |
| `ISigilInteractableInterface` | 可交互 Actor 上的可选表现回调（选中/取消、开始/结束、选项选中、显示名）。 |
| `USigilTargetingFilterTask_InteractionSmartObjects` | Targeting System 过滤任务：只保留带交互入口的目标。 |
| `USigilSmartObjectFunctionLibrary` | 蓝图工具库：查行为配置、查入口、从槽位取交互定义。 |
| `USigilSocketRelationshipMapping` | 独立数据资产：网格资产 → 按骨骼的插槽变换修正映射。 |

## 配置（Configuration）

没有项目级设置，配置都在资产和组件上：

- **组件** —— `DefaultRequestFilter`（`FSmartObjectRequestFilter`，默认 `GetSmartObjectRequestFilter` 就返回它）、`bNewActorHasPriority`。
- **SmartObject 定义资产** —— 入口、槽位、行为配置都在这里编排。
- **交互定义资产** —— 每种交互动词一个 `USigilInteractionDefinition`。
- 行为配置与交互技能都实现了编辑器数据校验（`IsDataValid`），配错会在编辑器里报出来。

## 网络（Networking）

组件采用"服务端计算、Owner 消费"模型，所有同步属性均为 Push Model + `COND_OwnerOnly`：

| 状态 | 同步策略 |
| --- | --- |
| `InteractableActors`（完整候选数组） | **不同步**，仅服务端持有。 |
| `NumsOfInteractableActors` | 仅同步给 Owner。 |
| `InteractableActor`（当前选中） | 仅同步给 Owner。 |
| `InteractionOptions` | 仅同步给 Owner（其中 `RequestResult`、`BehaviorDefinition` 字段标记 `NotReplicated`）。 |
| `InteractingOption` | 仅同步给 Owner。 |
| `CycleInteractableActors` | Reliable Server RPC（客户端可直接调）。 |
| `SetInteractableActors` / `SetInteractableActor` / `StartInteraction` / `EndInteraction` / `InstantInteraction` / `SearchInteractableActors` | 仅 Authority，且不是 RPC——客户端输入到服务端的通路要项目自己搭（通常经由 GAS 技能）。 |

技能执行本身按 GAS 与 SmartObject 子系统的常规方式同步。

## 相关文档（See Also）

- [sigil.input](sigil-input.zh-CN.md) —— 可用于触发交互技能的标签化输入层。
- [sigil.ui](sigil-ui.zh-CN.md) —— 渲染交互提示与选项列表的 UI 层。
- [sigil.effects](sigil-effects.zh-CN.md) —— 为交互动画提供情景化音效/特效反馈。
