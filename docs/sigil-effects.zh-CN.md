[English](sigil-effects.md) | [简体中文](sigil-effects.zh-CN.md)

# sigil.effects

**插件：** `SigilEffects` · **模块：** `SigilEffects`（Runtime） · **依赖：** Niagara（引擎插件）、PhysicsCore、Gameplay Tags

sigil.effects 是一套情景驱动的反馈系统：动画通知里不再硬引用某个具体音效或粒子，而是由玩法发出一个"效果标签 + 情景标签"，再由库资产解析出该播的声音、Niagara 系统或 Cascade 粒子。情景可以来自调用方、来自 Actor 的默认配置、来自游戏标签提供者，也可以来自射线检测到的物理表面——同一个"脚步"通知在草地、水泥地、水面上就能播出不同的效果。

## 概述（Overview）

### 效果解析：标签 + 标签查询

`USigilContextEffectsLibrary` 数据资产持有一个 `FSigilContextEffects` 条目数组，每条包含：

- `EffectTag` —— 与请求的效果标签做**精确匹配**。
- `SourceTagQuery` —— 对聚合后的源情景求值的 `FGameplayTagQuery`。**`SourceTagQuery` 为空的条目在加载时会被直接丢弃**，也就是说配不出"无条件总是播放"的条目——每条至少要有一个源情景条件。
- `TargetTagQuery` —— 可选；为空则忽略目标情景。
- `Effects` —— 软对象路径，限定为 `USoundBase`、`UNiagaraSystem`、`UParticleSystem` 资产。

旧版按条目的 `Context` 标签容器已废弃；保存时会自动转换成等价的"含全部标签"`SourceTagQuery`。

`GetEffects(Effect, SourceContext, TargetContext, ...)` 返回所有匹配条目的资产。匹配要求效果标签有效**且源情景非空**。

### 加载模型（如实说明）

`USigilContextEffectsLibrary::LoadEffects` 对所有软引用做**同步**解析（`TryLoad`），源码里留有明确的 `TODO Add Async Loading for Libraries`。库在 Actor 注册时加载（见下），通常发生在 `BeginPlay`——大库注册时可能卡顿，注册较晚时首次播放也可能出现停顿。

### 注册粒度 = Actor

`USigilContextEffectsSubsystem`（`UWorldSubsystem`）维护一张 **Actor → 已加载库集合** 的映射（`LoadAndAddContextEffectsLibraries(OwningActor, Libraries)` / `UnloadAndRemoveContextEffectsLibraries(OwningActor)`）。生成效果的入口：

- `SpawnContextEffectsExt(SpawningActor, Input, Output)` —— 在**该 Actor 注册过的所有库**里解析；
- `SpawnContextEffects(WorldContextObject, EffectsLibrary, Input, Output)` —— 只查指定的单个库；
- `GetContextFromSurfaceType(PhysicalSurface, OutContext)` —— 查设置里的表面→标签映射。

输入/输出载荷为 `FSigilSpawnContextEffectsInput`（效果标签、附加模式 + 骨骼/组件/偏移或世界位置旋转、源/目标情景、`SourceContextType` Merge/Override、VFX 缩放、音量音高、可选 `HitResult`）与 `FSigilSpawnContextEffectsOutput`（生成出的音频/Niagara/粒子组件）。

### 组件：情景三来源聚合

`USigilContextEffectComponent` 实现 `ISigilContextEffectsInterface`，是 Actor 上的标准接收端。`BeginPlay` 时它把 `DefaultContextEffectsLibraries` 以 Owner 名义注册进子系统，并用 `DefaultEffectContexts` 初始化 `CurrentContexts`。调用 `PlayContextEffectsWithInput` 时（`SourceContextType == Merge` 的情况下），源情景从**三个来源**聚合：

1. 调用方传入的 `Input.SourceContext`；
2. 组件的 `CurrentContexts`（默认情景，可用 `UpdateEffectContexts` 更新）；
3. 可选的 `GameplayTagsProvider`——任何实现 `IGameplayTagAssetInterface` 的对象提供的标签；开着 `bAutoSetupTagsProvider` 时，Owner 若实现该接口会被自动采用。

若勾选 `bConvertPhysicalSurfaceToContext` 且输入带 `HitResult`，命中物理材质的表面类型会经 `USigilContextEffectsSettings::SurfaceTypeToContextMap` 转换后注入源情景；查不到映射或没有物理材质时使用 `FallbackPhysicalSurface`。

### AnimNotify 触发链

`USigilAnimNotify_ContextEffects`（显示名 **Play Context Effects**）是主要的玩法触发器。`Notify` 时它：计算生成变换（附加到插槽，或在不附加时用可选的实例化 `USigilContextEffectsSpawnParametersProvider`）；可选地做一次射线检测（`bPerformTrace` + `FSigilContextEffectAnimNotifyTraceSettings`，已开启 `bReturnPhysicalMaterial`）供表面转换使用；然后找到网格的 Owner **及其所有实现了 `ISigilContextEffectsInterface` 的组件**，逐个调用 `PlayContextEffectsWithInput`。Actor 上挂着 `USigilContextEffectComponent` 时，整条链就通了：通知 → 组件 → 子系统 → 库 → 生成效果。

编辑器里有预览通路：`USigilContextEffectsSettings::bPreviewInEditor` 配合 `USigilContextEffectsPreviewSetting` 资产，可以在动画编辑器里用指定的预览表面类型试播效果。

### 游戏标签（如实说明）

插件原生只声明了一个标签：`GES`（注释为 "Generic Effects System"）。组件的 `FallbackPhysicalSurface` 属性把选择器过滤到 **`Sigil.Effects.SurfaceType`** 类目下，但插件**并没有**声明该类目下的任何标签——`Sigil.Effects.SurfaceType.*` 标签树（以及全部效果/情景标签）都要项目自己创建。

## 前置条件（Prerequisites）

- [ ] **启用 Niagara**（`.uplugin` 已声明依赖）。
- [ ] **项目自己的标签词表**：效果标签（如脚步/受击事件）与情景标签；用表面转换的话还包括 `Sigil.Effects.SurfaceType.*` 树。插件一个都没帮你声明。
- [ ] **场景配好物理材质**（且碰撞设置允许返回物理材质），并在设置里填好表面→标签映射，才能得到按表面变化的效果。
- [ ] **至少一个 `USigilContextEffectsLibrary` 资产**——记住每条条目的 `SourceTagQuery` 不能为空。
- [ ] **每个要播效果的 Actor 上有接收端**——`USigilContextEffectComponent`（或你自己的 `ISigilContextEffectsInterface` 实现）。

## 快速上手（Quick Start）

1. **建标签。** 定义效果标签（如每种脚步/受击事件一个）和情景标签（如 `Sigil.Effects.SurfaceType.*` 下的表面类型）。
2. **映射表面。** 在**项目设置**（`USigilContextEffectsSettings` 节，config 存 `Game`）里把 `EPhysicalSurface` 值映射到表面标签（`SurfaceTypeToContextMap`）。
3. **配库。** 新建 `USigilContextEffectsLibrary`；每个（效果 × 情景）组合加一条 `FSigilContextEffects`：填 `EffectTag`、`SourceTagQuery`（比如"含草地表面标签"），并在 `Effects` 里放音效/Niagara 资产。
4. **挂组件。** 给角色加 `USigilContextEffectComponent`：`DefaultContextEffectsLibraries` 指向库，`DefaultEffectContexts` 填常驻情景标签，配好 `FallbackPhysicalSurface`。
5. **摆通知。** 在脚步/受击动画里加 **Play Context Effects** 通知；设置 `Effect`、附加方式（`bAttached` + `SocketName`）或偏移，需要表面检测时开启 `bPerformTrace` 并给 `EndTraceLocationOffset` 一个向下的偏移。
6. **进游戏验证。** 通知调到组件，情景聚合、表面标签注入，匹配的声音/粒子生成。非动画触发的场景，直接调组件的 `PlayContextEffectsWithInput`（或子系统的生成函数）即可。

## 关键类型（Key Types）

| 类型 | 说明 |
| --- | --- |
| `USigilContextEffectsLibrary` | 条目数组数据资产；`LoadEffects` 同步加载；按条目的标签查询选取资产。 |
| `FSigilContextEffects` | 单条定义：`EffectTag`（精确匹配）、`SourceTagQuery`（必须非空）、`TargetTagQuery`（可选）、`Effects`（Sound/Niagara/Cascade 软路径）。 |
| `USigilContextEffectsSubsystem` | 世界子系统：按 Actor 注册库并生成效果（`SpawnContextEffects`、`SpawnContextEffectsExt`、`GetContextFromSurfaceType`）。 |
| `USigilContextEffectsSettings` | 开发者设置：`SurfaceTypeToContextMap`、编辑器预览开关 + `PreviewSetting`。 |
| `USigilContextEffectComponent` | Actor 侧接收端：默认库/情景、三来源情景聚合、物理表面注入、标签提供者挂接。 |
| `ISigilContextEffectsInterface` | 含 `PlayContextEffectsWithInput` 的接口；实现它即可接收通知驱动的播放。 |
| `USigilAnimNotify_ContextEffects` | "Play Context Effects" 动画通知：附加/追踪设置、VFX/音频参数、编辑器预览。 |
| `USigilContextEffectsSpawnParametersProvider` | 实例化、可蓝图化的提供者：通知不附加时自定义生成位置/旋转。 |
| `FSigilSpawnContextEffectsInput` / `FSigilSpawnContextEffectsOutput` | 生成请求载荷（情景、附加、VFX/音频参数、命中结果）与生成结果组件。 |
| `USigilContextEffectsPreviewSetting` | 仅编辑器的预览数据（如预览用物理表面）。 |

## 配置（Configuration）

- **项目设置 → `USigilContextEffectsSettings`**（config `Game`）：`SurfaceTypeToContextMap`；`bPreviewInEditor` + `PreviewSetting`（仅编辑器）。
- **库资产** —— 条目列表；`SourceTagQuery` 为空的条目加载时被静默丢弃。
- **组件** —— `DefaultContextEffectsLibraries`、`DefaultEffectContexts`、`bConvertPhysicalSurfaceToContext`、`FallbackPhysicalSurface`、`bAutoSetupTagsProvider` / `SetGameplayTagsProvider`；运行时可用 `UpdateLibraries` / `UpdateEffectContexts` 替换。
- **通知** —— 效果标签、附加或生成参数提供者、追踪设置、VFX 缩放、音量/音高。

## 网络（Networking）

完全本地、纯表现。没有任何同步：效果在触发所在的机器上生成（动画通知在动画播放到的每台机器上触发，包括各客户端）。若需要服务器权威的效果事件，请用你自己的玩法系统同步触发条件，再由各客户端本地播放。

## 相关文档（See Also）

- [sigil.interaction](sigil-interaction.zh-CN.md) —— 交互动画是情景效果通知的天然来源。
- [sigil.input](sigil-input.zh-CN.md) —— 触发这些动画的玩法所用的输入层。
