[English](sigil-gas.md) | [简体中文](sigil-gas.zh-CN.md)

# sigil.gas

**插件：** `SigilGas` · **模块：** `SigilGas`（Runtime）、`SigilGasEditor`（Editor） · **依赖：** Gameplay Abilities、Enhanced Input、Targeting System、Modular Gameplay（均为引擎插件）

sigil.gas 是 Sigil 套件的 GAS（Gameplay Ability System）基建层：在引擎的 `UAbilitySystemComponent` 与 `UGameplayAbility` 之上补齐了显式初始化生命周期、数据驱动的技能集（AbilitySet）、三态激活组互斥、技能 Tag 关系映射、游戏阶段（GamePhase）子系统、全局技能系统、一批可复用的 Ability/Async Task、可复用目标捕获 Actor，以及 Health / Stamina / Mana 三套常用属性集和配套的属性变化回调分发组件。

## 概述

### 扩展版 ASC

`USigilAbilitySystemComponent` 在引擎 ASC 基础上增加：

- **显式生命周期** —— 调用 `InitializeAbilitySystem(OwnerActor, AvatarActor)` / `UninitializeAbilitySystem()`。初始化时授予 `DefaultAbilitySets` 里的全部技能集、按 `AttributeSetInitializeGroupName` / `AttributeSetInitializeLevel` 初始化属性、注册进 `USigilGlobalAbilitySystem`，并广播 `OnAbilitySystemInitialized` / `OnAbilitySystemUninitialized`。
- **激活组** —— `IsActivationGroupBlocked`、`AddAbilityToActivationGroup`、`CanChangeActivationGroup`、`ChangeActivationGroup`、`CancelActivationGroupAbilities`（详见下文）。
- **Tag 关系映射** —— 挂一份 `USigilAbilityTagRelationshipMapping` 资产（运行时可用 `SetTagRelationshipMapping` 替换），在技能激活判定时追加 block / cancel / required / blocked 标签。
- **激活事件** —— 可蓝图绑定的 `OnAbilityActivated`、`OnAbilityActivationFailed`（失败原因通过不可靠 Client RPC 送回本机客户端）、`AbilityEndedEvent`。
- **RPC 批处理** —— `ShouldDoServerAbilityRPCBatch()` 返回 true；`BatchRPCTryActivateAbility` 可把激活 + 目标数据 + 结束压进更少的 RPC。
- **复制的 Gameplay Event** —— `SendGameplayEventToActor_Replicated` 经 Server RPC + Multicast 把事件广播到所有端。
- **复制模式** —— 通过 `AbilitySystemReplicationMode` 属性暴露（`EGameplayEffectReplicationMode`）。

### 技能集（AbilitySet）

`USigilAbilitySet` 是 Const 的 `UPrimaryDataAsset`，打包 `GrantedGameplayAbilities`、`GrantedGameplayEffects`、`GrantedAttributes` 三类授予项。用 `GiveToAbilitySystem`（或静态、仅服务器的 `GiveAbilitySetToAbilitySystem`）授予，之后凭返回的 `FSigilAbilitySet_GrantedHandles` 调 `TakeFromAbilitySystem` 一键收回。每条技能项含 `Ability`、`AbilityLevel`、`InputID`、`DynamicTags`。

### Sigil 技能基类

`USigilGameplayAbility`（Abstract）是推荐的技能基类：

- **`ActivationGroup`**（`ESigilAbilityActivationGroup`）：`Independent`（独立运行）、`Exclusive_Replaceable`（可被顶替：新的独占技能激活时被取消）、`Exclusive_Blocking`（阻塞：激活期间不许其他独占技能启动）。独占技能同一时刻只能有一个在跑。
- **附加成本** —— `AdditionalCosts` 里内联的 `USigilAbilityCost` 实例（弹药、次数等），在 `CheckCost` 检查、`ApplyCost` 扣除；每个成本可勾 `bOnlyApplyCostOnHit`（仅命中时扣）。
- **效果容器** —— `EffectContainerMap` 把 Tag 映射到 `FSigilGameplayEffectContainer`（`UTargetingPreset` + 一组 GameplayEffect 类），运行时用 `MakeEffectContainerSpec` / `ApplyEffectContainer` 结算。
- **激活期松散 Tag** —— `ActivationOwnedLooseTags`（`FSigilGameplayTagCount`）在技能激活期间加到持有者身上。
- **可选 Tick** —— 勾 `bEnableTick` 后激活期间收到 `AbilityTick` 事件（按 Actor 实例化的技能）。
- **特性 Tag** —— 技能带 `Sigil.Ability.Trait.ActivationOnSpawn` 则授予后立即激活（`TryActivateAbilityOnSpawn`）；`Sigil.Ability.Trait.Persistent` 标记常驻技能。
- **蓝图钩子** —— `K2_OnGiveAbility`、`K2_OnRemoveAbility`、`K2_OnAvatarSet`、`K2_OnInputPressed`、`K2_OnInputReleased`、`K2_OnCheckCost`、`K2_OnApplyCost`、`K2_ShouldActivateAbility`、`OnActivationFailed`；客户端预测目标数据用 `SendTargetDataToServer` 上报。

### Tag 关系映射

`USigilAbilityTagRelationshipMapping` 数据资产由若干 `FSigilAbilityTagRelationship` 组成：针对一个 `AbilityTag`，声明 `AbilityTagsToBlock`（激活期间阻挡谁）、`AbilityTagsToCancel`（激活时取消谁）、`ActivationRequiredTags`、`ActivationBlockedTags`。另有 `Layered` 列表（`FSigilAbilityTagRelationshipsWithQuery`）：只在角色 Tag 满足 `ActorTagQuery` 时叠加生效。技能间的相互克制关系集中配在这一处，不用散落在各技能资产里。

### 游戏阶段（GamePhase）

`USigilGamePhaseSubsystem`（World Subsystem，服务器权威）用嵌套的 Gameplay Tag 管理游戏阶段：父子阶段可共存、兄弟阶段互斥——启动 `GamePhase.Playing.SuddenDeath` 会结束 `GamePhase.Playing.NormalPlay`，但 `GamePhase.Playing` 保持激活。阶段本体是 `USigilGamePhaseAbility` 子类（`GamePhaseTag` 定义阶段），用 `StartPhase`（蓝图节点 **Start Phase**）启动；用 `WhenPhaseStartsOrIsActive` / `WhenPhaseEnds`（`ExactMatch` / `PartialMatch` 两种匹配）订阅，或用 `IsPhaseActive` 查询。

### 全局技能系统

`USigilGlobalAbilitySystem`（World Subsystem）追踪所有已注册的 `USigilAbilitySystemComponent`（ASC 初始化时自动注册），提供 `ApplyAbilityToAll` / `ApplyEffectToAll` / `RemoveAbilityFromAll` / `RemoveEffectFromAll`——对之后才注册进来的 ASC 也会补发。

### 属性

- **属性集** —— `USigilHealthSet`（`Health`、`MaxHealth`，外加不复制的 meta 属性 `IncomingHealing`、`IncomingDamage`）、`USigilStaminaSet`（`Stamina`、`MaxStamina`、`IncomingHealing`、`IncomingDamage`）、`USigilManaSet`（`Mana`、`MaxMana`）。当前值钳制在最大值内，最大值变化时按比例调整当前值。每个属性都注册了 `Sigil.Attribute.<Set名>.<属性名>` 形式的 Tag（如 `Sigil.Attribute.HealthSet.Health`），配合 Tag↔属性工具使用。

  > **由项目实现：** `IncomingDamage` / `IncomingHealing` 是 *meta 属性*。插件只负责声明、钳制和转发回调，**不会**把它们换算成 `-Health` / `+Health`。写入和消费这两个属性的 GameplayEffect / `UGameplayEffectExecutionCalculation` 必须由项目侧提供（sigil.combat 的受击流水线会写 `IncomingDamage`，但最终扣血结算同样在项目侧）。

- **变化回调分发** —— 在同一 Actor 上挂 `USigilAttributeSystemComponent`，Sigil 属性集会把回调转发给它：`OnPostAttributeChange`、`OnAttributeChanged`（服务端和客户端都触发）、`OnPostGameplayEffectExecute`，每个都有对应的 `Handle...` 蓝图原生事件供子类重写。做 UI / 表现响应不必子类化属性集。
- **Tag ↔ 属性注册表** —— `USigilGameplayAttributesHelper` 维护全局 Tag→属性映射（`RegisterTagToAttribute`、`TagToAttribute`、`AttributeToTag`、`SetFloatAttribute`、百分比查询等）。
- **数据驱动默认值** —— `USigilAbilitySystemGlobals` 扩展 `UAbilitySystemGlobals`：按 `FSigilAttributeGroupName`（`MainName` + 可选 `SubName`）执行 `InitAttributeSetDefaults` / `ApplyAttributeDefault`。曲线表行名沿用引擎的 `组.属性集.属性` 三段格式；子组编码在组名段里写成 `Main->Sub`（如 `Hero->Warrior.SigilHealthSet.MaxHealth`），因为引擎按 `.` 拆行名。另通过 `ISigilAbilitySystemGlobalsEventReceiver` 暴露全局的「效果应用前」事件。属性组初始化**要求**项目的 AbilitySystemGlobals 类是 `USigilAbilitySystemGlobals` 或其子类（见「配置」），否则只打警告日志、什么都不做。
- `SigilGasEditor` 编辑器模块为 `FSigilAttributeGroupName` 提供了属性面板定制。

### 四种现成的 ASC 宿主 Actor

| Actor | ASC 位置 |
| --- | --- |
| `ASigilCharacter` | 自身不带 ASC；实现 `IAbilitySystemInterface` + `IGameplayTagAssetInterface`，转发给蓝图事件 `CustomGetAbilitySystemComponent`——适合 ASC 放在 PlayerState 的架构。 |
| `ASigilCharacterWithAbilities` | 自带 `USigilAbilitySystemComponent` 子对象。 |
| `ASigilPlayerState` | 自带 `USigilAbilitySystemComponent`；面向 Game Feature 扩展的极简 PlayerState。 |
| `ASigilGameStateBase` / `ASigilGameState` | 自带全局用途的 `USigilAbilitySystemComponent`（主要用于全局 GameplayCue）。 |

### Task 与目标捕获 Actor

**Ability Task**（技能内使用）：

- `USigilAbilityTask_PlayMontageAndWaitForEvent`（`PlayMontageAndWaitForEvent` / `...Ext`）—— 播蒙太奇并接收期间的 Gameplay Event。
- `USigilAbilityTask_WaitTargetDataUsingActor`（`WaitTargetDataWithReusableActor`）—— 用已生成的可复用目标 Actor 做目标确认。
- `USigilAbilityTask_ServerWaitForClientTargetData` —— 服务器等待客户端预测的目标数据。
- `USigilAbilityTask_WaitInputPressWithTags` —— 带 Tag 条件（必需/忽略容器）的输入按下等待；同时会比较 `Sigil.State.Interacting` 与 `Sigil.State.InteractingRemoval` 的计数。
- `USigilAbilityTask_WaitGameplayEvents` —— 同时监听多个事件 Tag。
- `USigilAbilityTask_WaitDelayOneFrame` —— 延一帧。

**Async Task**（任意蓝图可用）：

- `USigilAsyncTask_AttributeChanged`（`ListenForAttributeChange` / `ListenForAttributesChange`）
- `USigilAsyncTask_GameplayTagAddedRemoved`（`ListenForGameplayTagAddedOrRemoved`）
- `USigilAsyncTask_WaitGameplayAbilityActivated`（`WaitGameplayAbilityActivated`）
- `USigilAsyncTask_WaitGameplayAbilityEnded`（`WaitGameplayAbilityEnded` / `WaitAbilitySpecHandleEnded`）

**目标捕获 Actor** —— `ASigilAbilityTargetActor_Trace`（可配置基类：`MaxRange`、`TraceProfile`、瞄准扩散、`NumberOfTraces`、`MaxHitResultsPerTrace` 等）及两个形状子类 `ASigilAbilityTargetActor_LineTrace`、`ASigilAbilityTargetActor_SphereTrace`，设计上与 `WaitTargetDataWithReusableActor` 配套。

### 原生 Gameplay Tag

| Tag | 含义 |
| --- | --- |
| `Sigil.Ability.ActivateFail.Cooldown` | 激活失败：冷却中。 |
| `Sigil.Ability.ActivateFail.Cost` | 激活失败：成本检查未通过。 |
| `Sigil.Ability.ActivateFail.TagsBlocked` | 激活失败：被 Tag 阻挡。 |
| `Sigil.Ability.ActivateFail.TagsMissing` | 激活失败：缺少必需 Tag。 |
| `Sigil.Ability.ActivateFail.Networking` | 激活失败：网络检查未通过。 |
| `Sigil.Ability.ActivateFail.ActivationGroup` | 激活失败：激活组互斥。 |
| `Sigil.Ability.Trait.ActivationOnSpawn` | 授予后立即激活。 |
| `Sigil.Ability.Trait.Persistent` | 玩法过程中常驻的技能。 |
| `Sigil.State.Interacting` | 持有者正在交互中。 |
| `Sigil.State.InteractingRemoval` | 交互态的待移除计数，与 `Sigil.State.Interacting` 计数比较。 |

## 前置条件

- 启用引擎插件 **Gameplay Abilities**、**Enhanced Input**、**Targeting System**、**Modular Gameplay**（`SigilGas.uplugin` 已声明）。
- 要用属性组初始化，项目的 AbilitySystemGlobals 类必须是 `USigilAbilitySystemGlobals` 或其子类（如 sigil.combat 的），见「配置」。
- 为技能 / 阶段准备好 Gameplay Tag；插件自己的原生 Tag（见上表）自动注册。

## 快速上手

1. **选 ASC 宿主。** 继承 `ASigilCharacterWithAbilities`（ASC 在 Pawn 上），或 `ASigilPlayerState` + `ASigilCharacter`（ASC 在 PlayerState 上），或在自己的 Actor 上挂 `USigilAbilitySystemComponent`。

2. **建技能集资产。** 新建 `USigilAbilitySet`，填入技能（`USigilGameplayAbility` 子类）、起始效果、属性集（如 `USigilHealthSet`），赋给 ASC 的 `DefaultAbilitySets`。

3. **初始化 ASC。** 在 Possess / 复制就绪的时机（如 `PossessedBy`、`OnRep_PlayerState`）调用：

   ```cpp
   AbilitySystemComponent->InitializeAbilitySystem(/*Owner*/ PlayerState, /*Avatar*/ Pawn);
   ```

4. **配激活规则。** 给每个技能设置 `ActivationGroup`；另建一份 `USigilAbilityTagRelationshipMapping` 赋给 ASC 的 `TagRelationshipMapping`，集中管理技能间的阻挡 / 取消关系。

5. **可选——曲线表属性默认值。** 配好 Globals 类（见「配置」），在 `UAbilitySystemGlobals` 注册属性默认曲线表，再在 ASC 上填 `AttributeSetInitializeGroupName` / `AttributeSetInitializeLevel`，初始化时自动套用。

6. **可选——属性响应。** 在化身 Actor 上加 `USigilAttributeSystemComponent`，绑定 `OnAttributeChanged`（两端都触发）驱动 UI 和表现。

7. **可选——游戏阶段。** 建 `USigilGamePhaseAbility` 子类并设 `GamePhaseTag`（如 `GamePhase.Playing`），服务器上调 **Start Phase** 启动，用 **When Phase Starts or Is Active** 订阅。

## 关键类型

| 类型 | 说明 |
| --- | --- |
| `USigilAbilitySystemComponent` | 扩展版 ASC：生命周期、技能集、激活组、Tag 关系、RPC 批处理、复制事件。 |
| `USigilGameplayAbility` | 技能基类：激活组、附加成本、效果容器、可选 Tick、蓝图钩子。 |
| `USigilAbilitySet` | 授予技能/效果/属性集的数据资产，凭 `FSigilAbilitySet_GrantedHandles` 收回。 |
| `USigilAbilityCost` | 内联实例化、可蓝图化的附加激活成本（弹药、次数等）。 |
| `ESigilAbilityActivationGroup` | `Independent` / `Exclusive_Replaceable` / `Exclusive_Blocking`。 |
| `USigilAbilityTagRelationshipMapping` | 阻挡/取消/必需/禁止 Tag 关系的数据资产，含按 TagQuery 分层的规则。 |
| `USigilGamePhaseSubsystem` | 嵌套 Tag 式游戏阶段的 World Subsystem（仅服务器）。 |
| `USigilGamePhaseAbility` | 代表一个游戏阶段的技能基类（`GamePhaseTag`）。 |
| `USigilGlobalAbilitySystem` | 向所有已注册 ASC 批量施加技能/效果的 World Subsystem。 |
| `USigilHealthSet` / `USigilStaminaSet` / `USigilManaSet` | 常用属性集，带 `Sigil.Attribute.*` Tag；Health/Stamina 含 `IncomingDamage`/`IncomingHealing` meta 属性（由项目消费）。 |
| `USigilAttributeSystemComponent` | 接收属性变化 / 效果执行回调的 Actor 组件（两端可用）。 |
| `USigilAbilitySystemGlobals` | 扩展版 `UAbilitySystemGlobals`：按组名初始化属性默认值、全局效果应用前事件。 |
| `USigilGameplayAttributesHelper` | Tag↔属性注册表与属性工具函数库。 |
| `ASigilAbilityTargetActor_Trace`（+`_LineTrace`、`_SphereTrace`） | 可复用、可配置的射线目标捕获 Actor，配合 `WaitTargetDataWithReusableActor`。 |
| `USigilAnimNotify_SendGameplayEvent` | 向拥有者发送 Gameplay Event（`EventTag`）的动画通知。 |

## 配置

- **AbilitySystemGlobals 类**（属性组初始化的必要条件）。在 `DefaultGame.ini`：

  ```ini
  [/Script/GameplayAbilities.AbilitySystemGlobals]
  AbilitySystemGlobalsClassName=/Script/SigilGas.SigilAbilitySystemGlobals
  ```

  同时使用 sigil.combat 时改配它的子类（见 combat 文档）。不配 Sigil 系 Globals 类时，`InitializeAttributes` 只打警告、不生效。

- **ASC 级默认值** —— `DefaultAbilitySets`、`AttributeSetInitializeGroupName`（`MainName`/`SubName`）、`AttributeSetInitializeLevel`、`TagRelationshipMapping`、`AbilitySystemReplicationMode`。
- **数据校验** —— `USigilAbilitySet`、`USigilGameplayAbility`、`USigilGamePhaseAbility` 均实现了编辑器数据校验 / 预保存处理；技能集条目带仅编辑器的启用开关便于调试。
- 本插件没有 `UDeveloperSettings`，全部走资产与 ini 配置。

## 网络

- **服务器权威的 GAS 流程** —— 技能授予（`GiveAbilitySetToAbilitySystem` 为 `BlueprintAuthorityOnly`）、游戏阶段、全局技能系统都在服务器执行；属性走标准 GAS 复制（`Health`、`MaxHealth`、`Stamina`、`MaxStamina`、`Mana`、`MaxMana` 复制；`IncomingDamage` / `IncomingHealing` 按 meta 属性惯例不复制）。
- **客户端反馈** —— 激活失败原因经不可靠 Client RPC 回传本机客户端，从 `OnAbilityActivationFailed` / `OnActivationFailed` 冒出。
- **复制的 Gameplay Event** —— `SendGameplayEventToActor_Replicated` = 可靠 Server RPC + Multicast。
- **预测支持** —— RPC 批处理（`BatchRPCTryActivateAbility`）、技能上的 `SendTargetDataToServer`、服务器侧的 `USigilAbilityTask_ServerWaitForClientTargetData`。
- **已知空白** —— `USigilAttributeSystemComponent` 各回调的触发端不同：`OnAttributeChanged` 两端都触发，`OnPostGameplayEffectExecute` 只在效果执行端（服务器）触发。GamePhase 子系统的 API 是 `BlueprintAuthorityOnly`，没有内建的客户端阶段同步。

## 相关文档

- [sigil.input](sigil-input.zh-CN.md) —— Tag 化输入层，输入事件与技能激活的天然搭档。
- [sigil.combat](sigil-combat.zh-CN.md) —— 直接构建在本插件之上的战斗层。
