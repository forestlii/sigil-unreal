---
name: sigil-movement
description: 在接入、修改或排查 SigilMovement 的运行时初始化、移动配置、旋转控制权、Locomotion 或配套动画节点时读取。
---

# Sigil 移动与动画配套

源码核对基线：9ffca225f3313e93932cadb1ecd56ec7e7fd7ea2。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilMovement/Source/SigilMovement/。

- Public/SigilCharacterMovementSystemComponent.h 与对应 Private 实现。
- Public/SigilMovementSystemComponent.h 与对应 Private 实现。
- Public/Locomotions/SigilMainAnimInstance.h 及对应实现。
- Public/Locomotions/SigilSecondaryAnimInstance.h 及对应实现（次级 Mesh 的只读移动快照）。
- 动画图编辑问题再读
  source/SigilMovement/Source/SigilMovementEditor/。
- Public/SigilMoverMovementSystemComponent.h 标为 WIP、不要使用。

## 已核实的接口约束

- 初始化模式包含 Strict 与 DeferredUntilConfigured。
- 旋转控制权包含 SigilMovement、Controller、
  MovementDirection 和 External。
- CharacterMovementSystemComponent.BeginPlay 应用旋转控制权，
  随后锁定该设置。
- DeferredUntilConfigured 分支不会直接走 Strict 的运行启动路径；
  配置运行尚未激活时，Tick 跳过后续配置驱动的移动处理。
- StartConfiguredRuntime 绑定 MovementModeChanged；
  EndPlay 移除对应绑定并清除运行激活状态。
- SigilMovement 持有旋转控制权时，BeginPlay 检查
  Pawn 的 Controller Rotation 设置未同时启用。
- RefreshMovementState 不再因 MovementState 等于 DesiredMovementState 提前返回；
  实际档位按 LocomotionState.Speed 持续解析，不改写 Desired。
  Tick 刷新路径不再调用 ApplyMovementSetting，期望档位的移动参数由
  SetDesiredMovement 与 MovementSet/ControlSetting 切换负责。
- SigilSecondaryAnimInstance 直接继承 UAnimInstance，不继承或注册 MainAnimInstance。
  Game Thread 从同 Pawn 的 SigilMovementSystemComponent 拷贝 MovementSet、MovementState、
  LocomotionMode、RotationMode、OverlayMode、输入方向、Tags、LocomotionState 与 ViewState；
  NativeThreadSafeUpdateAnimation 不读取 Pawn 或组件。
  缺 Owner 或组件时重置为空快照，只记录一次警告。
  来源：source/SigilMovement/MD/devlog/decisions.md。

## 处理当前任务

先区分初始化模式、配置是否齐备、运行是否激活和旋转控制权。
按实际调用追踪 MovementDefinition、动画实例和配置刷新，
不要把组件存在当作配置运行已经启动。
动画节点任务保持 Runtime 与 Editor 模块边界。

## 范围

覆盖现有移动与配套动画机制。
不预设游戏翻越、钩爪或其他动作，也不把 WIP Mover 当作成熟接入方案；
动画资产及许可由消费项目另行处理。
