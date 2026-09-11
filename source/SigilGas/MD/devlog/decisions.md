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
