---
name: sigil-movement
description: 在接入、修改或排查 SigilMovement 的运行时初始化、移动配置、旋转控制权、Locomotion、配套动画节点，或攀爬翻越检测（Traversal：可攀接口、前探与空间检查、动作判定、盒体积与样条边缘）时读取。
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

## 攀爬翻越检测（Traversal，核对基线 0f1212d52635de5f0da3db1ff8a1fdd720e44827）

入口：Public/Traversal/ 下 SigilTraversalTypes.h、SigilTraversableInterface.h、SigilTraversalLibrary.h、
SigilTraversalLedgeComponent.h 及对应 Private 实现；测试在 Private/Tests/SigilTraversalTest.cpp。
设计取舍与阈值来源见 source/SigilMovement/MD/devlog/decisions.md 的两条 2026-09-20 记录，这里只列约束：

- 只做判定，不含移动与动画；把角色送过去由消费项目决定。
- 障碍物自己回答边缘在哪（ISigilTraversableInterface::GetTraversalLedges）。查找顺序：被命中的组件、其 Actor、该 Actor 上实现接口的组件。
  边缘法线水平、指向障碍物外侧。盒体积用 USigilTraversalLibrary::ComputeBoxLedges，样条用 USigilTraversalLedgeComponent（按名字配对，可运行时开关）。
- CheckTraversal 走 ECC_Visibility。前探胶囊与角色胶囊分开传（FSigilTraversalCheckInputs 的 Trace* 与 Capsule*）；
  前探碰到不可攀物会忽略它再探，最多 4 次，且只接受距第一个阻挡物 TraversableSearchDepth 以内的可攀物；空间检查只忽略角色自身。
- ClassifyAction 是四行判定表加高度上限，阈值全在 FSigilTraversalRules，除 CheckTraversal 外都是纯函数，可无世界单测。
- 未做：规则数据资产与动画选择、网络同步。Host 未构建；测试在消费项目 ProjectSpecter 内运行通过。

## 处理当前任务

先区分初始化模式、配置是否齐备、运行是否激活和旋转控制权。
按实际调用追踪 MovementDefinition、动画实例和配置刷新，
不要把组件存在当作配置运行已经启动。
动画节点任务保持 Runtime 与 Editor 模块边界。

## 范围

覆盖现有移动与配套动画机制。
翻越只提供判定，不预设游戏如何移动角色，也不预设钩爪或其他动作；不把 WIP Mover 当作成熟接入方案；
动画资产及许可由消费项目另行处理。
