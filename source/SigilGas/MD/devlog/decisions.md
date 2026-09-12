# SigilGas 决策日志

> 本文件只记录 `SigilGas` 插件内部的设计取舍，不记录消费项目（如 ProjectSpecter）的玩法、资产或产品范围。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-09-11。
>
> 本轮（2026-09-11）条目来源：外部审计 `D:\P4\Code_UE5.6\MD\analysis\gasshooter-sigil-borrow-audit.md`（EXT-20260911-GASSHOOTER-BORROW-AUDIT-001，R2），只实施其中 Sigil 侧、不依赖枪械产品门的项。抄自 GASShooter / GASDocumentation 的代码保留 `Copyright 2020 Dan Kestranek`（MIT）署名。

### [2026-09-11] A2：技能"来源对象激活"门禁走接口，不 Cast 具体角色类

- 阶段: 迭代
- 面临的选择: 照抄 GASShooter `bSourceObjectMustEqualCurrentWeaponToActivate`（`Cast<AGSHeroCharacter>` 后比较 `GetCurrentWeapon()`），或让 SourceObject 自己汇报激活态。
- 定了什么: SigilGas 新增 `ISigilAbilitySourceInterface::IsAbilitySourceActive()`（Blueprintable）；`USigilGameplayAbility` 新增 `bRequireSourceObjectActive`（默认 false，语义不变），开启时 `CanActivateAbility` 在 `Super` 之前查 `GetSourceObject()`，SourceObject 缺失 / 未实现接口 / 汇报未激活 → 以新失败标签 `Sigil.Ability.ActivateFail.SourceObjectInactive` 拒绝。`ASigilWeaponActor`（SigilCombat）实现该接口并桥接到 `IsWeaponActive`。
- 否掉了什么 + 为什么: 否掉在 SigilGas 里引用 `ISigilWeaponInterface` / `USigilEquipmentInstance`——依赖图只允许 combat→gas，SigilInventory 与 GAS 零耦合是刻意设计（`docs/sigil-inventory.zh-CN.md`），装备实例的桥接留给消费项目在 `OnEquipmentBeginPlay` 里做。否掉"未实现接口时放行"——"require" 语义应严格，误配时宁可拒绝并给出失败标签。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 A2；GASShooter `GSGameplayAbility.h:64-66`、`.cpp:180-191`；Automation `SigilGas.Ability.RequireSourceObjectActive`。

### [2026-09-11] A7：按类 + SourceObject 查技能句柄放函数库，不放 ASC

- 阶段: 迭代
- 面临的选择: 照 GASShooter 把 `FindAbilitySpecHandleForClass` 加在 ASC 上，或放进 `USigilAbilitySystemFunctionLibrary`。
- 定了什么: 加在函数库：`FindAbilitySpecHandleForClass(ASC, AbilityClass, OptionalSourceObject)`，精确类匹配 + 可选 SourceObject 过滤，用公开的 `GetActivatableAbilities()` 遍历（5.8 已公开，无需 `ABILITYLIST_SCOPE_LOCK`），无匹配返回无效句柄。
- 否掉了什么 + 为什么: 否掉挂在 ASC 上——Sigil 现有"按 Tag / Query 找技能"全在函数库（`FindAbilityWithTags` 等），保持同一入口；否掉子类匹配（`IsChildOf`）——GS 原语义是精确类，装备授予/回收场景需要精确定位。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 A7；GASShooter `GSAbilitySystemComponent.cpp:140-156`；Automation `SigilGas.Library.FindAbilitySpecHandleForClass`。

### [2026-09-11] C1：冷却 / 堆叠变化异步节点基于 UAbilityAsync，修掉原版两处假设

- 阶段: 迭代
- 面临的选择: 逐行照抄 GASDocumentation 的 `UAsyncTaskCooldownChanged` / `UAsyncTaskEffectStackChanged`（基于 `UBlueprintAsyncActionBase` + 手写 `EndTask`），或改基于引擎 `UAbilityAsync`（Sigil 现有 `AttributeChanged` / `TagAddedRemoved` 同款）。
- 定了什么: 新增 `USigilAsyncTask_CooldownChanged`（`ListenForCooldownChange(ASC, CooldownTags, bUseServerCooldown)`，`OnCooldownBegin` / `OnCooldownEnd`）与 `USigilAsyncTask_EffectStackChanged`（`ListenForGameplayEffectStackChange(ASC, EffectTag)`），都继承 `UAbilityAsync`，用 `EndAction` 结束、`ShouldBroadcastDelegates` 守卫。文件头保留 `Copyright 2020 Dan Kestranek`（MIT）。两处改动：① 冷却剩余时间按命中的冷却标签本身查询，不再假设"冷却标签永远是 GrantedTags[0]"；② 结束时 `OnGameplayEffectStackChangeDelegate` 返回空指针要判空（效果已移除时原版会解引用空指针），移除广播的旧堆叠数用真实 `GetStackCount()` 而非常量 1。
- 否掉了什么 + 为什么: 否掉 `UBlueprintAsyncActionBase` 直系——与 Sigil 现有异步节点风格不一致，且 `UAbilityAsync` 自带 ASC 解析与取消守卫。
- 踩坑 / 反思: Minimal 复制模式的 ASC 上客户端收不到冷却 GE，`OnCooldownBegin` 不会触发（审计已提示）；写进头文件注释，未做绕过。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 C1；GASDocumentation `AsyncTaskCooldownChanged.h/.cpp`、`AsyncTaskEffectStackChanged.h/.cpp`，README `1555-1564,3113-3126`；Automation `SigilGas.AsyncTask.CooldownChanged`、`SigilGas.AsyncTask.EffectStackChanged`。
### [2026-09-11] C2：共享冷却 GE 走 SetByCaller，未配置时完全走引擎原路径

- 阶段: 迭代
- 面临的选择: GASDocumentation 4.5.15 的两种复用冷却 GE 方案——① SetByCaller 时长，② MMC 读技能上的时长；以及是否无条件走新路径。
- 定了什么: `USigilGameplayAbility` 新增 `CooldownTags`（FGameplayTagContainer）、`CooldownDuration`（FScalableFloat）、`CooldownDurationSetByCallerTag`（默认新原生标签 `Sigil.SetByCaller.CooldownDuration`）；覆写 `GetCooldownTags()` 返回"冷却 GE 授予标签 ∪ CooldownTags"，覆写 `ApplyCooldown()`：**仅当** `CooldownTags` 非空或 `CooldownDuration > 0` 时才自建 Spec 注入 `DynamicGrantedTags` / SetByCaller 量值；两者都未配置则原样调用 `Super::ApplyCooldown`，既有技能行为逐字节不变。
- 否掉了什么 + 为什么: 否掉 MMC 方案——多一个资产类且依赖 `GetAbilityInstance_NotReplicated`，SetByCaller 更直接；否掉"无条件走新路径"——会给未配置 SetByCaller 的旧冷却 GE 塞无用量值，且违反"不改公开 API 语义"的约束。
- 踩坑 / 反思: 审计处方：枪械射速**不要**用 Cooldown GE（不可预测、高延迟下射速变慢，GASDocumentation README 1572），本机制只给技能冷却用。`TempCooldownTags` 是 `mutable` 成员而非 `UPROPERTY(Transient)`，因为返回指针的容器不需要 GC 跟踪。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 C2；GASDocumentation README `1398-1516`（本轮已读实现段）；Automation `SigilGas.Ability.SharedCooldown`。

### [2026-09-11] C3：GameplayCueManager 子类默认按需加载 Cue，用 Config 开关而非硬编码

- 阶段: 迭代
- 面临的选择: 照 GASShooter 硬编码 `ShouldAsyncLoadRuntimeObjectLibraries() { return false; }`，或做成可配置。
- 定了什么: 新增 `USigilGameplayCueManager`（`UCLASS(Config=Game)`），`UPROPERTY(Config) bAsyncLoadRuntimeObjectLibraries` 默认 false，覆写返回该值。消费项目在 `DefaultGame.ini` 的 `[/Script/GameplayAbilities.AbilitySystemGlobals]` 设 `GlobalGameplayCueManagerClass=/Script/SigilGas.SigilGameplayCueManager` 启用；要恢复引擎"启动时全量异步加载"只需 ini 里把该布尔设 True。Host 工程未配置该 ini（Host 没有 Config 目录、也没有 Cue 资产），只验证类行为。
- 否掉了什么 + 为什么: 否掉硬编码 false——大项目地图内 Cue 少时按需加载省内存，但小项目 / 首次触发卡顿敏感的场景可能想保留预加载，一行 ini 比改代码便宜。否掉抄 `GSEngineSubsystem::InitGlobalData`——UE5.6 起引擎 `GetAbilitySystemGlobals()` 首次调用自动 `InitGlobalData()`（审计 §4.2）。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 C3；GASShooter `GSGameplayCueManager.h:22-26`；GASDocumentation README `2429-2454`；Automation `SigilGas.CueManager.LoadsRuntimeObjectLibrariesOnDemand`。

### [2026-09-11] C4：被动技能在 GA 的 OnAvatarSet 重试激活，保留 ASC 侧既有重试

- 阶段: 迭代
- 面临的选择: 审计称"`OnAvatarSet` 只转发 K2，Avatar 晚到时被动静默失效"；核实后发现该前提对 Sigil ASC 不成立——`USigilAbilitySystemComponent::InitAbilityActorInfo` 在 AvatarChanged 时已遍历技能调 `TryActivateAbilityOnSpawn`。选择：什么都不做 / 只改 GA / 改 GA 并删 ASC 循环。
- 定了什么: 在 `USigilGameplayAbility::OnAvatarSet` 末尾调用 `TryActivateAbilityOnSpawn(ActorInfo, Spec)`（GASShooter 做法），ASC 侧循环保留。引擎 5.8 `InitAbilityActorInfo` 在 AvatarChanged 时先对每个 Spec 的实例 / CDO 调 `OnAvatarSet`（`AbilitySystemComponent_Abilities.cpp:182-200`，本轮已核实），随后才轮到 Sigil ASC 的循环；`TryActivateAbilityOnSpawn` 以 `!Spec.IsActive()` 守卫，两次尝试不会重复激活（测试断言激活计数为 1）。
- 否掉了什么 + 为什么: 否掉"什么都不做"——GA 自带重试让被动技能不依赖 ASC 是否为 Sigil 子类，行为更自洽；否掉删 ASC 循环——它同时服务实现了 `ISigilGameplayAbilityInterface` 的非 Sigil GA，删掉属改公开语义。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 C4；GASShooter `GSGameplayAbility.cpp:36-45`；引擎 `AbilitySystemComponent_Abilities.cpp:165-200`；Automation `SigilGas.Ability.PassiveActivatesWhenAvatarArrives`。
### [2026-09-11] §3.3：TargetActor 散布随时间衰减按世界时间惰性结算，默认关闭

- 阶段: 迭代
- 面临的选择: 审计指出 GASShooter 与 Sigil 共有的坑——`CurrentTargetingSpread` 只累加、靠外部 `ResetSpread()` 归零。选择：在 Tick 里每帧衰减 / 按世界时间惰性结算 / 交给 GA 自己管。
- 定了什么: `ASigilAbilityTargetActor_Trace` 新增 `TargetingSpreadDecayRate`（度/秒，默认 0 = 关）与 `TargetingSpreadDecayDelay`（秒）。`AddTargetingSpread()`（原 `AimWithPlayerController` 里的累加行）先结算衰减再累加并记时间戳；`UpdateTargetingSpreadDecay()` / `GetCurrentTargetingSpread()` 用 `LastIncreaseTime + Delay` 与 `LastDecayTime` 的较大者作起点按世界时间结算，`GetCurrentSpread()` 读衰减后的值；Tick 里也结算一次。`ResetSpread()` 一并清零新字段与时间戳（沿用它"清零一切"的既有语义）。
- 否掉了什么 + 为什么: 否掉"只在 Tick 衰减"——即时确认模式 Tick 从不运行（Start→Confirm→Stop 一帧内完成），衰减会永远不发生；否掉改 `LineTrace::Configure` 签名塞新参数——两个 `BlueprintReadWrite` 字段直接设即可，不动既有蓝图节点引脚。随机种子仍是 `FMath::Rand()`（审计另一条"不可复现"未在本轮处理）。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §3.3；`SigilAbilityTargetActor_Trace.cpp` 原 L252；Automation `SigilGas.TargetActor.SpreadDecaysOverTime`。
### [2026-09-11] §1：AimWithPlayerController 意图核对——保留 Sigil 门控语义，加 GASShooter 兼容开关

- 阶段: 迭代
- 面临的选择: 审计 §1 指出行为差异：GASShooter `GSGATA_Trace.cpp:220-223` 只要有 `MasterPC` 就用玩家视线瞄准（再从 `TraceStart` 朝该点出射），Sigil 只在 `bTraceFromPlayerViewPoint` 为真时才用玩家视线，否则沿 `StartLocation` 的旋转瞄准。是有意改动还是漏抄？
- 核对结果: `git log -S` 显示该门控随开源基线提交（`6b90416` "open-source baseline: four UE5.6 Generic* plugins"）一并进入，之后 `1f5a7ce` 仅改名，没有单独提交或注释说明动机 → **意图【未知】**。本轮不擅自改回 GS 语义（会让所有把 `bTraceFromPlayerViewPoint` 留 false 的第三人称 / AI 用法突然改成"从相机瞄准"，属于改公开语义）。
- 定了什么: 把视点解析抽成 `GetAimViewPoint(TraceStart, OutViewStart, OutViewRot)`（虚函数，可测），新增 `bAlwaysAimWithPlayerController`（默认 false）——为 true 时恢复 GASShooter 的"相机瞄准、枪口出射"；`bTraceFromPlayerViewPoint` 语义不变。FP 枪械两者任一为 true 即可；把 GS 蓝图参数照搬到 Sigil 时需显式打开其一，否则会"从枪口而非相机瞄准"（审计原话）。
- 否掉了什么 + 为什么: 否掉直接改回 GS 语义（理由见上）；否掉不做任何事——差异不显式暴露，迁移 GS 配置的人会踩坑。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §1；GASShooter `GSGATA_Trace.cpp:210-235`；`git log -S"PrimaryPC && bTraceFromPlayerViewPoint"`；Automation `SigilGas.TargetActor.AimViewPointRespectsFlags`。
### [2026-09-11] B1①：多网格蒙太奇走"主网格引擎路径 + 次要网格本地播放"的单机简化版，不移植复制版

- 阶段: 选型
- 面临的选择: 审计 B1 两档——①单机简化版（只抄 API 形状：主网格走引擎 `PlayMontage`，其他网格仅本地 `Montage_Play`、不复制）；②完整复制版（GASShooter 约 690 行逐 mesh 复制 / 预测 / OnRep，5.8 下 `RepAnimMontage` 字段全变、`PlayMontageInternal` 私有，等于重写）。
- 定了什么: 做①。`USigilAbilitySystemComponent` 新增 `PlayMontageForMesh / CurrentMontageStopForMesh / StopAllCurrentMontages / CurrentMontageJumpToSectionForMesh / CurrentMontageSetNextSectionNameForMesh / CurrentMontageSetPlayRateForMesh / GetCurrentMontageForMesh / GetAnimatingAbilityForMesh / IsAnimatingAbilityForAnyMesh / ClearAnimatingAbilityForMesh / ClearAnimatingAbilityForAllMeshes`，以 `IsAvatarMainMesh`（`ActorInfo->SkeletalMeshComponent`）分流：主网格原样调用引擎函数，其他网格在 `LocalMeshMontages`（`FSigilLocalMeshMontage`，Transient、不复制）记账，且只在 `ActorInfo->IsLocallyControlled()` 时播放；网格须直接或经拥有链属于化身（武器 / 装备 Actor 上的网格也算）。`USigilGameplayAbility` 新增 `GetCurrentMontageForMesh / SetCurrentMontageForMesh / MontageJumpToSectionForMesh / MontageSetNextSectionNameForMesh / MontageStopForMesh / MontageStopForAllMeshes`，主网格映射到引擎 `CurrentMontage`。`USigilAbilityTask_PlayMontageAndWaitForEvent` 加可选 `Mesh`（Params 字段 + 新静态 `PlayMontageForMeshAndWaitForEvent`），`Mesh` 为空时代码路径与改动前逐行一致。
- 否掉了什么 + 为什么: 否掉②——产品层单机，复制版收益为零且 5.8 下是重写（审计 §5.6-1）；否掉给次要网格应用 `AnimRootMotionTranslationScale`（GS 原样应用）——根运动只由主网格驱动，改 Character 全局缩放会误伤世界身体；否掉 GS 在 `NotifyAbilityEnded` 里连主网格一起清 AnimatingAbility——引擎主网格记账由蒙太奇混出回调清理，保持不变，只清次要网格。
- 踩坑 / 反思: 审计 §3.2 的两个 GS 源码 bug已处理：`SetCurrentMontageForMesh` 按值拷贝导致同网格第二次 Set 无效 → 改用指针查找（测试 `SigilGas.Montage.AbilityTracksMontagePerMesh` 覆盖）；Task 混用单网格 `GetCurrentMontage / ClearAnimatingAbility` → 全部按 `Mesh` 分流到 `GetAbilityCurrentMontage / ClearAnimatingAbilityForMesh`。Automation 无法造出带 AnimInstance 的骨骼网格，因此**真实蒙太奇播放未被自动化覆盖【未验证】**，测试只覆盖记账、路由与守卫（主网格识别、无 AnimInstance 返回 -1 且不记账、非化身网格拒绝、Task 无 AnimInstance 时广播 OnCancelled）。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 B1、§3.2、§5.6-1；GASShooter `GSAbilitySystemComponent.h/.cpp:270-961`、`GSGameplayAbility.h:149-198`、`GSAT_PlayMontageForMeshAndWaitForEvent`；引擎 `AbilitySystemComponent_Abilities.cpp:3035-3090,3504-3540`；Automation `SigilGas.Montage.*`。
### [2026-09-12] C3 修订（PR #4 评审 P1-A）：启用说明核实——ini 段名原本就对，补 Host 实配与接线断言

- 阶段: 迭代
- 面临的选择: 评审称 5.8 只从 `UGameplayAbilitiesDeveloperSettings` 读 `GlobalGameplayCueManagerClass`、`UAbilitySystemGlobals` 上的同名属性已 `UE_DEPRECATED(5.5)`，因此原注释教的 `[/Script/GameplayAbilities.AbilitySystemGlobals]` 段"照配 = 静默 no-op"，建议改成 `[/Script/GameplayAbilities.GameplayAbilitiesDeveloperSettings]`。
- 核对结果: 前半句属实（`AbilitySystemGlobals.cpp:628-630` 只读 DeveloperSettings），**结论不成立**：`UGameplayAbilitiesDeveloperSettings::OverrideConfigSection`（头文件 L125-132）把自己的 config 段强制映射到 `/Script/GameplayAbilities.AbilitySystemGlobals` 以兼容旧项目，所以原注释的段名正是唯一生效的写法。本机实测：Host 配 `[…GameplayAbilitiesDeveloperSettings]` 段时新断言失败（读回仍是引擎默认类），改回 `[…AbilitySystemGlobals]` 段后通过。
- 定了什么: ini 段名保持 `[/Script/GameplayAbilities.AbilitySystemGlobals]`，头文件注释补充"5.5+ 由 DeveloperSettings 读取但映射到该旧段、DeveloperSettings 段不生效"的说明；Host 新增被跟踪的 `Host/Config/DefaultGame.ini` 只配这一行；测试追加断言：`GetDefault<UGameplayAbilitiesDeveloperSettings>()->GlobalGameplayCueManagerClass` 等于 `/Script/SigilGas.SigilGameplayCueManager`，且 `UAbilitySystemGlobals::Get().GetGameplayCueManager()` 返回的实例 `IsA<USigilGameplayCueManager>` 并按需加载。
- 否掉了什么 + 为什么: 否掉照评审改段名——实测不生效；否掉只改注释不配 Host——评审说得对，原测试只 `NewObject` 读布尔，暴露不了接线问题，断言必须打在引擎真实创建的全局管理器上。
- 踩坑 / 反思: 上一条目写的"Host 没有 Config 目录"已不符现状——编辑器每次跑 Automation 会生成 `Host/Config/DefaultEngine.ini`（含 AndroidFileServer SecurityToken）与 `DefaultInput.ini`，这两个是垃圾、不提交；只有 `DefaultGame.ini` 是有意跟踪的。外部评审的"已复核"也要再复核一遍——这次是评审看漏了 `OverrideConfigSection`。
- 复用层🔑: ② 引擎相关
- 来源: `D:\P4\Code_UE5.6\MD\analysis\sigil-pr4-review.md` §2 P1-A；引擎 `GameplayAbilitiesDeveloperSettings.h:29,51-53,125-132`、`AbilitySystemGlobals.cpp:615-640`；Automation `SigilGas.CueManager.LoadsRuntimeObjectLibrariesOnDemand`（新增接线断言，两种段名各跑一次）。
### [2026-09-12] B1① 修订（PR #4 评审 P1-B）：非本地控制的次要网格"跳过"不等于"技能取消"

- 阶段: 迭代
- 面临的选择: 原实现次要网格在非本地控制时 `PlayMontageForMesh` 返回 -1，Task 把 `Duration <= 0` 一律当播放失败广播 `OnCancelled`——listen server 上为远端 Pawn 跑该技能时，纯表现网格没播会把技能砍掉。选项：① 返回蒙太奇长度假装播了；② 返回 0 且 Task 不广播取消、立即完成；③ 返回 0，Task 按蒙太奇缩放时长起定时器，到时广播 `OnBlendOut`+`OnCompleted`。
- 定了什么: ③。ASC 新增虚函数 `ShouldPlaySecondaryMeshMontages()`（= `ActorInfo->IsLocallyControlled()`），`PlayMontageForMesh` 对次要网格在其为 false 时返回 **0**（有意跳过），-1 只表示真失败。Task 在该分支仍绑定 GameplayEvent 委托，起 `GetPlayLength() - StartTimeSeconds) / Rate` 的定时器，到时先后广播 `OnBlendOut`、`OnCompleted` 并 `EndTask`；`OnDestroy` 清定时器；长度为 0 则立即完成。主网格路径不变。
- 否掉了什么 + 为什么: 否掉①——调用方拿到"长度"会去绑蒙太奇委托，永远收不到结束回调；否掉②——立即完成会让依赖 `OnCompleted` 收尾的技能在远端提前结束，时序仍然不一致。
- 踩坑 / 反思: 单机 `IsLocallyControlled()` 恒真，这个分支之前根本走不到，"现在不炸不等于语义对"（评审原话）。测试用 `USigilGasTestAbilitySystemComponent::bScriptedLocallyControlled` 强制走远端分支，`FTimerManager::Tick` 推进时间断言完成时机。
- 复用层🔑: ② 引擎相关
- 来源: `D:\P4\Code_UE5.6\MD\analysis\sigil-pr4-review.md` §2 P1-B；Automation `SigilGas.Montage.RemoteSecondaryMeshKeepsAbilityTiming`。
