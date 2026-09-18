# SigilInput 决策日志

> 本文件只记录 `SigilInput` 插件内部的设计取舍，不记录消费项目（如 ProjectSpecter）的玩法、资产或产品范围。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-09-18。

### [2026-09-18] 补齐 PlayerController 宿主：五个 C++ 生命周期 API + 私有有序拆除核心

- 阶段: 迭代
- 面临的选择: `OnRegister` 的 PC 分支原为 `// TODO 支持放到PC上。` 空实现，无 Pawn 时（前端菜单、登录）组件不可用。实现方式受 ProjectSpecter 已批方案 B 的设计合同约束（`MD/production/architecture/proposals/sigil-character-component-and-input-ownership.md` Revision `TD-PS-CHAR-INPUT-0.6-FINAL-L2-CANDIDATE` 第 6.2 / 6.2.1 / 6.5 节）。
- 定了什么: 新增且只新增五个 public C++ API：`BindPlayerControllerInput`、`UnbindPlayerControllerInput`、`SetGameplayRoutingEnabled`、`IsPlayerControllerInputBound`、`IsGameplayRoutingEnabled`，均不暴露为 Blueprint。所属 PC 在自己的 `SetupInputComponent` 里调用 Bind；组件在 `OnRegister` 时若 PC 已有 InputComponent 也会补绑一次（与 Pawn 路径"注册晚于 restart 时补调"同理）。Bind 先做只读 preflight（PC 宿主、本地 PC、LocalPlayer、EnhancedInputComponent、EI 子系统、InputConfig、当前 InputControlSetup），任一缺失即返回 false 且零副作用；同组件重复 Bind 为无操作成功；换组件先拆旧再建新；新绑定后 route 保持关闭，由消费方显式开启。
- 否掉了什么 + 为什么: 否掉"Bind 时直接加 IMC 并开路由"——合同要求 PC 宿主 fail-closed，Gameplay IMC 只在 route 开启时存在，由项目侧 readiness bridge 决定何时开启。否掉 Blueprint 暴露——合同明确避免 Widget/Pawn 任意重复绑定或绕过 teardown。
- 复用层🔑: ② 引擎相关
- 来源: Likeon 2026-09-18 决定"直接新起一个会话，修改 sigil 系列源码"（交接文档 `ProjectSpecter/MD/production/handoffs/2026-09-18/md/sigil-input-pc-host-handoff.md`）；上述提案第 6.2 节。

### [2026-09-18] Setup/Cleanup 改为 `final` 适配器，所有拆除汇入同一私有核心

- 阶段: 迭代
- 面临的选择: 既有 protected virtual `SetupInputComponent/CleanupInputComponent` 允许派生类不调 `Super` 而另写一套句柄/IMC/瞬态，与合同"lifecycle 调用图不可绕过"冲突。
- 定了什么: 两者保留名称与签名但标 `final`：PC 宿主分别转发到 Bind/Unbind；Pawn 宿主进入与 route-off 相同的私有 `TeardownGameplayRouteCore`（顺序固定：中和旧 receiver → 关 route → 移除自有 IMC → `ResetTransientInputStateAfterOrderedTeardown`），再由私有 `ReleaseInputBindingsCore` 只按句柄移除自有绑定。`OnUnregister` 与新增的 `EndPlay` 也走同一路径。派生扩展只能用既有 `OnSetup/OnCleanupPlayerInputComponent` 钩子与新的 protected `NeutralizeGameplayReceiver`。
- 否掉了什么 + 为什么: 否掉保留 virtual——无法阻止派生类绕开核心。已扫描：Sigil 各插件与 ProjectSpecter 均无这两个函数的 override（ProjectSpecter 只 override `OnCleanupPlayerInputComponent_Implementation`），`final` 不破坏现有消费者编译【ProjectSpecter 侧编译待验证】。
- 复用层🔑: ① 通用
- 来源: 提案第 6.2 节调用图、AC-INPUT-011；`grep` ProjectSpecter `Source/`（2026-09-18）。

### [2026-09-18] 绑定所有权按句柄登记；外部预置 Value Binding 视为借用

- 阶段: 迭代
- 面临的选择: 原实现按数组 index 记录并删除 Value Binding（删一个后 index 漂移），且会删掉别人预置的同 Action binding；Event Binding 在 `OnSetupPlayerInputComponent` 默认实现里绑定、没有句柄记录。
- 定了什么: 绑定核心自己创建 5 个 TriggerEvent 的 Event Binding 并登记句柄；Value Binding 先查同 Action 是否已存在（UE5.8 `BindActionValue` 对已存在 Action 返回原 binding），已存在则为借用、不登记，新建才进 owned 集；清理一律 `RemoveBindingByHandle`，不调用全局 `ClearActionBindings`。`BindInputActions()` 保留为兼容空操作（事件绑定已由核心完成，重复调用不再叠加）。`InputActionValueBindings` 的 value 从 index 改存句柄。
- 否掉了什么 + 为什么: 否掉 `ClearBindingsForObject(this)` 作为主清理——会误伤同对象其他绑定语义，合同只允许作兜底。
- 复用层🔑: ② 引擎相关
- 来源: 引擎 `EnhancedInputComponent.h` `BindActionValue` / `RemoveBindingByHandle`（UE5.8 本机源码）；提案第 6.2 节第 3 条、AC-INPUT-012。

### [2026-09-18] 路由关闭即 fail-closed；按住的输入只对原 receiver 中和一次

- 阶段: 迭代
- 面临的选择: route 关闭期间的输入怎么处理；关闭/换 Pawn 时仍按住的 Sprint/Ability 如何收口；Pawn 宿主是否也受 route 影响。
- 定了什么: `InputActionCallback` 在 route 关闭时直接丢弃（不检查、不缓冲、不广播、不记录），开启后不回放。已路由的 `Started` 记入 held 集与 `RoutedGameplayReceiver`（弱引用），`Completed/Canceled` 移除。拆除时先快照并清空 held 集再调 `NeutralizeGameplayReceiver`，保证重复拆除不重复释放；默认实现只在旧 receiver 仍是当前受控 Pawn 时经当前 Setup 派发 `Canceled`（绕过 Checker），否则跳过——绝不把旧 release 发给新 Pawn，服务器权威取消交给 Pawn/ASC 生命周期。IMC 重加使用 `bIgnoreAllPressedKeysUntilRelease=true`。Pawn 宿主为兼容旧行为，在 Setup 成功后自动开启 route。
- 否掉了什么 + 为什么: 否掉"关闭 route 时对当前 Pawn 合成 Completed"——换 Pawn 场景会命中新 Pawn；否掉 Pawn 宿主默认关闭 route——会让现有 Pawn 消费者全部失效。
- 复用层🔑: ① 通用
- 来源: 提案第 6.2 节第 5–8 条、不变量 7/11。

### [2026-09-18] Checker 与 Gameplay Debugger 以 `GetControlledPawn()` 为玩法主体

- 阶段: 迭代
- 面临的选择: `USigilInputChecker_TagRelationship::GetActorTags` 原读 `IC->GetOwner()`，PC 宿主下会拿 PC（无标签）当主体；Debugger 直接 `OwnerPC->GetPawn()->GetName()`，Pawn 为空时崩溃，且组件在 PC 上时按 Pawn 目标查不到。
- 定了什么: Checker 改为只查 `IC->GetControlledPawn()` 的 `IGameplayTagAssetInterface`，IC/Pawn 为空返回空集。Debugger 新增 `ResolveInputSystem`（先查目标自身，目标是 Pawn 时再查其 Controller，不借用 viewer 的 OwnerPC）与 `GetDisplayName`（无 Pawn 时显示 `宿主名 (NoPawn)`），`CollectData` 每次先清空 DataPack，`DrawInputEntries` 复用同一 resolver。Pawn 宿主下 `GetControlledPawn()` 即 Owner，行为不变。
- 否掉了什么 + 为什么: 否掉 Debugger 回退到 viewer Pawn——多玩家时会显示别人的输入。
- 复用层🔑: ① 通用
- 来源: 提案第 6.2.1、6.5 节，AC-INPUT-007 / 010。
