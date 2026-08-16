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

详见 2026-08-17 两轮审查批次的 `fix(...)` 提交：蒙太奇 RPC 校验 / 限频 / 回滚协议、AttackResult FastArray 标脏与重入、计时器生命周期、GamePhase 判空、相机 BlendInfo / FOV Offset / 穿模规避、模态失败生命周期、Movement 倍率安全与线程安全，以及两个非编辑器构建修复。

### 已知空白

- 尚无自动化测试（计划：FastArray、蒙太奇协议、属性组、UI 时序、Movement 速度、相机栈）。
- 运行时 / 联机行为经过推演与编译，未在 PIE 或多客户端会话中实测。
