# SigilArsenal 决策日志

> 本文件只记录 `SigilArsenal` 插件内部的设计取舍，不记录消费项目（如 ProjectSpecter）的玩法、资产或产品范围。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-09-13。

### [2026-09-13] 新开 sigil.arsenal 插件承载"物品 → 技能 / 蒙太奇 / 动画层"的桥，不让老包互相依赖

- 阶段: 选型
- 面临的选择: Likeon 定下枪械模型：枪是 item，item 配 ability，装备并激活后 ability 可用，每把枪自己配"技能 → 蒙太奇"和动画层。核对发现 Sigil 只有两半：`SigilInventory` 的"物品 → 装备实例 → 生成武器 Actor"，`SigilCombat` 的"技能标签 → 蒙太奇"查表（`USigilAbilityActionSetSettings`）；把它们连起来的边不存在（Inventory 与 GAS 零耦合是文档承诺，Combat 也不认识 Inventory）。选项：① `SigilCombat` 依赖 `SigilInventory`；② 新开小插件依赖 gas + inventory + combat；③ `SigilInventory` 依赖 GAS。
- 定了什么: ②，插件名 `SigilArsenal`（sigil.arsenal），依赖 `SigilGas` / `SigilInventory` / `SigilCombat`。三个老包一行不动、`docs` 里"其余包全独立"的承诺保住；不用库存 / 不用枪的项目不必加载它。
- 否掉了什么 + 为什么: 否掉①——Combat 是"近战 Trace + 子弹 + 攻击定义"，逼所有 Combat 用户加载 Inventory 不合理；否掉③——推翻 Inventory 的零耦合设计。Likeon 未对此项明确表态（"你只管做"），按我给出的倾向执行，可否决。
- 复用层🔑: ② 引擎相关
- 来源: Likeon 2026-09-13 原话"枪械是 item, 这个 item 可以配置 ability… 枪械配置不同 ability 对应的蒙太奇… 动画的 layer 也是一样的原理"；`docs/getting-started.md` 依赖图；`SigilArsenal.uplugin`。

### [2026-09-13] 配置跟物品走：一个 `WeaponLoadout` 片段装技能集、动作集、动画层

- 阶段: 迭代
- 面临的选择: 把技能集 / 动作集 / 动画层配在**武器 Actor 类**上（每把枪一个 BP 类），还是配在**物品定义**的片段上（武器 Actor 只是壳）。
- 定了什么: 新片段 `USigilItemFragment_WeaponLoadout`：`AbilitySet`（`USigilAbilitySet`，软引用）、`GrantPolicy`（`WhileEquipped` 默认 / `WhileActive`）、`AbilityActionSet`（`USigilAbilityActionSetSettings`，软引用）、`AnimLayerClass`（软类）、`AnimLayerMeshLookupTag`。与既有 `Equippable` 片段并列，后者的 `InstanceType` 指向新装备实例类即可。
- 否掉了什么 + 为什么: 否掉配在 Actor 类上——数据跟物品走，同一把枪换皮不必复制配置；Actor 也可能根本不存在（纯数据武器）。软引用是为了跟 Inventory 既有片段（`InstanceType` 软类）一致、避免物品定义硬拉动画资产。
- 复用层🔑: ② 引擎相关
- 来源: `SigilItemFragment_WeaponLoadout.h`；`SigilItemFragment_Equippable.h`。

### [2026-09-13] 授予策略默认"装备即授予 + 激活门控"，SourceObject = 装备实例

- 阶段: 迭代
- 面临的选择: 装备进槽就授予、只有当前激活的那把能放（A2 的 `bRequireSourceObjectActive` 门控）；还是切到激活才授予、切走就收回。SourceObject 用装备实例、武器 Actor 还是物品实例。
- 定了什么: 两种都支持，默认 `WhileEquipped`（换枪零延迟，靠门控），`WhileActive` 给要 ASC 干净的项目。`USigilWeaponEquipmentInstance` 实现 `ISigilAbilitySourceInterface`（= `IsEquipmentActive`），授予时 SourceObject = 装备实例：技能从它能拿到 `GetSourceItem()`（弹药等物品态）、`GetWeaponActor()`（枪口 / Trace）和 `GetAbilityActionSet()`（蒙太奇表）三样，Actor 或物品单独做 SourceObject 都少一样。生成的武器 Actor 通过 `ISigilWeaponInterface::SetSourceObject` 反指装备实例，`USigilArsenalFunctionLibrary::GetWeaponEquipmentOfWeaponActor / OfAbility` 双向可达。
- 否掉了什么 + 为什么: 否掉只做一种策略——两者成本都只有几行，且审计 Q4 明确是待决项。
- 踩坑 / 反思: 授予发生在 `OnEquipmentBeginPlay`，要求 ASC 已 `InitAbilityActorInfo`；否则记 Warning 并跳过（不在装备侧排队等待，那是项目初始化顺序的事）。`GrantAbilities` / `RevokeAbilities` 都是幂等的，`OnEquipmentEntryRemoved` 先发 `OnActiveStateChanged(false)` 再 `OnEquipmentEndPlay` 的双路径不会重复释放。
- 复用层🔑: ② 引擎相关
- 来源: 审计 `gasshooter-sigil-borrow-audit.md` §2 A1/A2、§8 Q4；`SigilWeaponEquipmentInstance.cpp`；Automation `SigilArsenal.WeaponLoadout.GrantsGatesAndSwitchesWeapons`、`…WhileActivePolicyGrantsOnlyWhenActive`。

### [2026-09-13] 蒙太奇 / 动画层：动作集从"当前激活武器"查，动画层只链主网格

- 阶段: 迭代
- 面临的选择: 项目角色的 `ISigilCombatInterface::QueryAbilityActions` 该从哪拿表；动画层链主网格还是连第一人称网格一起。
- 定了什么: `USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions(Pawn, AbilityTags, …)` 找装备系统里激活的 `USigilWeaponEquipmentInstance`，用其动作集 `SelectBestAbilityActions`——项目角色的 `QueryAbilityActions` 一行转发即可，手枪 / 步枪同一个"射击"技能查到各自的蒙太奇。动画层在 `OnActiveStateChanged(true)` 时 `LinkAnimClassLayers` 到 `USigilCombatFunctionLibrary::GetMainCharacterMeshComponent` 找到的主网格（可用 `AnimLayerMeshLookupTag` 覆盖），失活 / 卸下时 `Unlink`；每台机器各自链接（`OnActiveStateChanged` 服务器与客户端都会回调）。
- 否掉了什么 + 为什么: 否掉此批次链第一人称网格——多网格路线（PR #4 B1①）还没在项目里落地，等它定了再扩成"按网格列表"。
- 踩坑 / 反思: `USigilEquipmentSystemComponent::GetActiveEquipments` 把**空** `FGameplayTagQuery` 当"什么都不匹配"，库里用 `NoTagsMatch()` 的空表达式构造出"匹配所有槽位"的查询（`MakeAnySlotQuery`），对外把空查询解释为"任意槽位"。Automation 无法造出带 AnimInstance 的骨骼网格，动画层真实链接【未验证】，测试只覆盖授予 / 门控 / 切换 / 查表 / 反指。
- 复用层🔑: ② 引擎相关
- 来源: `SigilArsenalFunctionLibrary.cpp`；`SigilEquipmentSystemComponent.cpp:134-154`；`SigilCombatFunctionLibrary.cpp:87-120`。

### [2026-09-13] A3：弹匣使用物品整数属性，Cost 在武器所属 authority 扣除

- 阶段: 迭代
- 面临的选择: 浮点缩放成本还是明确整数；Cost 放 SigilGas 还是已依赖 Inventory 的 SigilArsenal；读取当前激活武器还是读取被检查的技能来源。
- 定了什么: `USigilAbilityCost_ItemIntegerAttribute` 放 SigilArsenal；`Quantity` 为正整数，标签可配置并提供 Magazine / MagazineCapacity / Fail.Ammo 原生默认值。弹匣由 DynamicAttributes 初始化，容量由物品定义的 StaticIntegerAttributes 表达。CheckCost 依据传入 Handle/ActorInfo 的 `Ability->GetSourceObject` 解析装备和源物品；ApplyCost 仅 owning pawn 的 authority 扣除，执行时再次检查属性与余额。
- 否掉了什么 + 为什么: 不引入物品 AttributeSet、浮点舍入、复制抑制、备弹、换弹或 Tag→Attribute 映射；这些超出本批弹匣范围。拒绝无效标签和非正 Quantity，避免错误配置变成免费射击或增加弹药。没有调用依赖 CurrentSpec 的便捷库，是为覆盖激活前检查及同能力多 Spec 的来源定位；数据仍走同一装备→源物品链。
- 复用层: 引擎相关，SigilArsenal 可复用武器库存桥。
- 来源: Likeon 2026-09-13 本次指令；本机交接 `MD/handoff/2026-09-13-firearms-batch2-handoff.md` §3.1；审计 `gasshooter-sigil-borrow-audit.md` §2 A3（Mine，仅借结构，未复制第三方实现）；本机 UE5.8 `GameplayAbility.cpp` 的 `GetSourceObject`；`SigilItemInstance.cpp` 的整数属性方法。联网与真实玩法【未验证】。
- 本批验证（2026-09-13，Codex 实跑）: 空实现的 `SigilArsenal.Ammo` 4 项均失败（抓到未扣弹/未拒绝）；实现后 HostEditor Win64 Development UE5.8 编译通过，`Automation RunTests SigilArsenal` 为 5 过 / 1 带警告过 / 0 失败，4 个新增 Ammo 用例均通过。带警告通过来自引擎未配置 GameplayCueNotifyPaths 提示。证据：`Host/Saved/Batch2/a3-red/index.json`、`a3-build.log`、`a3/index.json`。非 authority 测试只切换本地 Pawn Role，真实联网【未验证】。
