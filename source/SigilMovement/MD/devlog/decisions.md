# SigilMovement 决策记录

## 2026-09-02 · PS-PLAYER-LOCOMOTION-001 Task 1

- `RefreshMovementState()` 不再以 `MovementState == DesiredMovementState` 提前返回。Desired 表示玩家目标档位，Actual 必须持续按当前 `LocomotionState.Speed` 解析，因此静止、减速或受阻时可以从 Jog/Sprint 解析到 Walk，同时不改写 Desired。
- 从 Tick 刷新路径移除 `ApplyMovementSetting()`。期望档位的 CMC 参数由 `SetDesiredMovement()` 与 MovementSet/ControlSetting 切换负责；Tick 重复 Apply 会反复写 CMC 并重播配置广播，而 Actual 解析只需调用 `SetMovementState()`。
- 新增 `USigilSecondaryAnimInstance`，直接继承 `UAnimInstance`，不继承或注册 `USigilMainAnimInstance`。Game Thread 从同 Pawn 的 `USigilMovementSystemComponent` 拷贝状态快照；`NativeThreadSafeUpdateAnimation()` 不读取 Pawn 或组件，只消费本实例字段。无 Owner/组件时重置为安全默认并只记录一次诊断。
- 为只读复制既有私有 `FSigilLocomotionState`，经总控 2026-09-02 裁决，在 `SigilMovementSystemComponent.h` 仅增加 Secondary 前置声明和 friend。未公开 getter、未新增通用 snapshot API，避免扩大 L1 公共接口。
- 验证：TDD RED 覆盖实际步态早退和缺失 Secondary 类；GREEN 后 `SigilMovement.Runtime` Automation 发现 5 个测试且全部成功，ProjectSpecterEditor Win64 Development Build 成功。`UnrealEditor-Cmd.exe` 在测试运行前被 LinuxArm64/VisionOS SDK 校验阻断；使用同引擎 `UnrealEditor.exe` 配合相同 Automation 参数取得运行时证据。
- 未运行：PIE/人工双身体表现、多人/网络、延迟、坏资产、Cook 与打包运行时；本任务未修改 UE 二进制资产。
