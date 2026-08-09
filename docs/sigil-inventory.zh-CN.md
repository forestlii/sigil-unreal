[English](sigil-inventory.md) | [简体中文](sigil-inventory.zh-CN.md)

# sigil.inventory

**插件：** `SigilInventory` · **模块：** `SigilInventory`（Runtime） · **依赖：** ModularGameplay（引擎插件）、Gameplay Tags、Developer Settings

sigil.inventory（日志与分类中简称 **GIS**，即 Generic Inventory System）是一套基于「数据片段（Fragment）」的库存框架，覆盖堆叠、槽位/多栈集合、装备、拾取、掉落、货币、商店、合成与存档序列化。道具数据被拆成界限分明的三层：**常量定义**（数据资产）、**无状态常量片段**（组合式扩展）、**外置运行时状态**（`FInstancedStruct` Mixin）——同一个 `USigilItemDefinition` 资产可以被任意数量的运行时道具实例共享，互不干扰。

## 总览

### 道具模型：定义 + 片段 + Mixin

`USigilItemDefinition` 是 `Const` 主数据资产：显示名/描述/图标、`bUnique` 标记（唯一道具不可堆叠）、`ItemTags`、静态的「标签→浮点 / 标签→整数」属性表，以及 `Fragments` 数组（同类型片段不允许重复）。

`USigilItemFragment` 是组合单元——挂在定义上的 `Const` 无状态对象。内置片段：`USigilItemFragment_DynamicAttributes`、`USigilItemFragment_Equippable`、`USigilItemFragment_Shoppable`、`USigilItemFragment_CraftingRecipe`。片段自身不能存每个道具的状态，而是声明一个**兼容的 Mixin 数据类型**（`GetCompatibleMixinDataType` / `MakeDefaultMixinData` / `IsMixinDataSerializable`）。

`USigilItemInstance` 是由定义创建的运行时对象（以子对象方式复制，用 `FGuid` 作为道具 id）。它的 `FSigilMixinContainer`——一个由 `FSigilMixin` 条目组成的 FastArray——为每个片段外挂一份 `FInstancedStruct` 运行时状态。这就是 Mixin 模式：*常量对象 + 外部附加的运行时结构体*。片段状态变化通过 `FSigilItemFragmentStateEventSignature` 委托和蓝图节点 `USigilAsyncAction_WaitItemFragmentDataChanged` 对外广播；实例还带两个可复制的动态标签属性容器（`FSigilGameplayTagFloatContainer` / `FSigilGameplayTagIntegerContainer`），同样有变更事件。

### 三层容器

1. **Stack（堆）**——`FSigilItemStack`（道具实例 + 数量），存放在 `FSigilItemStackContainer` FastArray 里。
2. **Collection（集合）**——由 `USigilItemCollectionDefinition` 数据资产（集合标签、内联的 `USigilItemRestriction` 限制列表、溢出选项）实例化出 `USigilItemCollection`。两种特化形态：
   - `USigilItemSlotCollectionDefinition` / 槽位集合——「槽对道具」布局，适合装备栏、快捷栏；槽位由带 `Sigil.Inventory.Slots.*` 标签的 `FSigilItemSlotDefinition` 定义，支持槽位分组，`bNewItemPriority` 控制冲突时是否顶替旧道具。
   - `USigilItemMultiStackCollectionDefinition` / `USigilItemMultiStackCollection`——背包式存储，同一道具可占多个栈；栈上限取 `DefaultStackSizeLimit`（默认 99），或由 `StackSizeLimitAttribute` 指定的道具整数属性决定。

   内置限制器：`USigilItemRestriction_StackSizeLimit`、`_StacksNumLimit`、`_TagRequirements`、`_UniqueOnly`。
3. **Inventory（库存）**——`USigilInventorySystemComponent` 持有集合的 `FSigilCollectionContainer`，对外提供事务式 API：`CanAddItem`/`AddItem`/`AddItems`/`AddItemByDefinition`、`CanMoveItem`/`MoveItem`、`CanRemoveItem`/`RemoveItem`，以及按 `Sigil.Inventory.Collection.*` 标签查询集合。增删与堆叠变化以消息结构体广播（`FSigilInventoryStackUpdateMessage` 服务器客户端都触发；添加/移除/被拒消息仅服务器端）。

### 装备

`USigilEquipmentSystemComponent`（`UPawnComponent`）盯住一个槽位集合（标签可配，惯例用 `Sigil.Inventory.Collection.Equipped`）。当带 `USigilItemFragment_Equippable` 片段的道具进入受监控的槽位，组件调用 `CreateEquipmentInstance`，按片段的 `InstanceType` 创建装备对象——任何实现 `ISigilEquipmentInterface` 的类都行（对象或 Actor 均可，`bActorBased` 自动推断）。`USigilEquipmentInstance` 是现成的默认实现，可按片段的 `ActorsToSpawn` 在 Pawn 身上生成武器模型等 Actor。

接口生命周期回调：`OnEquipmentBeginPlay` / `OnEquipmentEndPlay`（旧的 `OnEquipped` / `OnUnequipped` 仍在但已**标记弃用**）、`OnActiveStateChanged`，以及面向切枪流程的分组激活索引切换（`FSigilEquipment_GroupActiveIndexChangedSignature`）；`ReceiveOwningPawn` / `ReceiveSourceItem` 让装备知道自己属于哪个 Pawn、来自哪件道具。装备条目通过 `FSigilEquipmentContainer` FastArray 复制。

### 世界道具、拾取、掉落

- `USigilWorldItemComponent` 表示躺在关卡里的道具：持有可复制的 `FSigilItemInfo`，若道具实例由它创建则注册为复制子对象。
- `USigilPickupComponent`（抽象）→ `USigilItemPickupComponent`、`USigilInventoryPickupComponent`、`USigilCurrencyPickupComponent`；拾取 Actor 可实现 `ISigilPickupActorInterface`。
- `USigilDropperComponent`（抽象）→ `USigilItemDropperComponent`、`USigilRandomItemDropperComponent`、`USigilCurrencyDropper`，负责战利品生成。

### 货币、商店、合成

- **货币**：`USigilCurrencyDefinition` 数据资产；余额存在 `USigilCurrencySystemComponent` 的 `FSigilCurrencyContainer` FastArray 里。库存组件会自动发现同一 Owner 上的货币组件（`GetCurrencySystem`）。
- **商店**：`USigilShopSystemComponent` 基于一套后备库存 + 顾客的货币系统实现 `BuyItem` / `SellItem`，校验入口可覆写（`CanBuyerBuyItem`、`CanSellerSellItem`、`IsItemBuyable` 等），另有条件接口（`ISigilShopBuyCondition` / `ISigilShopSellCondition`）；价格来自 `USigilItemFragment_Shoppable`。
- **合成**：`USigilCraftingSystemComponent` 消费以 `USigilItemFragment_CraftingRecipe` 片段声明的配方（`InputItems`、`InputCurrencies`、`OutputItems`）。

### 序列化与工厂

`USigilInventorySubsystem`（`UGameInstanceSubsystem`）是道具创建与存档的门面：`CreateItem`、`DuplicateItem`、`SerializeItem`/`DeserializeItem`、`SerializeCollection`/`DeserializeCollection`，以及整库序列化 `SerializeInventory`/`DeserializeInventory`，产物为 `FSigilItemRecord` / `FSigilCollectionRecord` / `FSigilInventoryRecord`（定义在 `SigilSerializationStructLibrary.h`）。所有底层操作都委托给**可替换**的 `USigilInventoryFactory`（`BlueprintNativeEvent`，蓝图可覆写），由项目设置指定——继承它即可完全接管道具构建与序列化。`USigilInventorySystemComponent::InitializeInventorySystemWithRecord` 用存档记录恢复库存。

### Schema 校验

`USigilItemDefinitionSchema` 是编辑器期的校验资产：每条 `FSigilItemDefinitionValidationEntry` 用标签查询匹配道具定义，并强制要求实例类、必需/禁止的片段、必需的浮点/整数属性，还可强制 `bUnique` 取值。Schema 在项目设置中按内容路径前缀绑定，`IsDataValid` 会在编辑阶段把不合规的道具定义标出来。

## 前置要求

插件**不含任何内容资产**（`CanContainContent: false`）——没有示例道具、集合或 UI。你需要：

- **Owner Actor 开启子对象列表复制。** 每个道具实例与集合都以子对象方式复制，宿主 Actor **必须**开启 `bReplicateUsingRegisteredSubObjectList`。没开的话，`USigilInventorySystemComponent` 与 `USigilEquipmentSystemComponent` 会在 `InitializeComponent` 打 Error 日志——*"requires enable bReplicateUsingRegisteredSubObjectList."*（组件自身的组件级开关它们已自行设置）。
- **Gameplay Tags。** 插件原生注册：
  - `Sigil.Inventory.Collection.{Main, Equipped, Hidden, QuickBar}`
  - `Sigil.Inventory.Attribute.StackSizeLimit`

  **`Sigil.Inventory.Slots.*` 没有任何原生标签**——槽位标签（如 `Sigil.Inventory.Slots.Weapon.Primary`）必须项目自建；槽位集合的属性都限定在这个前缀下过滤。四个内置集合标签之外的集合标签同样由项目自建。
- **项目设置**（见「配置方式」）：`InventoryFactoryClass` 用默认值即可跑通，但 Schema 校验要等你配好 Schema 才生效。
- **装备宿主**：`USigilEquipmentSystemComponent` 是 `UPawnComponent`，必须挂在 `APawn` 上。
- **玩法整合自己做**——见下方「已知留白」；尤其注意插件**与 GAS 零耦合**：装备时授予技能、加属性是刻意留给项目做的。

## 快速上手

1. **建标签。** 在 **项目设置 → Gameplay Tags** 里建好 `Sigil.Inventory.Slots.*` 槽位标签，以及需要的额外 `Sigil.Inventory.Collection.*` 集合标签。

2. **建道具定义。** 新建 `USigilItemDefinition` 资产：名称、图标、`ItemTags`、静态属性（如 `Sigil.Inventory.Attribute.StackSizeLimit → 20`）、片段（`Equippable Settings`、`Crafting Recipe Settings` 等）。

3. **建集合定义。**
   - 背包：`USigilItemMultiStackCollectionDefinition`，标签 `Sigil.Inventory.Collection.Main`（设 `DefaultStackSizeLimit`，需要的话再设 `StackSizeLimitAttribute`）。
   - 装备栏：`USigilItemSlotCollectionDefinition`，标签 `Sigil.Inventory.Collection.Equipped`，每个装备位一条 `FSigilItemSlotDefinition`。
   - 按需挂 `USigilItemRestriction` 实例（标签要求、栈上限、仅唯一等）。

4. **给角色/PlayerState 挂组件。** Actor 上开启 `bReplicateUsingRegisteredSubObjectList`；挂 `USigilInventorySystemComponent` 并登记集合定义；用到货币/装备就再挂 `USigilCurrencySystemComponent` 与 `USigilEquipmentSystemComponent`（指向装备集合标签）。在服务器端调用 `InitializeInventorySystem()`（或用蓝图异步节点 `USigilAsyncAction_WaitInventorySystem` / `...Initialized` 等它就绪）。

5. **搬运道具**（服务器权威；客户端走自带的 `Server*` RPC）：

   ```cpp
   USigilInventorySystemComponent* Inv =
       USigilInventorySystemComponent::GetInventorySystemComponent(Actor);

   // 往主背包塞 5 瓶药水
   Inv->AddItemByDefinition(SigilCollectionTags::Main, PotionDefinition, 5);

   // 穿戴：把道具移入槽位集合
   FSigilItemInfo Move = Info;                       // 来自查询或事件
   Move.CollectionTag = SigilCollectionTags::Equipped;
   if (Inv->CanMoveItem(Move)) { Inv->MoveItem(Move); }
   ```

6. **实现装备行为。** 继承 `USigilEquipmentInstance`（或让某个 Actor 实现 `ISigilEquipmentInterface`），把它设为道具 `Equippable Settings` 片段的 `InstanceType`，覆写 `OnEquipmentBeginPlay` / `OnEquipmentEndPlay` / `OnActiveStateChanged`——挂模型、授予 GAS 技能、加属性、切运动集（见 sigil.movement）都在这里做。

7. **存档 / 读档。**

   ```cpp
   USigilInventorySubsystem* Sub = USigilInventorySubsystem::Get(this);
   FSigilInventoryRecord Record;
   Sub->SerializeInventory(Inv, Record);             // → 存进你的 SaveGame
   // 之后：
   Inv->InitializeInventorySystemWithRecord(Record); // 仅服务器
   ```

   自定义片段状态想进存档：Mixin 结构体的字段打上 `SaveGame` 标记、`IsStateSerializable` 返回 true 即可自动参与。

## 关键类型

| 类型 | 说明 |
| --- | --- |
| `USigilItemDefinition` | Const 主数据资产：显示数据、`bUnique`、`ItemTags`、静态浮点/整数属性、片段列表。 |
| `USigilItemFragment` | 抽象常量片段；声明兼容的运行时 Mixin 结构类型及其默认值/可否序列化。 |
| `USigilItemInstance` | 可复制的运行时道具（GUID id、定义指针、Mixin 容器、动态标签属性、变更委托）。 |
| `FSigilMixinContainer` / `FSigilMixin` | 把 `FInstancedStruct` 运行时状态外挂到常量对象上的 FastArray（片段状态的存放处）。 |
| `FSigilItemInfo` / `FSigilItemStack` | 值结构体：道具 + 数量（+ 集合标签/槽位寻址），贯穿整套 API。 |
| `USigilItemCollectionDefinition` / `USigilItemCollection` | 基础集合：标签、内联限制器、溢出选项。 |
| `USigilItemSlotCollectionDefinition` | 槽位式集合（装备栏/快捷栏）：槽位定义、槽位分组、顶替策略。 |
| `USigilItemMultiStackCollectionDefinition` | 背包式多栈集合：默认栈上限、栈上限属性。 |
| `USigilItemRestriction`（+4 个内置） | 可插拔的集合增删门禁对象。 |
| `USigilInventorySystemComponent` | 库存中枢：集合容器、增/移/删 API、初始化/重置、存档恢复、事件。 |
| `USigilEquipmentSystemComponent` | 监控槽位集合的 Pawn 组件；创建/销毁装备实例、分组激活索引切换。 |
| `ISigilEquipmentInterface` / `USigilEquipmentInstance` | 装备契约及默认实现（生成 Actor、生命周期回调）。 |
| `USigilItemFragment_Equippable` | 片段：装备 `InstanceType`、自动激活、待生成 Actor 列表。 |
| `USigilCurrencySystemComponent` / `USigilCurrencyDefinition` | 可复制的货币钱包与货币资产。 |
| `USigilShopSystemComponent` | 基于库存 + 货币系统的买卖，校验条件可覆写。 |
| `USigilCraftingSystemComponent` / `USigilItemFragment_CraftingRecipe` | 配方驱动的合成（输入道具/货币 → 产出道具）。 |
| `USigilWorldItemComponent` + 拾取/掉落组件 | 关卡中的道具、拾取流程、战利品掉落。 |
| `USigilInventorySubsystem` / `USigilInventoryFactory` | 创建与序列化门面 / 可替换的底层工厂。 |
| `USigilItemDefinitionSchema` | 道具定义的编辑器期校验 Schema。 |
| `USigilInventorySystemSettings` | 开发者设置：工厂类、Schema 映射、默认 Schema。 |

## 配置方式

**项目设置 → Sigil Inventory System Settings**（`USigilInventorySystemSettings`，`Config=Game`）：

| 属性 | 说明 |
| --- | --- |
| `InventoryFactoryClass` | 全局使用的 `USigilInventoryFactory` 软类引用，所有道具/集合的构建与（反）序列化都经它。指向你的子类即可全局改底层行为。 |
| `ItemDefinitionSchemaMap` | 「路径前缀 → Schema」条目（如 `/Game/ActionRPG` → `Schema_ARPG`）；前缀下的资产按对应 Schema 校验。 |
| `DefaultItemDefinitionSchema` | 没有前缀命中时的兜底 Schema。 |

其余全是数据资产：道具定义（+片段）、集合定义（+限制器）、货币定义、Schema。定义类实现了 `IsDataValid`/`PreSave`，校验在编辑器里就会跑。

## 网络

- **模型：** 严格**服务器权威、无客户端预测**。所有修改型 API 都是 `BlueprintAuthorityOnly`；客户端通过自带的可靠 `Server*` RPC 发起请求（`ServerAddItemByDefinition`、`ServerMoveItem`、`ServerRemoveItem`、`ServerLoadDefaultLoadouts` 等），等状态复制回来才看到结果。
- **复制机制：** 七个 FastArray 序列化器——`FSigilItemStackContainer`、`FSigilCollectionContainer`、`FSigilMixinContainer`、`FSigilEquipmentContainer`、`FSigilCurrencyContainer`、`FSigilGameplayTagFloatContainer`、`FSigilGameplayTagIntegerContainer`——加上 `USigilItemInstance` / 集合对象的注册子对象复制，以及 Push Model 脏标记（`MARK_PROPERTY_DIRTY_FROM_NAME`）。构建脚本调用了 `SetupIrisSupport`，支持 Iris。
- **宿主要求：** Owner Actor 必须开启 `bReplicateUsingRegisteredSubObjectList`（否则打 Error 日志；见「前置要求」）。
- **仅服务器/本地：** `OnInventoryAddItemInfo`、`OnInventoryAddItemInfo_Rejected`、`OnInventoryRemoveItemInfo` 只在服务器触发；`OnInventoryStackUpdate` 两端都触发。Schema 校验仅编辑器期，不参与网络。

## 已知留白与设计边界

均已在源码核实，做产品前心里有数：

- **与 GAS 零耦合——刻意为之。** 模块内没有任何 Ability System 引用。装备道具**不会**自动授予技能或属性加成。官方预留的接入点是 `ISigilEquipmentInterface::OnEquipmentBeginPlay` / `OnEquipmentEndPlay`（旧的 `OnEquipped`/`OnUnequipped` 已弃用）以及覆写 `USigilEquipmentSystemComponent::CreateEquipmentInstance`——ASC 相关调用接在这里。
- **没有 UI 层。** 消息与委托（堆叠更新、被拒、集合增删）为 UI 准备好了，但不带任何控件。
- **库存操作没有客户端预测**；对延迟敏感的流程（快速换装/换槽）会感受到一个服务器往返的延迟。
- **槽位标签归项目所有**（`Sigil.Inventory.Slots.*` 无原生条目）；`Main` / `Equipped` / `Hidden` / `QuickBar` 之外的集合标签同理。
- `OnEquipped` / `OnUnequipped` 为兼容保留、已标 `DeprecatedFunction`——新代码请用 `OnEquipmentBeginPlay` / `OnEquipmentEndPlay`。

## 相关文档

- [sigil.movement](sigil-movement.zh-CN.md)——在装备回调里压入武器运动定义。
- [sigil.input](sigil-input.zh-CN.md)——标签驱动输入，适合使用道具/快捷栏操作。
