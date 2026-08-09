[English](sigil-movement.md) | [简体中文](sigil-movement.zh-CN.md)

# sigil.movement

**插件：** `SigilMovement` · **模块：** `SigilMovement`（Runtime）、`SigilMovementEditor`（UncookedOnly） · **依赖：** ModularGameplay、AnimationWarping、AnimationLocomotionLibrary、PoseSearch、Chooser（引擎插件）、Gameplay Tags

sigil.movement（日志与分类中简称 **GMS**，即 Generic Movement System）是一套数据驱动的运动控制与走跑动画框架，走的是 Lyra 路线：传统动画状态机 + 距离匹配（Distance Matching）+ 步幅/朝向扭曲（Warping），全程用 Gameplay Tag 描述状态。它**不是** Motion Matching 方案——PoseSearch 与 Chooser 虽在依赖列表里，但只服务于旁路工具函数（`USigilUtility::EvaluatePoseSearchDatabasesChooser` 等），主干走跑流程完全不需要 PoseSearch 数据库。

## 总览

### 组件层

`USigilMovementSystemComponent` 是抽象基类（不可蓝图化），持有全部核心复制状态——`MovementSet`（运动集）、`DesiredMovementState`/`MovementState`（运动状态）、`DesiredRotationMode`/`RotationMode`（旋转模式）、`LocomotionMode`（移动模式）、`OverlayMode`（叠层模式）、`OwnedTags`——以及让这些状态在服务器、主控端、模拟端之间保持一致的整套 RPC 管线。每种状态变化都有对应的 `BlueprintAssignable` 事件（`OnMovementSetChangedEvent`、`OnMovementStateChangedEvent`、`OnRotationModeChangedEvent`、`OnLocomotionModeChangedEvent`、`OnOverlayModeChangedEvent`）。

`USigilCharacterMovementSystemComponent`（显示名 *"GMS Movement System Component(Character)"*）是面向 `ACharacter` + `UCharacterMovementComponent` 的具体实现：把当前运动状态的速度、加速度写进 CMC，负责地面/空中的旋转系统与视角平滑，并通过 `MovementModeToTagMapping` / `CustomMovementModeToTagMapping` 把 CMC 的移动模式映射为 `Sigil.Movement.LocomotionMode.*` 标签。

`USigilMoverMovementSystemComponent` 只是一个空壳，头文件自己就写着 *"You should not use this class."*——Mover 支持目前不可用。

### 数据资产链

```
USigilMovementDefinition（Const 数据资产）
  └─ MovementSets : TMap<FGameplayTag, FSigilMovementSetSetting>
       ├─ ControlSetting : USigilMovementControlSetting_Default（逻辑面：速度表、跳跃参数、旋转模式）
       ├─ ControlSettings（可选：按叠层模式覆写控制设置）
       ├─ AnimDataSetting_General : FSigilAnimDataSetting_General（共享动画设置）
       └─ 五套动画层设置：
            States  → USigilAnimLayerSetting_States   （内联实例或共享数据资产）
            Overlay → USigilAnimLayerSetting_Overlay  （内联实例或共享数据资产）
            View    → USigilAnimLayerSetting_View
            Additive → USigilAnimLayerSetting_Additive
            SkeletalControls → USigilAnimLayerSetting_SkeletalControls

USigilAnimGraphSetting（数据资产，每套骨架一份）
  ├─ AnimLayerSettingToInstanceMapping : TMap<设置类, USigilAnimLayer 类>
  └─ OrientationWarping : FSigilOrientationWarpingSettings（脊柱/IK 骨骼引用）
```

组件在运行时维护一个可复制的定义**栈**（`MovementDefinitions`）：`PushAvailableMovementDefinition` / `PopAvailableMovementDefinition` 允许运行时叠加武器专属定义（比如装备大剑时压入大剑的定义）；切换 `MovementSet` 时从栈底往上查，后压入的定义优先。

### 动画层系统

`USigilMainAnimInstance` 是必须使用的主动画实例。核心状态一变，它就调用 `RefreshLayerSettings()`：按 `USigilAnimGraphSetting::AnimLayerSettingToInstanceMapping` 解析出每个动画层设置对应的 `USigilAnimLayer`（本质是 Linked Layer 的 `UAnimInstance`），Link/Unlink 到自己身上，再通过 `ApplySetting` 把设置对象喂进去。

自带的层：

- **States（状态层）**——`USigilAnimLayerSetting_States_Default` + `USigilAnimLayer_States_DefaultLocomotion`：类 Lyra 的地面走跑状态机（Idle/IdleBreaks、Start、Cycle、Stop、Pivot、原地转身、Jump/Fall/Land），核心逻辑在 C++ 里以保证性能。该类是 `Abstract`：需要用动画蓝图继承它，在图表里搭真正的状态机，并把节点绑定接到它的 `*_AnimUpdate` / `*_StateUpdate` 函数上。
- **Overlay（叠层）**——两套原生实现：`USigilAnimLayerSetting_Overlay_Stack` / `USigilAnimLayer_Overlay_Stack`（最多 10 个并行的身体部位栈，每个栈从多个 `FSigilAnimData_Overlay` 里选第一个 `TagQuery` 命中的），以及 `USigilAnimLayerSetting_Overlay_PoseBased` / `USigilAnimLayer_Overlay_PoseBased`（手臂/静止/移动姿势混合）。
- **View（视角层）**——`USigilAnimLayerSetting_View_Default` / `USigilAnimLayer_View_Default`：瞄准偏移 BlendSpace，带偏航角限制与平滑。
- **Additive / SkeletalControls**——`USigilAnimLayerSetting_Additive` 与 `USigilAnimLayerSetting_SkeletalControls` 是**空的抽象基类**，纯扩展点，插件不带任何原生实现；自己写子类后在 `AnimLayerSettingToInstanceMapping` 里注册。

另有辅助动画实例 `USigilAnimGraph_Layering`，负责把分部位的分层曲线（`LayerHead`、`LayerArmLeft`、`LayerSpineAdditive` 等）读进分层状态，供自定义分层图表使用。

### 自定义动画节点

Runtime 模块带四个动画节点（对应的编辑器图表节点在 `SigilMovementEditor` 模块）：

| 节点 | 用途 |
| --- | --- |
| `FSigilAnimNode_GameplayTagsBlend` | 用 Gameplay Tag 而非枚举/整数作为键的 Blend List。 |
| `FSigilAnimNode_CurvesBlend` | 只把第二个 Pose 的曲线混进源 Pose（模式对应 `ECurveBlendOption`）。 |
| `FSigilAnimNode_LayeredBoneBlend` | 分层骨骼混合，Branch Filter 可通过 `ExternalLayerSetup` 引脚**动态**传入。 |
| `FSigilAnimNode_OrientationWarping` | 朝向扭曲变体：脊柱/IK 骨骼引用改由 `FSigilOrientationWarpingBoneReference` 引脚传入（来自 `USigilAnimGraphSetting`），一张图表就能服务多套骨架。 |

### Gameplay Tag 是状态词汇表

一切靠 Tag 驱动。组件的 `GetGameplayTags()` 会把 `OwnedTags` 与可选的 `GameplayTagsProvider`（任何实现 `IGameplayTagAssetInterface` 的对象）的标签合并。把 ASC 设为 Provider，就能用 GAS 的标签驱动叠层选择与旋转封锁（`GroundedRotationBlockingTags`、`InAirRotationBlockingTags`）——这是官方预留的 GAS 桥接方式，插件本身对 GAS 零依赖。

## 前置要求

插件**不含任何内容资产**（`CanContainContent: false`）：没有骨架、没有示例动画蓝图、没有现成数据资产。以下都得项目自备。

- **宿主 Actor**：基类组件对 Owner 是 `APawn` 做了 `check`；`USigilCharacterMovementSystemComponent` 进一步要求 `ACharacter` + `UCharacterMovementComponent`（Owner 不是 Character 时组件不会工作，`BeginPlay` 会触发 `ensure` 乃至空指针访问）。
- **关掉控制器旋转**：`bUseControllerRotationPitch/Yaw/Roll` 必须全为 `false`，`BeginPlay` 里有 `ensureMsgf` 把关。Actor 偏航由旋转系统接管。
- **主动画实例**：Mesh 的动画类必须派生自 `USigilMainAnimInstance`。每个 Linked 的 `USigilAnimLayer` 都会执行 `checkf(Parent != nullptr, TEXT("Parent is not SigilMainAnimInstance!"))`——把 Sigil 动画层挂到别的 AnimInstance 下会**直接崩溃**。
- **Gameplay Tags**：插件原生注册了：
  - `Sigil.Movement.LocomotionMode.{None, Grounded, InAir, Flying, Swimming}`
  - `Sigil.Movement.RotationMode.{VelocityDirection, ViewDirection}`
  - `Sigil.Movement.State.{Walk, Jog, Sprint}`
  - `Sigil.Movement.OverlayMode.{None, Default}`
  - `Sigil.Movement.SM` 及 `Sigil.Movement.SM.{InAir, InAir.Jump, InAir.Fall, Grounded, Grounded.Idle, Grounded.Start, Grounded.Cycle, Grounded.Stop, Grounded.Pivot, Grounded.Land}`

  **`Sigil.Movement.Set.*` 没有任何原生标签**——运动集标签（如 `Sigil.Movement.Set.Unarmed`）必须在项目的 Tag 设置里自建；`SetMovementSet` 的参数与 `MovementSets` 的键都限定在这个前缀下。
- **动画曲线**（加在动画资产上）：
  - `Distance`——Start / Stop / Pivot 动画的距离匹配必需（`UAnimDistanceMatchingLibrary` 按 `FName("Distance")` 采样）。
  - `GroundDistance`——可选的 `JumpFallLand` 落地动画必需（与地面预测射线的距离做匹配）。
  - `RotationYawSpeed`、`RotationYawOffset`——由 `USigilCharacterMovementSystemComponent` 读取，让动画驱动/偏移 Actor 偏航（原地转身等）。
  - `AllowTurnInPlace`、`AllowAiming`——`USigilConstants` 暴露的曲线名常量，供你的动画图表使用；Runtime 模块自身并不采样它们。
- **蒙太奇 Slot**：名为 `TurnInPlace` 的 Slot（`USigilConstants::TurnInPlaceSlotName()`）。
- **动画蓝图**：一个派生自 `USigilMainAnimInstance` 的主 ABP，外加你要用的每个动画层各一个 ABP（如派生自 `USigilAnimLayer_States_DefaultLocomotion` 的状态层 ABP），并在 `USigilAnimGraphSetting` 中完成注册。

## 快速上手

1. **先建标签。** 在 **项目设置 → Gameplay Tags** 里至少加一个运动集标签，如 `Sigil.Movement.Set.Default`。

2. **建控制设置。** 新建 `USigilMovementControlSetting_Default` 数据资产（*GMS Movement Control Setting*）。按速度从小到大填 `MovementStates`——每条 `FSigilMovementStateSetting` 包含 `Sigil.Movement.State.*` 标签、`SpeedLevel`、`Speed` / `StrafeSpeed` / `BackwardsSpeed`、`Acceleration`、`BrakingDeceleration`、允许的旋转模式及各模式的旋转参数。`MovementStates` **不能为空**（否则 `checkf` 崩溃）。`JumpStates` 按需填写。

3. **建动画层设置 + 动画蓝图。**
   - 用 ABP 继承 `USigilAnimLayer_States_DefaultLocomotion` 实现地面状态机；新建 `USigilAnimLayerSetting_States_Default`，填 Idle/转身/跳跃/落地数据，以及按运动状态划分的 Start/Cycle/Stop/Pivot 动画（`MovingStates`，键为 `Sigil.Movement.State.*`）。`MovingStates` 不能为空。
   - 按需再建叠层设置（`USigilAnimLayerSetting_Overlay_Stack` 或 `_PoseBased`）与视角层设置（`USigilAnimLayerSetting_View_Default`）及各自的 ABP。

4. **为骨架建 `USigilAnimGraphSetting`**，在 `AnimLayerSettingToInstanceMapping` 里把每个设置类映射到对应的动画层（ABP）类，并配好 `OrientationWarping` 的骨骼引用（脊柱链、IK Foot Root、IK 脚）。

5. **建 `USigilMovementDefinition`。** 在 `MovementSets` 里以你的运动集标签为键加一条 `FSigilMovementSetSetting`，塞入第 2 步的控制设置和第 3 步的动画层设置。

6. **配置角色。**
   - 挂 `USigilCharacterMovementSystemComponent`；设置 `MovementDefinitions`（至少一条）、`MovementSet`（你的运动集标签）、`AnimGraphSetting`。
   - Mesh 的动画类设为你的主 ABP（派生自 `USigilMainAnimInstance`）。
   - 关闭 `bUseControllerRotationYaw`（含 Pitch/Roll）；CMC 的 `bOrientRotationToMovement` 也设为 `false`——旋转交给组件。

7. **从玩法侧驱动：**

   ```cpp
   USigilMovementSystemComponent* MSC =
       USigilMovementSystemComponent::GetMovementSystemComponent(Character);

   MSC->SetDesiredMovement(SigilMovementStateTags::Sprint);          // 走/跑/疾跑
   MSC->SetDesiredRotationMode(SigilRotationModeTags::VelocityDirection);
   MSC->SetOverlayMode(OverlayTag);                                  // Sigil.Movement.OverlayMode.*
   MSC->SetMovementSet(WeaponSetTag);                                // Sigil.Movement.Set.*（项目自建标签）
   MSC->PushAvailableMovementDefinition(GreatswordDefinition);       // 运行时定义栈
   ```

## 关键类型

| 类型 | 说明 |
| --- | --- |
| `USigilMovementSystemComponent` | 抽象基类组件：复制的核心状态（集/状态/旋转/移动/叠层）、标签容器 + `GameplayTagsProvider`、定义栈、状态变化事件。 |
| `USigilCharacterMovementSystemComponent` | Character/CMC 实现：把运动状态设置写入 CMC，地面与空中旋转系统、视角平滑、距离匹配的预测参数。 |
| `USigilMoverMovementSystemComponent` | Mover 插件的 WIP 占位，勿用。 |
| `USigilMovementDefinition` | Const 数据资产：`MovementSets` 映射（运动集标签 → `FSigilMovementSetSetting`）。 |
| `FSigilMovementSetSetting` | 单个运动集：控制设置（可按叠层模式覆写）、通用动画设置、五套动画层设置，以及可扩展的 `UserSettings`（`USigilMovementSetUserSetting`）。 |
| `USigilMovementControlSetting_Default` | 逻辑数据资产：`MovementStates` 速度表、`JumpStates`、移动阈值、空中旋转模式，以及 `OnMovementStatesUpdated` / `OnJumpStatesUpdated` 广播委托。 |
| `FSigilMovementStateSetting` | 单个步态：标签、速度等级、分方向速度、加/减速度、允许的旋转模式、旋转插值参数。 |
| `USigilAnimGraphSetting` | 每骨架一份的数据资产：动画层设置 → 动画层类的映射、朝向扭曲骨骼引用。 |
| `USigilMainAnimInstance` | 必需的主动画实例：刷新并 Link 动画层，暴露线程安全的走跑/视角/倾斜/空中状态与节点相关性标签。 |
| `USigilAnimLayer` | 所有 Linked 动画层的基类；`ApplySetting`/`ResetSetting`、`OnLinked`/`OnUnlinked`。只能挂在 `USigilMainAnimInstance` 之下。 |
| `USigilAnimLayer_States_DefaultLocomotion` | 原生类 Lyra 地面走跑层（Idle、Start、Cycle、Stop、Pivot、原地转身、跳/落）——用 ABP 继承使用。 |
| `USigilAnimLayer_Overlay_Stack` / `USigilAnimLayer_Overlay_PoseBased` | 两套原生叠层实现。 |
| `USigilAnimLayer_View_Default` | 原生瞄准偏移视角层。 |
| `USigilConstants` | 规范名称常量：`TurnInPlace` Slot，`RotationYawSpeed`、`RotationYawOffset`、`AllowTurnInPlace`、`AllowAiming` 曲线。 |
| `USigilUtility` | 蓝图工具库，含 PoseSearch/Chooser 旁路工具（`EvaluatePoseSearchDatabasesChooser`、`IsValidPoseSearchDatabasesChooser`）。 |

## 配置方式

全部走资产配置（没有 `DeveloperSettings`）。链条从粗到细：

1. **`USigilMovementDefinition`**——项目里有哪些运动集。多个定义可在运行时叠栈；同一个集标签，后压入的定义优先。
2. **`FSigilMovementSetSetting`**——每个集内部：控制设置（`bControlSettingPerOverlayMode` + `ControlSettings` 可按叠层模式各配一份）、共享动画数据（`FSigilAnimDataSetting_General`：根骨旋转偏移、倾斜速度、地面预测通道等）、五套层设置。States/Overlay 两层可选择内联实例（`bUseInstancedStatesSetting` / `bUseInstancedOverlaySetting`）或引用共享数据资产以便跨集复用。
3. **`USigilAnimGraphSetting`**——每套骨架一份。自定义层设置/实现要在 `AnimLayerSettingToInstanceMapping` 里注册。
4. **`USigilMovementSetUserSetting`**——继承它给运动集挂任意自定义数据，在动画层里用 `USigilUtility::GetMovementSetUserSetting` 读取。

数据资产实现了编辑器期校验（`IsDataValid`、`PreSave` 生成加速映射），大部分配置错误会在编辑器里提前暴露。

## 网络

- **会同步的：** `MovementDefinitions`（栈）、`MovementSet`、`DesiredRotationMode`、`DesiredMovementState`、`OwnedTags`、`OverlayMode`、`InputDirection`（量化）、`DesiredVelocityYawAngle`、`ReplicatedViewRotation`。各 Setter 内部走成对的 `Server*`/`Client*` 可靠 RPC，任一端都能发起变更；视角旋转走不可靠 RPC 并带网络平滑。
- **纯本地的：** `LocomotionMode`、`MovementState`、`RotationMode`（Transient，各端自行推导）、整个 locomotion/view 状态块，以及 AnimInstance 与各动画层内部的一切。
- **没有移动预测整合**：组件不扩展 `FSavedMove`/CMC 预测，只是配置标准 `UCharacterMovementComponent`，预测仍由引擎自带机制完成。

## 已知留白与扩展点

在此之上做产品前先了解这些（均已在源码核实，属刻意设计或当前限制）：

- **跳跃参数只广播、不落地。** `FSigilJumpStateSetting`（重力倍率、空中控制等）只会经 `USigilMovementControlSetting_Default::OnJumpStatesUpdated` 广播出来，插件不会把它们写进 CMC——需要项目自己绑定委托并应用。
- **Mover 支持是空壳**（`USigilMoverMovementSystemComponent`）。
- **蹲伏、游泳、攀爬不是内建状态。** `Sigil.Movement.LocomotionMode.Swimming/Flying` 标签存在、CMC 模式也能映射过去，但没有任何游泳/飞行/蹲伏的控制与动画逻辑；Idle 动画数据里只是预留了可选的 `CrouchEntry`/`CrouchExit` 序列位。
- **Additive 与 SkeletalControls 层是空基类**——纯扩展点。
- `bAllowRefreshCharacterMovementSettings = false` 搭配 `SpeedToMovementStateCurve`，可让外部系统（如 AI）掌控 CMC 速度，由 GMS 反过来按实际速度推导运动状态。

## 相关文档

- [sigil.input](sigil-input.zh-CN.md)——标签驱动的输入层，与 `SetDesiredMovement` / `SetDesiredRotationMode` 天然搭配。
- [sigil.inventory](sigil-inventory.zh-CN.md)——装备事件是触发 `PushAvailableMovementDefinition` / `SetMovementSet` 的天然时机。
