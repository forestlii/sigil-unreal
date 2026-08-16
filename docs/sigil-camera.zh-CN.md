[English](sigil-camera.md) | [简体中文](sigil-camera.zh-CN.md)

# sigil.camera

**插件：** `SigilCamera` · **模块：** `SigilCamera`（Runtime） · **依赖：** Gameplay Tags（引擎模块；无引擎插件依赖）

sigil.camera 是一套 Lyra 风格的相机模式栈，但架构上有一个关键差异：它不像 Lyra 那样直接输出最终的 `FMinimalViewInfo`，而是**驱动你 Pawn 上现成的 `USpringArmComponent` + `UCameraComponent` 的参数**。相机模式在栈上按各自的混合曲线过渡，另附一个移植了 Lyra 触须射线逻辑的穿模规避基类。

## 概述（Overview）

### 模式栈驱动 SpringArm

`USigilCameraSystemComponent`（`ActorComponent`）持有一个 `USigilCameraModeStack`，每帧做三件事：

1. 轮询 `DetermineCameraModeDelegate`（纯 C++ 委托，返回 `TSubclassOf<USigilCameraMode>`），有绑定就把返回的模式压栈。
2. 求值整个栈，混合出一个 `FSigilCameraModeView`。
3. 把结果写到关联组件上：`ControlRotation` → Pawn 的 PlayerController，`FieldOfView` → 相机组件，`SpringArmLength` → `TargetArmLength`，`SpringArmSocketOffset` → `SocketOffset`，`SpringArmTargetOffset` → `TargetOffset`。

视图结构里还有 `Location`、`Rotation` 两个字段，参与混合但 Tick **不会**把它们写到任何组件上——见"已知限制"。

也可以显式切模式：`PushCameraMode(Class)`，或 `PushDefaultCameraMode()`（压入可编辑属性 `DefaultCameraMode`）。`GetBlendInfo` 返回栈顶模式的 `CameraTypeTag` 和当前权重，玩法代码不用知道具体模式类就能问"现在是不是瞄准视角"。

### 相机模式

`USigilCameraMode` 是 `Abstract, Blueprintable` ——**插件不带任何可用的具体相机模式**，项目必须自己继承（通常用蓝图）。核心要实现的是 `BlueprintNativeEvent` `OnUpdateView(DeltaTime, PivotLocation, PivotRotation)`：把想要的相机状态写进 `View` 属性。`GetPivotLocation` / `GetPivotRotation` 同样是 `BlueprintNativeEvent`，原生默认实现取 Character 胶囊体中心（考虑蹲伏），或回退到 Pawn 的视角位置/旋转。

每个模式携带：`FieldOfView`、`ViewPitchMin` / `ViewPitchMax`、混合设置（`BlendTime`、`BlendFunction`——Linear / EaseIn / EaseOut / EaseInOut——和 `BlendExponent`）、`CameraTypeTag` 游戏标签、`MaxActiveTime`（激活时长超过它就回落到默认模式）。模式进出栈时会触发 `OnActivation` / `OnDeactivation` 蓝图事件。

栈内做了实例池（`CameraModeInstances`）：每个模式类只实例化一次、反复复用，模式内部状态在两次激活之间会保留。

### 穿模规避

`USigilCameraMode_WithPenetrationAvoidance`（同样 `Abstract, Blueprintable`）移植了 Lyra 的触须式碰撞逻辑：`FSigilCameraPenetrationAvoidanceFeeler` 射线数组（第 0 条是主射线，第 1 条起在 `bDoPredictiveAvoidance` 开启时做预测规避）、`bPreventPenetration`、`PenetrationBlendInTime` / `PenetrationBlendOutTime`、`CollisionPushOutDistance`、`ReportPenetrationPercent`。注意这套逻辑**不会自动运行**——类只是把 `UpdatePreventPenetration(DeltaTime)` 暴露为 `BlueprintCallable`，需要你在自己的 `OnUpdateView` 里调它。

配合的 Actor 可以实现 `ISigilCameraAssistInterface`（纯 C++ 虚函数）：`GetIgnoredActorsForCameraPentration`、`GetCameraPreventPenetrationTarget`、`OnCameraPenetratingTarget`。带 Actor 标签 `IgnoreCameraCollision` 的 Actor 会被触须射线跳过。

### 坐标归属与穿模规避

**最终相机变换由 SpringArm 拥有**：系统组件每 Tick 只写 ControlRotation、FOV、臂长和两个臂偏移；`View.Location` / `View.Rotation` 是弹簧臂悬挂的枢轴，不是相机位置。`UpdatePreventPenetration` 遵循同一模型——它算出弹簧臂将产生的相机位置，沿瞄准线做触须探测，再按被遮挡比例**缩短 `View.SpringArmLength`**。请在你的 `OnUpdateView` 填完 View 后调用它，并**关闭 SpringArm 自带的 `bDoCollisionTest`**，避免两套系统叠加推近。

`AddFieldOfViewOffset` 是单帧偏移：在应用视图的那一帧加到 FOV 上然后清零，想持续生效就每帧重新调用。

### 已知限制（如实说明）

- 穿模规避是沿瞄准线按比例缩臂长的近似；Socket/Target 偏移越大近似越粗。尚未在 PIE 里验证（墙角 / 窄门 / 背墙三个场景待测）。
- `DetermineCameraModeDelegate` 是普通 C++ 委托，蓝图绑不了。纯蓝图项目请改用 `PushCameraMode` / `PushDefaultCameraMode`。

## 前置条件（Prerequisites）

- [ ] Pawn 上已经配好 **`USpringArmComponent` + `UCameraComponent`**（插件不会替你创建）。
- [ ] **必须手动调用 `Initialize(CameraComponent, SpringArmComponent)`**（例如在 `BeginPlay`）。这一步不做，Tick 会因关联组件为空而直接跳过——整个系统静默失效。
- [ ] **至少一个项目自制的相机模式**——蓝图（或 C++）继承 `USigilCameraMode` 或 `USigilCameraMode_WithPenetrationAvoidance` 并实现 `OnUpdateView`。插件本身没有可直接使用的模式。
- [ ] 可选：为 `CameraTypeTag` 准备游戏标签（普通标签即可，插件未声明任何标签）。

## 快速上手（Quick Start）

1. **做一个相机模式。** 蓝图继承 `USigilCameraMode`（或 `USigilCameraMode_WithPenetrationAvoidance`），实现 **OnUpdateView**：至少根据传入的 Pivot 设置 `View.SpringArmLength`、`View.SpringArmSocketOffset` / `SpringArmTargetOffset`、`View.ControlRotation`、`View.FieldOfView`。配好 `BlendTime` / `BlendFunction`。
2. **挂组件。** 给持有弹簧臂和相机的 Pawn 加 `USigilCameraSystemComponent`，`DefaultCameraMode` 指向你的模式类。
3. **初始化。** 在 Pawn 的 `BeginPlay` 里调用组件的 `Initialize(Camera, SpringArm)`，然后 `PushDefaultCameraMode()`。
4. **运行时切换。** 需要时调 `PushCameraMode(其它模式类)`（比如开镜时压入瞄准模式），栈会按新模式的混合设置做过渡；再压默认模式即可返回。C++ 项目也可以绑定 `DetermineCameraModeDelegate`，让组件每帧轮询。
5. **查询状态。** 用 `GetBlendInfo(OutWeight, OutTag)` 基于当前模式的 `CameraTypeTag` 驱动玩法逻辑；`GetCameraSystemComponent(Actor)` 定位组件。
6. **调试。** 控制台输入 `showdebug CAMERA`，组件注册了模式栈的调试绘制。

## 关键类型（Key Types）

| 类型 | 说明 |
| --- | --- |
| `USigilCameraSystemComponent` | 持有模式栈的组件；必须用相机 + 弹簧臂初始化；每帧应用混合结果。 |
| `USigilCameraMode` | 相机模式抽象基类（可蓝图化）；重写 `OnUpdateView`（可选重写 `GetPivotLocation` / `GetPivotRotation`）；携带 FOV、俯仰限制、混合设置、`CameraTypeTag`、`MaxActiveTime`。 |
| `USigilCameraModeStack` | 带按类实例池的混合栈，求值输出单个 `FSigilCameraModeView`。 |
| `FSigilCameraModeView` | 混合视图数据：`Location`、`Rotation`、`SpringArmSocketOffset`、`SpringArmTargetOffset`、`SpringArmLength`、`ControlRotation`、`FieldOfView`。 |
| `ESigilCameraModeBlendFunction` | `Linear`、`EaseIn`、`EaseOut`、`EaseInOut`。 |
| `USigilCameraMode_WithPenetrationAvoidance` | 附带 Lyra 式触须检测的抽象模式基类；需在 `OnUpdateView` 里调 `UpdatePreventPenetration`。 |
| `FSigilCameraPenetrationAvoidanceFeeler` | 单条触须射线：`AdjustmentRot`、`WorldWeight`、`PawnWeight`、`Extent`、`TraceInterval`。 |
| `ISigilCameraAssistInterface` | 可选 C++ 接口：穿模忽略列表、目标覆盖、贴脸回调。 |

## 配置（Configuration）

配置都在组件与模式类上，没有项目级设置：

- **组件上** —— `DefaultCameraMode`。
- **各模式类（类默认值）** —— `FieldOfView`、`ViewPitchMin` / `ViewPitchMax`、`BlendTime`、`BlendFunction`、`BlendExponent`、`CameraTypeTag`、`MaxActiveTime`；穿模模式另有 `PenetrationAvoidanceFeelers` 与各项开关。
- **单个 Actor** —— 加 Actor 标签 `IgnoreCameraCollision` 可让触须射线忽略它。

## 网络（Networking）

完全本地。模块内没有任何同步；相机模式在观察该 Pawn 的机器上求值。唯一的跨系统写入是对本地 PlayerController 调 `SetControlRotation`。

## 相关文档（See Also）

- [sigil.input](sigil-input.zh-CN.md) —— 通常由它触发相机模式切换的输入层。
- [sigil.ui](sigil-ui.zh-CN.md) —— 游戏 UI 层。
