[English](sigil-input.md) | [简体中文](sigil-input.zh-CN.md)

# sigil.input

**插件：** `SigilInput` · **模块：** `SigilInput`（Runtime） · **依赖：** Enhanced Input（引擎插件）、Gameplay Tags

sigil.input 是架在 Enhanced Input 之上的一层「Tag 化」输入抽象：玩法代码不再直接绑死 `UInputAction` 资产，而是把每个动作映射到一个 Gameplay Tag，之后在游戏任何地方按 Tag 监听输入事件。插件另外提供数据驱动的输入准入管线（Checker）、分发管线（Processor），以及面向动作游戏的输入缓冲（预输入）系统。

## 概述

### Tag 化输入事件

系统核心是 `USigilInputSystemComponent`，挂在 Pawn 或 PlayerController 上。组件注册时会把自己的 `InputMappingContext` 加进 Enhanced Input 本地玩家子系统，并绑定 `USigilInputConfig` 里列出的全部 `UInputAction`。此后每个 Enhanced Input 触发事件（`Started`、`Triggered`、`Ongoing`、`Canceled`、`Completed`）都会被转换成 `(FInputActionInstance, FGameplayTag, ETriggerEvent)` 三元组，送入控制管线。

输入 Tag 约定挂在 `InputTag` 或 `Sigil.Input.InputTag` 两个根下——插件里所有 Tag 属性都带 `meta=(Categories="InputTag,Sigil.Input.InputTag")` 过滤。

### 控制管线：Checker 与 Processor

每个组件持有一个 `USigilInputControlSetup` 数据资产栈（`InputControlSetups`），栈顶（列表最后一项）是当前生效的 Setup。每个输入事件经过两个阶段：

1. **准入检查** —— `InputCheckers` 里所有内联实例化的 `USigilInputChecker` 都放行，输入才算通过。内置的 `USigilInputChecker_TagRelationship` 用 `FGameplayTagQuery` 匹配角色当前的 Tag，命中哪条关系就只放行那条关系列出的输入。
2. **分发处理** —— 通过检查的输入交给 `InputProcessors` 里的 `USigilInputProcessor` 实例。每个 Processor 按 `InputTags` 和 `TriggerEvents` 过滤，再按事件类型分发到可蓝图实现的处理函数（`HandleInputStarted`、`HandleInputTriggered`、`HandleInputOngoing`、`HandleInputCanceled`、`HandleInputCompleted`）。`InputProcessorExecutionType` 决定是所有命中的 Processor 依次执行（`MatchAll`）还是只执行第一个（`FirstOnly`）。

运行时用 `PushInputSetup` / `PopInputSetup` 整套切换规则——比如菜单、载具、过场各配一套 Setup。

### 输入缓冲（预输入）

当前 Setup 打开 `bEnableInputBuffer` 后，被 Checker **拒绝**的输入不会直接丢弃，而是尝试存进缓冲区。缓冲窗口在 `USigilInputConfig::InputBufferDefinitions` 里定义，用 `OpenInputBufferWindow` / `CloseInputBufferWindow` 按名字开关（典型用法是在攻击后摇的动画通知里开窗）。每个 `FSigilInputBufferWindow` 声明接受哪些输入（`AllowedInputs`）以及缓冲策略 `ESigilInputBufferType`：

| 缓冲类型 | 行为 |
| --- | --- |
| `LastInput` | 窗口关闭时只触发最后存入的那个输入。 |
| `Instant` | 窗口开着期间输入立刻触发。 |
| `HighestPriority` | `AllowedInputs` 列表中越靠前优先级越高，窗口关闭时触发优先级最高的那个。 |

缓冲输入触发时通过 `OnFireBufferedInput` 广播。

### 调试

组件维护三份滚动历史（`PassedInputEntries`、`BlockedInputEntries`、`BufferedInputEntries`，条数上限 `MaxInputEntriesNum`）。插件注册了名为 **SigilInput** 的 Gameplay Debugger 分类，可视化缓冲窗口和输入历史。日志走 `LogSigilInput`；Setup 上的调试日志（`bEnableInputDebug`）需要把 `LogSigilInput` 调到 `VeryVerbose` 才生效。

## 前置条件

- 启用 **Enhanced Input** 插件（`SigilInput.uplugin` 已声明依赖）。
- 在 `InputTag.*` 或 `Sigil.Input.InputTag.*` 下建好输入 Tag（如 `InputTag.Jump`、`InputTag.Attack`）。
- 照常准备 Enhanced Input 的 `UInputMappingContext` 和 `UInputAction` 资产。
- 准备一个 Pawn 或 PlayerController 承载 `USigilInputSystemComponent`（组件对宿主类型有断言要求）。

## 快速上手

1. **建 Input Config。** 新建 `USigilInputConfig` 数据资产，在 `InputActionMappings` 里把每个输入 Tag 映射到一个 `FSigilInputActionSetting`（`UInputAction` + `bValueBinding`；开着值绑定才能随时用 `GetInputActionValueOfInputTag` 查当前值）。

2. **建 Input Control Setup。** 新建 `USigilInputControlSetup` 数据资产：`InputCheckers` 加准入检查器（留空即全部放行），`InputProcessors` 加处理器。最简单的做法是蓝图子类化一个 Processor，重写 `HandleInputStarted`，在里面激活技能或驱动移动。

3. **挂组件。** 在 Pawn 或 PlayerController 上添加 `USigilInputSystemComponent`，配置：
   - `InputMappingContext` 与 `InputPriority`
   - `InputConfig`（第 1 步）
   - `InputControlSetups`（至少一项，第 2 步）

4. **响应输入。** 三选一：实现 Processor（推荐，数据驱动）；绑定组件的 `OnReceivedInput` 委托；或用蓝图异步节点 **Listen Input Event**（`USigilAsyncAction_ListenInputEvent::ListenInputEvent`），它还能顺带监听缓冲输入的触发。

5. **可选——预输入。** 在 `USigilInputConfig::InputBufferDefinitions` 定义缓冲窗口，Setup 上打开 `bEnableInputBuffer`，然后在需要排队输入的时间段调用 `OpenInputBufferWindow(Tag)` / `CloseInputBufferWindow(Tag)`，并绑定 `OnFireBufferedInput` 消费排队的输入。

6. **按需查询输入值**（C++ 或蓝图均可）：

   ```cpp
   USigilInputSystemComponent* Input = USigilInputSystemComponent::GetInputSystemComponent(Actor);
   FInputActionValue Move = Input->GetInputActionValueOfInputTag(MoveTag);
   bool bAllowed = Input->CheckInputAllowed(JumpTag, ETriggerEvent::Started);
   ```

## 关键类型

| 类型 | 说明 |
| --- | --- |
| `USigilInputSystemComponent` | 核心组件：绑定 Enhanced Input、跑检查/分发管线、管理输入缓冲。必须挂在 Pawn 或 PlayerController 上。 |
| `USigilInputConfig` | Const 数据资产：`InputActionMappings`（Tag → Action 映射）+ `InputBufferDefinitions`（缓冲窗口）。 |
| `USigilInputControlSetup` | Const 数据资产：打包 `InputCheckers`、`InputProcessors`、`bEnableInputBuffer` 与执行顺序，可用 `PushInputSetup`/`PopInputSetup` 栈式切换。 |
| `USigilInputChecker` | 抽象、可蓝图化的输入校验基类，重写 `DoCheckInput` 决定放行与否。 |
| `USigilInputChecker_TagRelationship` | 内置检查器：按 `FGameplayTagQuery` 匹配角色 Tag，只放行对应关系里列的输入；Tag 来源可重写 `GetActorTags`。 |
| `USigilInputProcessor` | 可蓝图化的处理器：按 `InputTags`/`TriggerEvents` 过滤，按触发事件分发蓝图事件。 |
| `FSigilInputActionSetting` | `UInputAction` 引用 + `bValueBinding`。 |
| `FSigilInputBufferWindow` | 缓冲窗口定义：`Tag`、`BufferType`、`AllowedInputs`。 |
| `FSigilAllowedInput` | 一个输入 Tag 及其允许的触发事件（留空 = 全部允许）。 |
| `FSigilBufferedInput` | 一条输入记录：Tag、`FInputActionInstance` 数据、触发事件。 |
| `ESigilInputBufferType` | `LastInput` / `Instant` / `HighestPriority`。 |
| `ESigilInputProcessorExecutionType` | `MatchAll` / `FirstOnly`。 |
| `USigilAsyncAction_ListenInputEvent` | 蓝图异步节点：按 Tag 过滤监听输入事件或缓冲输入触发。 |
| `USigilInputFunctionLibrary` | 工具库：`GetInputActionValue`（支持从 `FInputActionInstance` 自动转换）、Tag 名工具、TagQuery 描述等。 |

## 配置

本插件全部走资产配置，没有项目设置（DeveloperSettings）。

- **`USigilInputConfig`** —— 每套输入方案一份。它和 `USigilInputControlSetup` 都实现了编辑器数据校验（`IsDataValid`），配置错误会在数据校验时暴露。
- **`USigilInputControlSetup`** —— 每种输入「模式」一份。主要属性：

  | 属性 | 说明 |
  | --- | --- |
  | `InputCheckers` | 内联检查器列表，需全部通过。 |
  | `bEnableInputBuffer` | 被拒输入存入已打开的缓冲窗口（默认关；UI 类 Setup 通常不需要）。 |
  | `InputProcessorExecutionType` | `MatchAll`（默认）或 `FirstOnly`。 |
  | `InputProcessors` | 内联处理器列表，自上而下执行。 |
  | `bEnableInputDebug`、`DebugInputTags`、`DebugTriggerEvents` | 调试日志过滤；需 `LogSigilInput` 为 `VeryVerbose`。 |

- **`USigilInputSystemComponent`** —— 逐 Actor 配置：`InputMappingContext`、`InputPriority`、`InputConfig`、`InputControlSetups`、`MaxInputEntriesNum`（0–10，默认 5），以及 `bProcessingInputExternally`（绕过内置管线，自行处理 `OnReceivedInput`）。

## 网络

本插件**刻意设计为纯本地**：输入采集、检查、分发、缓冲都发生在持有输入的那台机器上，模块内没有任何属性复制、也没有 RPC。如果 Processor 要触发联网玩法（比如激活一个会复制的技能），复制由被调用的系统负责——例如 sigil.gas。

## 相关文档

- [sigil.gas](sigil-gas.zh-CN.md) —— 技能系统层，Tag 化输入事件的天然消费方。
- [sigil.combat](sigil-combat.zh-CN.md) —— 基于 sigil.gas 的战斗层。
