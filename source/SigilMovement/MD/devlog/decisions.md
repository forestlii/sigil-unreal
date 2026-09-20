# SigilMovement 决策记录

## 2026-09-02 · PS-PLAYER-LOCOMOTION-001 Task 1

- `RefreshMovementState()` 不再以 `MovementState == DesiredMovementState` 提前返回。Desired 表示玩家目标档位，Actual 必须持续按当前 `LocomotionState.Speed` 解析，因此静止、减速或受阻时可以从 Jog/Sprint 解析到 Walk，同时不改写 Desired。
- 从 Tick 刷新路径移除 `ApplyMovementSetting()`。期望档位的 CMC 参数由 `SetDesiredMovement()` 与 MovementSet/ControlSetting 切换负责；Tick 重复 Apply 会反复写 CMC 并重播配置广播，而 Actual 解析只需调用 `SetMovementState()`。
- 新增 `USigilSecondaryAnimInstance`，直接继承 `UAnimInstance`，不继承或注册 `USigilMainAnimInstance`。Game Thread 从同 Pawn 的 `USigilMovementSystemComponent` 拷贝状态快照；`NativeThreadSafeUpdateAnimation()` 不读取 Pawn 或组件，只消费本实例字段。无 Owner/组件时重置为安全默认并只记录一次诊断。
- 为只读复制既有私有 `FSigilLocomotionState`，经总控 2026-09-02 裁决，在 `SigilMovementSystemComponent.h` 仅增加 Secondary 前置声明和 friend。未公开 getter、未新增通用 snapshot API，避免扩大 L1 公共接口。
- 验证：TDD RED 覆盖实际步态早退和缺失 Secondary 类；GREEN 后 `SigilMovement.Runtime` Automation 发现 5 个测试且全部成功，ProjectSpecterEditor Win64 Development Build 成功。`UnrealEditor-Cmd.exe` 在测试运行前被 LinuxArm64/VisionOS SDK 校验阻断；使用同引擎 `UnrealEditor.exe` 配合相同 Automation 参数取得运行时证据。
- 未运行：PIE/人工双身体表现、多人/网络、延迟、坏资产、Cook 与打包运行时；本任务未修改 UE 二进制资产。

## 2026-09-20 · 攀爬翻越检测（Traversal）第一版

来源：ProjectSpecter 提案 `MD/production/architecture/proposals/traversal-system.md`，Likeon 2026-09-20 批准「通用部分现在就进 Sigil」。

- 新增 `Public/Traversal/`：`SigilTraversalTypes.h`（动作枚举、检测输入、边缘、结果、判定阈值）、`SigilTraversableInterface.h`、`SigilTraversalLibrary.h`。只有判定，不含移动与动画：怎么把角色送过去由消费项目决定（ProjectSpecter 第一版用代码驱动的路径插值）。
- 障碍物自己回答边缘在哪（`ISigilTraversableInterface::GetTraversalLedges`），检测不去猜几何。`FindTraversable` 依次找：被命中的组件、其 Actor、该 Actor 上实现接口的组件。`ComputeBoxLedges` 给盒体积障碍物用，任意旋转与缩放都成立。
- `ClassifyAction` 是四行判定表加高度上限，默认阈值取自参考设计（薄障碍进深小于 59；后边缘落差大于 50 为跨栏、小于 10 为攀上；无落脚地面为翻越；进深大于 29 可当平台；地面最高 275、空中最高 200、地面跨栏与翻越最高 125）。阈值全在 `FSigilTraversalRules` 里，消费项目可改。
- 前探胶囊与角色胶囊分开：前探用小胶囊掠过地面与台阶，空间检查用角色自身胶囊（站立放不下再试蹲姿，结果带 `bShouldCrouch` 与 `FitHalfHeight`）。前探起点就贴着障碍物时没有可用的命中点，改用角色位置。
- 「后方有无地面」只认不低于脚底 `BackFloorMaxDropBelowFeet`（默认 50）的地面：外面有落差的窗走翻越而不是跨栏。
- 验证：`SigilMovement.Traversal.Rules`、`SigilMovement.Traversal.BoxLedges` 两项 Automation 通过（在 ProjectSpecter 里随 ProjectSpecterEditor Win64 Development 编译后运行）；`CheckTraversal` 的世界检测由 ProjectSpecter 的 `ProjectSpecter.Traversal.*` 三项测试覆盖。Host 未构建。
- 未做：样条边缘组件（取代参考工程里窗用的边缘组件）、规则数据资产与动画选择、网络同步。未运行：PIE 人工手感、Cook、打包。

## 2026-09-20 · 攀爬翻越检测：样条边缘组件与前探越过不可攀物

来源：同上提案第 2.1 节；ProjectSpecter 的推拉窗需要沿摆好的样条翻越。

- 新增 `USigilTraversalLedgeComponent`（`Public/Traversal/SigilTraversalLedgeComponent.h`）：实现 `ISigilTraversableInterface`，按名字引用所属 Actor 上成对的样条（`FSigilTraversalLedgePair`），离角色最近的一条为前边缘、与它配对的为后边缘；`SetTraversalEnabled` 可在运行时关闭（关着的窗）。
- 法线不依赖样条上摆好的朝上向量：取「前边缘点 − 后边缘点」的水平方向；没有对边时取角色所在的一侧。几何部分是纯函数 `ComputePolylineLedges`，样条先采样成折线（默认 8 点，直边两点即精确）。
- `CheckTraversal` 的前探不再只认第一个命中：碰到不可攀的东西（或已关闭的可攀物）就忽略它再探，最多 4 次，且只接受距第一个阻挡物不超过 `FSigilTraversalCheckInputs::TraversableSearchDepth`（默认 50）的可攀物。原因：窗嵌在墙里、墙面与窗齐平时，先碰到的常是墙。空间检查仍只忽略角色自身，不会因此穿墙。
- 验证：新增 `SigilMovement.Traversal.PolylineLedges`，连同原两项在 ProjectSpecter 内通过；ProjectSpecter 里真实 PIE 对一扇打开的推拉窗按跳跃键，角色攀上窗台。Host 未构建。
- 未做：规则数据资产与动画选择、网络同步。
