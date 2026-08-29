[English](CHANGELOG.md) | [简体中文](CHANGELOG.zh-CN.md)

# 变更日志

Sigil 尚在 1.0 之前：次版本可能包含破坏性变更。每条列出公开 API 变化及迁移方式。

## 0.1.x（未发布）— 审查加固

### 破坏性变更

| 变更 | 迁移 |
|---|---|
| **sigil.combat** — 删除 `USigilAttackResultProcessor_Death`（无任何行为）。 | 子类化 `USigilAttackResultProcessor`（参考 `_GameplayEvent` / `_GameplayCue`）实现自己的死亡契约。 |
| **sigil.combat** — `ServerPlayPredictableMontageForTarget` 不再 `BlueprintCallable`。 | 改调 `PlayPredictableMontageForTarget`；它会分配请求序号、本地预测并发送 RPC。 |
| **sigil.combat** — 可预测蒙太奇的默认授权现在拒绝除发起者自身以外的所有目标。 | 在项目设置里把 `MaxPredictableMontageTargetDistance` 设为大于 0（允许同世界、该距离内的目标），或覆写 `CanPlayMontageOnTarget`。 |
| **sigil.combat** — 可预测蒙太奇请求必须是线性蒙太奇（无循环 / 非线性 Section 图）；倍率须在 `Min/MaxPredictableMontagePlayRate` 内（默认 0.1–4.0）。 | 把循环蒙太奇拆成线性资产，或不走可预测路径。 |
| **sigil.combat** — `FSigilPlayMontageRequest` 新增 `RequestId`（只读、自动分配）；`FSigilReplicatedMontageInfo` 新增 `StartTimeSeconds`、`RequestId`、`RootTranslationScale`。 | 除非自行序列化这些结构，否则无需改动。 |
| **sigil.combat** — `USigilCombatSystemComponent::PlayPredictedMontage` 现在返回 `bool`。 | 原调用方忽略返回值即可。 |
| **sigil.gas** — `FSigilAttributeGroupName::GetName()` 的子组编码改为 `Main->Sub`（原 `Main.Sub` 会被引擎误解析）。 | CurveTable 行名从 `Main.Sub.Set.Attr` 改为 `Main->Sub.Set.Attr`。 |
| **sigil.gas** — `WhenPhaseStartsOrIsActive` / `WhenPhaseEnds` 返回 `FSigilGamePhaseObserverHandle`（原 `void`，短暂为 `FDelegateHandle`）；`RemovePhaseObserver` 接受该句柄且 `BlueprintCallable`；蓝图节点同样返回句柄。 | 需要注销就保存返回句柄；否则无需改动。 |
| **sigil.gas** — `WhenPhaseStartsOrIsActive` 的即时通知现在遵循 `MatchType`，并传入实际活动的阶段 Tag。 | 只有子阶段活动时注册的 `ExactMatch` 观察者不再被即时触发。 |
| **sigil.ui** — 删除 `USigilGameUIExtensionPointWidget::CheckPlayerState()`（与重试路径重复）。 | 无需调用；注册会自动重试。 |
| **sigil.ui** — `USigilGameModalWidget::SetupModal` 返回 `bool`；设置失败的模态在激活时以 `Unknown` 自关。 | 通过 `USigilAsyncAction_ShowModel` 的调用方无需改动。 |
| **sigil.movement** — 删除 `FSigilJumpStateSetting::bIsShowDebug`。 | 删掉蓝图里对该字段的读取。 |
| **sigil.movement** — `bDynamicPlayRate = false` 现在真正关闭动态倍率（固定 1.0）。 | 复查曾把该开关关掉的循环动画数据。 |
| **sigil.movement** — `USigilUtility::CalculateAnimatedSpeed` 不再打日志；输入不可用时返回 0。 | 按资产缓存结果（默认 locomotion 层已这样做）。 |

### 修复

- **GAS-01 — sigil.gas：**`USigilAbilityCost::CheckCost` 在可选相关 Tag 缺失时改用本次调用局部的空 Tag 容器，避免蓝图 Cost 事件解引用空指针。
- **CBT-01 — sigil.combat：**`AddEntry` 会在 Flow 回调前把服务端存储项设为已消费并标脏，客户端 `ConsumeEntry` 则先把本地存储项设为已消费；回调收到未消费的值副本，延迟或重入消费都只会分发一次。
- **INVC-01 — sigil.inventory：**反序列化缺失物品的存档时会以 Stack 与 Item 标识给出警告、跳过该栈并继续后续有效栈，不再向 Map 插入空项。
- **INVC-02 — sigil.inventory：**装载配置的 Server RPC 改为仅调用一次既有本地 `LoadDefaultLoadouts()` 实现，不再经 RPC 包装器递归分发。
- **INVG-01 — sigil.inventory：**拾取改用目标集合的实际数量差，而不信任集合返回量；来源无法移除的差额会从目标回滚，来源余量得到保留，且仅有非零守恒转移才报告成功。
- **INVG-02 — sigil.inventory：**制作仅在每种请求材料都成功移除后返回 `true`；既有的首次失败即停止与非事务语义保持不变。
- **INVG-03 — sigil.inventory：**随机掉落安全构造累计权重；空或非正总权重返回空结果；掉落数量落在配置的闭区间内，末尾边界选择保持在有效范围内。
- **CAM-01 — sigil.camera：**激活但为空的相机栈不再产出视图；组件会保留已有 Camera/SpringArm 状态和待应用 FOV offset，直到有效模式求值成功。

### 自动化覆盖

- 本批共新增 17 项 Automation：`SigilGas.AbilityCost`（1 项）、`SigilCombat.AttackResult`（3 项）、`SigilInventory`（10 项：反序列化/装载 2 项、部分拾取 3 项、制作 2 项、随机掉落 3 项）及 `SigilCamera.Stack`（3 项）。最终 `SigilInventory` 过滤器执行 12 项，因为其中还包含 2 项既有拾取回归。
- 对应聚焦改动的 HostEditor Win64 Development 构建已成功；这不表示已执行完整测试套件或 ProjectSpecterEditor 构建。

### 已知空白

- 本批未修改 INVC-11 的多集合反序列化路由。
- 本批未修改 INVC-12 的多栈 `AddInternal` 返回值语义；拾取路径改为直接测量目标数量，不再依赖该返回量。
- 本批未执行 PIE、多人运行时、Win64 Cook、打包运行时或 ProjectSpecterEditor 验证。
