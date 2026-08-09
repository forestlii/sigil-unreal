[English](sigil-combat.md) | [简体中文](sigil-combat.zh-CN.md)

# sigil.combat

**插件：** `SigilCombat` · **模块：** `SigilCombat`（Runtime） · **依赖：** [sigil.gas](sigil-gas.zh-CN.md)、Gameplay Abilities、Modular Gameplay、Targeting System、Motion Warping、Niagara

sigil.combat 是直接构建在 sigil.gas 之上的 GAS 多人战斗框架，提供：可复制的受击流水线（Combat Flow）、DataTable 驱动的攻击/子弹定义、对象池化的近战碰撞扫描、带穿透与多发弹道的池化子弹系统、基于引擎 Targeting System 的锁定目标、可预测的蒙太奇播放、阵营归属、武器 Actor、战斗属性集（CombatSet / PoiseSet），以及在网络上携带攻击定义的自定义 GameplayEffectContext。

> **这是框架，不是成品战斗玩法。** 有若干刻意留给项目实现的扩展点，本页均以「由项目实现」标注。

## 概述

### Combat Flow：受击流水线

`USigilCombatSystemComponent` 是每个角色的战斗中枢。它按 `CombatFlowClass` 实例化一个 **`USigilCombatFlow`**，该实例会复制到客户端（`OnRep_CombatFlow`）。

- `USigilCombatFlow` 是 **Abstract** —— **由项目实现**：必须为每种角色形态（人形、四足、机械……）做一个子类（通常是蓝图），并赋给 `CombatFlowClass`，否则组件没有流水线可跑。
- Flow 以 `ISigilAbilitySystemGlobalsEventReceiver` 身份接入全局效果管线：`HandlePreGameplayEffectSpecApply` 可以给到来的效果 Spec 追加动态 Tag（这一步依赖 Sigil 系 Globals 类，见「配置」）；`HandleGameplayEffectExecute` 响应在受击方执行的效果。
- 攻击命中后，`RegisterAttackResult` 把一条 `FSigilAttackResult` 写入可复制的 **`FSigilAttackResultContainer`**（`FFastArraySerializer`）。服务器与客户端（经 `PostReplicatedAdd`）都会执行 Flow 的 `HandleAttackResult`；默认实现把结果转发给内联的 **`USigilAttackResultProcessor`** 列表（`AttackResultProcessors`）——一个个小而可复用的反应步骤（硬直、击退、播 Cue……），按 Flow 配置。
- `FSigilAttackResult` 携带 `ImpactResult` Tag、`TaggedValues`、`EffectContextHandle`，以及聚合的来源/目标 Tag 容器。

> **由项目实现：伤害结算本体。** sigil.combat 只负责把攻击数据（Tag、SetByCaller 数值、效果上下文）写进效果 Spec；真正消费 `CombatSet` 属性和 sigil.gas 的 `IncomingDamage` meta 属性、最终扣 Health 的 GameplayEffect / `UGameplayEffectExecutionCalculation` 在项目侧。

### 攻击定义与攻击请求

- **`FSigilAttackDefinition`**（`FTableRowBase`）—— 每个攻击一行 DataTable：`AttackTags`（作为动态 AssetTags 加进效果 Spec）、`SetByCallerMagnitudes`（Tag→float）、`TargetEffectClass`/`TargetEffectClassLevel`、`TargetEffectContainer`、`TargetGameplayCues`、受击反应参数（`KnockbackDistance`、`KnockbackMultiplier`）、打击感参数（`HitStallingDuration`、`HitPlayRateFactor`），以及可扩展的 `UserSettings` 映射（`FInstancedStruct`，基结构 `SigilUserSetting`）。
- **`USigilAttackRequest_Base`** → `USigilAttackRequest_Melee`（`TracesToControl` Tag + `AttackDefinitionHandle` 行句柄）与 `USigilAttackRequest_Bullet`（`ESigilAbilityTargetingSourceType` 瞄准来源：相机/Pawn/武器/自定义）。请求是内嵌在动画通知状态里的内联对象。
- **动画通知状态** —— `USigilANS_AttackTrace`（近战）与 `USigilANS_BulletTrace`（远程）各持有一个内联 `AttackRequest`。**由项目实现**：这两个类是 `HideDropdown` 的数据容器，**C++ 里没有任何 Notify 逻辑**——需在蓝图里子类化并自己实现 NotifyBegin/End（开关对应的碰撞检测、生成子弹）。`USigilANS_MovementCancellation`（角色移动时禁用蒙太奇根运动）逻辑在 C++ 里已实现，但类是 Abstract，同样要先子类化才能摆上动画。
- **`USigilAbilityActionSetSettings`** —— Const 数据资产，按技能 Tag（+来源/目标 Tag 条件）用 `SelectBestAbilityActions` 挑选 `FSigilAbilityAction`；通过 `ISigilCombatInterface::QueryAbilityActions` 查询。

### 近战碰撞扫描

`USigilCollisionSystemComponent`（`UPawnComponent`）管理池化的 **`USigilCollisionTraceInstance`**，来源是 `FSigilCollisionTraceDefinition`（组件上的 `TraceDefinitions`，或运行时传给 `CreateTraceInstances`）。每个实例沿一个 `UPrimitiveComponent` 的插槽做扫描，可选用 `UTargetingPreset` 过滤候选，命中时广播 `OnHit` / 状态变化广播 `OnTraceStateChangedEvent`；实例会缓存复用（`CachedTraceInstances`）。两种驱动入口：

- `USigilAbilityTask_CollisionTrace::HandleCollisionTraces` —— 在技能里用；
- `USigilAsyncAction_CollisionTrace::SetupAndListenForCollisionTraceHit` —— 任意蓝图可用。

### 子弹系统（远程）

`USigilBulletSubsystem`（World Subsystem）负责生成与池化 **`ASigilBulletInstance`**（`SpawnBullets` / `TakeBulletFromPool` / `DestroyBullet`）。一切由 **`FSigilBulletDefinition`**（`FTableRowBase`）驱动：子弹类、`Duration`、多发弹道（`BulletCount`、`LaunchAngle`、`LaunchAngleInterval`、`LaunchElevationAngle`）、以 `AttenuationRange` 为界的速度/重力/命中半径变化、穿透开关（`bPenetrateCharacter`、`bPenetrateMap`）、VFX/SFX 槽位、关联的 `AttackDefinition` 行、命中/失效时的子弹链（`HitBulletDefinition` + `LaunchCondition` 条件 Tag），以及 `UserSettings` 扩展映射。发射条件原生 Tag：`Sigil.Combat.Bullet.LaunchCond.Always` / `.DidNotHitPawn` / `.HitPawn`。形状实现提供了 `ASigilSphereBulletInstance`。

### 锁定目标

`USigilTargetingSystemComponent`（`UPawnComponent`）维护复制的 `TargetedActor` 和仅服务器的 `PotentialTargets` 列表，按 `UTargetingPreset` 刷新（`bAutoUpdatePotentialTargets`）。API 有 `SearchForActorToTarget`、`StaticSwitchToNewTarget(bRightDirection)`、`SelectClosestActorFromPotentialTargets`、`FilterActorsWithPreset`，及可重写的 `CanBeTargeted`。插件为引擎 Targeting System 附带任务：过滤器 `USigilTargetingFilterTask_Affiliation`、`_IsDead`、`_TagsRequirements`、`_TraceInstance`；选择器 `USigilTargetingSelectionTask_LineTrace`、`_TraceExt`、`_TraceExt_BindShape`；另有 `ISigilTargetingSourceInterface` 与 `USigilTargetingFunctionLibrary`。

### 可预测蒙太奇

`USigilCombatSystemComponent::PlayPredictableMontageForTarget` 在目标身上播受击蒙太奇：发起端客户端先本地立即播放（`PredictedMontageInfo`），同时走 `ServerPlayPredictableMontageForTarget`，服务器再复制带触发时间的 `ReplicatedMontageInfo`，其他客户端据此对齐进度播放（`OnRep_ReplicatedMontageInfo`）。`FSigilPlayMontageRequest` 携带蒙太奇、`PlayRate`、`StartSectionName`、`RootTranslationScale`、`StartTimeSeconds`。

### 阵营、武器与角色接口

- **阵营** —— `ISigilCombatTeamAgentInterface` + `USigilCombatTeamAgentComponent`：复制的 `FGenericTeamId`（`CombatTeamId`，可经 `bAssignTeamIdToController` 同步给控制器），变更时广播 `OnTeamIdChangedEvent`。归属过滤器据此判断敌我；设置里的 `bDisableAffiliationCheck` 可在调试时放开跨阵营伤害。
- **武器** —— `ISigilWeaponInterface` + 抽象默认实现 `ASigilWeaponActor`（持有 Pawn、武器 Tag、激活状态、主 Primitive 组件、源对象）。
- **`ISigilCombatInterface`** —— **由项目实现**在角色上：战斗目标读取、`QueryAbilityActions`、`QueryWeapon` / `SigilGetWeapon`、防御键状态、旋转/移动模式与状态读写，以及 `StartDeath` / `FinishDeath` / `IsDead` 死亡生命周期。用 `USigilCombatFunctionLibrary::GetCombatInterface` 访问。

### 战斗属性与效果上下文

- **`USigilCombatSet`** —— `Damage`、`DamageNegation`、`GuardDamageNegation`、`StaminaDamage`、`StaminaDamageNegation`（Tag：`Sigil.Attribute.CombatSet.*`）。
- **`USigilPoiseSet`** —— `Poise`、`MaxPoise`、`PoiseRecover`（Tag：`Sigil.Attribute.PoiseSet.*`）。
- **`FSigilGameplayEffectContext`** —— 自定义效果上下文，把攻击定义行句柄做网络序列化（`SetAttackDefinitionHandle` / `GetAttackDefinitionHandle`），让受击方的 Combat Flow 能读到完整攻击行。它由 `USigilCombatAbilitySystemGlobals::AllocGameplayEffectContext` 分配——这也是下面 Globals 类配置为硬性要求的原因。

## 前置条件

- 启用 **sigil.gas**（以及引擎插件 Gameplay Abilities、Modular Gameplay、Targeting System、Motion Warping、Niagara，均由 `SigilCombat.uplugin` 声明）。
- **`AbilitySystemGlobalsClassName` 必须配成 sigil.combat 的 Globals 类**（见「配置」）——不配则自定义效果上下文不会被分配，攻击定义无法随效果 Spec 传输。
- 角色类**实现 `ISigilCombatInterface`**。
- 角色主 Mesh 组件打上**组件 Tag `Main`**（可改，见「配置」），`USigilCombatFunctionLibrary::GetMainCharacterMeshComponent` 靠它定位。
- 准备行类型为 **`FSigilAttackDefinition`**（近战）与 **`FSigilBulletDefinition`**（远程）的 DataTable。
- 为每个 `USigilCombatSystemComponent` 准备一个 **`USigilCombatFlow` 蓝图子类**并赋给 `CombatFlowClass`。

## 快速上手

1. **配置 Globals 类。** `DefaultGame.ini`：

   ```ini
   [/Script/GameplayAbilities.AbilitySystemGlobals]
   AbilitySystemGlobalsClassName=/Script/SigilCombat.SigilCombatAbilitySystemGlobals
   ```

2. **准备角色。** 实现 `ISigilCombatInterface`、给主 Mesh 打 `Main` 组件 Tag，按需挂 `USigilCombatSystemComponent`、`USigilCollisionSystemComponent`（近战）、`USigilTargetingSystemComponent`（锁定）、`USigilCombatTeamAgentComponent`（阵营）。角色还需要 sigil.gas 的 ASC，并授予 `USigilCombatSet`（可选 `USigilPoiseSet`）。

3. **配攻击数据。** 建行类型 `FSigilAttackDefinition` 的 DataTable，填攻击 Tag、SetByCaller 数值、目标效果类、Cue、击退参数。

4. **做 Combat Flow。** 蓝图子类化 `USigilCombatFlow`，往 `AttackResultProcessors` 里塞 `USigilAttackResultProcessor` 子类实现受击反应，再把 Flow 子类赋给战斗组件的 `CombatFlowClass`。

5. **接近战。** 在攻击蒙太奇上摆 `USigilANS_AttackTrace` 的蓝图子类，其 `AttackRequest`（近战）引用攻击行并指定要启用的 Trace Tag；Notify 逻辑自己实现——配合攻击技能里的 `USigilAbilityTask_CollisionTrace::HandleCollisionTraces` 开关对应的 `USigilCollisionTraceInstance`。命中后按攻击定义组效果 Spec 施加，并在受击方战斗组件上 `RegisterAttackResult`，让它的 Flow 做出反应。

6. **接远程。** 建 `FSigilBulletDefinition` 行，用 `USigilANS_BulletTrace` 蓝图子类（或直接调用）`USigilBulletSubsystem::SpawnBullets` 生成子弹，传入含拥有者、定义行、变换、子弹攻击请求的 `FSigilBulletSpawnParameters`。

7. **实现伤害结算**（项目侧）：写一个 GameplayEffect / ExecutionCalculation，读 `CombatSet` 属性与 SetByCaller 数值算出最终伤害，写入受击方的 `IncomingDamage`（sigil.gas）并结算 Health。

## 关键类型

| 类型 | 说明 |
| --- | --- |
| `USigilCombatSystemComponent` | 角色战斗中枢：持有复制的 Combat Flow、攻击结果 FastArray、可预测蒙太奇播放。 |
| `USigilCombatFlow` | 抽象、可复制的受击流水线对象；按角色形态子类化（**必须**）。 |
| `USigilAttackResultProcessor` | 内联、可蓝图化的受击反应步骤，逐条处理 `FSigilAttackResult`。 |
| `FSigilAttackResult` / `FSigilAttackResultContainer` | 攻击结果载荷及其可复制 FastArray 容器。 |
| `FSigilAttackDefinition` | 描述一次攻击的 DataTable 行（Tag、SetByCaller、效果、Cue、击退、打击感、扩展设置）。 |
| `USigilAttackRequest_Melee` / `USigilAttackRequest_Bullet` | 内嵌于通知状态的攻击请求对象，解析到攻击定义行。 |
| `USigilANS_AttackTrace` / `USigilANS_BulletTrace` / `USigilANS_MovementCancellation` | 动画通知状态；前两个只带数据，Notify 逻辑在蓝图子类里实现。 |
| `USigilCollisionSystemComponent` / `USigilCollisionTraceInstance` | 池化、基于插槽的近战扫描，带 `OnHit` 事件。 |
| `USigilAbilityTask_CollisionTrace` / `USigilAsyncAction_CollisionTrace` | 碰撞系统的技能任务 / 异步动作两种入口。 |
| `USigilBulletSubsystem` / `ASigilBulletInstance` / `FSigilBulletDefinition` | 池化子弹系统：多发弹道、穿透、子弹链。 |
| `USigilTargetingSystemComponent` | 锁定目标：复制的目标、潜在目标刷新、切换目标 API。 |
| `USigilTargetingFilterTask_*` / `USigilTargetingSelectionTask_*` | Targeting System 任务（归属/死亡/Tag 条件/Trace 实例过滤；线性/扩展选择）。 |
| `USigilCombatTeamAgentComponent` / `ISigilCombatTeamAgentInterface` | 复制的 `FGenericTeamId` 阵营归属。 |
| `ASigilWeaponActor` / `ISigilWeaponInterface` | 武器抽象与默认 Actor 实现。 |
| `USigilCombatSet` / `USigilPoiseSet` | 战斗/韧性属性集（`Sigil.Attribute.CombatSet.*`、`Sigil.Attribute.PoiseSet.*`）。 |
| `FSigilGameplayEffectContext` / `USigilCombatAbilitySystemGlobals` | 携带攻击定义行句柄的自定义效果上下文，及负责分配它的 Globals 类。 |
| `ISigilCombatInterface` | 由项目实现的角色接口（目标、武器、移动状态、死亡生命周期）。 |

## 配置

- **`USigilCombatSystemSettings`**（`UDeveloperSettings`，`Config=Game`，出现在项目设置里）：

  | 属性 | 默认值 | 说明 |
  | --- | --- | --- |
  | `CharacterMeshLookupTag` | `Main` | 查找角色主骨骼 Mesh 用的组件 Tag。 |
  | `bDisableAffiliationCheck` | `false` | 调试开关：允许跨阵营伤害/锁定。 |

- **`DefaultGame.ini`** —— `AbilitySystemGlobalsClassName=/Script/SigilCombat.SigilCombatAbilitySystemGlobals`（硬性要求；它是 sigil.gas Globals 的子类，属性组初始化照常可用）。
- **DataTable** —— 攻击行类型 `FSigilAttackDefinition`；子弹行类型 `FSigilBulletDefinition`。两者都有 `UserSettings`（`FInstancedStruct`，基结构 `SigilUserSetting`）供项目扩展，无需改插件。
- **组件级** —— 战斗：`CombatFlowClass` + `AttackResultProcessors`；碰撞：`TraceDefinitions` + `bAutoInitialize`；锁定：`TargetingPreset` + `bAutoUpdatePotentialTargets`；阵营：`CombatTeamId` + `bAssignTeamIdToController`。

## 网络

**会复制：**

- Combat Flow 实例与 `FSigilAttackResultContainer` FastArray——攻击结果到达所有客户端，Flow 的处理器经 `PostReplicatedAdd` 在各端执行。
- 可预测蒙太奇——发起端本地预测，服务器复制带触发时间的 `ReplicatedMontageInfo` 供其他端对齐。
- 锁定目标 `TargetedActor` 与阵营 `CombatTeamId`。
- `FSigilGameplayEffectContext` 随效果 Spec 网络序列化攻击定义行句柄。

**纯本地 / 不复制：**

- **碰撞扫描不复制**——Trace 实例是 Transient 的本地对象，在你的技能流程指定的那台机器上跑（通常是服务器，或由本机客户端扫完把目标数据上报服务器）。
- 锁定组件的 `PotentialTargets` 列表仅存在于服务器。

**已知空白（以当前源码为准）：**

- **子弹预测对账未实现。** 子弹支持本地预测（`bIsLocalPredicting`、`FSigilBulletSpawnParameters` 里的预测子弹 ID），服务器生成的子弹也能检测到对应的客户端预测子弹——但对账钩子 `ASigilBulletInstance::FoundLocalPredictedBullet` 的默认实现是空的。需要平滑客户端预测弹道的项目要自己实现这段交接。
- 伤害结算在项目侧（见前文），其网络行为取决于项目自己的 GameplayEffect 方案。

## 相关文档

- [sigil.gas](sigil-gas.zh-CN.md) —— 本插件依赖的技能系统地基。
- [sigil.input](sigil-input.zh-CN.md) —— 触发战斗技能的 Tag 化输入层。
