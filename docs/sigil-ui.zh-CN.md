[English](sigil-ui.md) | [简体中文](sigil-ui.zh-CN.md)

# sigil.ui

**插件：** `SigilUI` · **模块：** `SigilUI`（Runtime） · **依赖：** CommonUI、Enhanced Input、ModularGameplay（引擎插件）

sigil.ui 是一层建立在 CommonUI 之上的扩展框架，提供 Lyra 风格的分层游戏 UI：由 GameInstance 子系统管理的 Policy/Layout 组合、用 GameplayTag 寻址的控件层、带确认弹窗的数据驱动 UI 操作、UI 扩展点、ListView/TileView 的控件工厂，以及移动端虚拟摇杆。它是框架不是控件库——几乎所有可见部件都是 `Abstract` 基类，需要项目自己继承实现。

## 概述（Overview）

### Policy / Layout / Layer 三件套

骨架由三个部分协作构成：

- **`USigilGameUISubsystem`**（`UGameInstanceSubsystem`）—— 持有当前 **Policy**。`Initialize` 时从项目设置加载 `USigilUISettings::GameUIPolicyClass`；**这个设置不配，子系统只会打一条错误日志，整个 UI 系统就此瘫痪**。专用服务器上不会创建该子系统；如果项目定义了它的派生子系统类，基类会自动让位。
- **`USigilGameUIPolicy`**（`Abstract, Blueprintable`，`Within=SigilGameUISubsystem`）—— 决定每个本地玩家生成哪个 **`LayoutClass`**，并为每人维护一份根布局（`RootViewportLayouts`），含本地多人策略（`ESigilLocalMultiplayerInteractionMode`：`PrimaryOnly` / `SingleToggle` / `Simultaneous`）。蓝图钩子：`OnRootLayoutAddedToViewport`、`OnRootLayoutRemovedFromViewport`、`OnRootLayoutReleased`。
- **`USigilGameUILayout`**（`Abstract`，`UCommonUserWidget`）—— 玩家的根控件。布局蓝图里摆好 `UCommonActivatableWidgetContainerBase`（栈/队列），并为每个容器调用 **`RegisterLayer(LayerTag, LayerWidget)`**，标签限定在 `Sigil.UI.Layer` 类目下。之后按标签推内容：C++ 模板 `PushWidgetToLayerStack` / `PushWidgetToLayerStackAsync`、蓝图异步节点 `USigilAsyncAction_PushContentToUILayer::PushContentToUILayer(ForPlayer)`，或 `USigilGameUIFunctionLibrary::PushContentToUILayer_ForPlayer` / `PopContentFromUILayer_ForPlayer`。

插件原生只声明了**一个**层标签：`Sigil.UI.Layer.Modal`。其余层标签（HUD、菜单等）全部由项目自己定义并注册。

**没有任何自动的玩家挂钩。** 子系统的 `AddPlayer(LocalPlayer)` / `RemovePlayer(LocalPlayer)` 负责创建/移除玩家布局，但插件里没有任何代码会调它们——必须由项目自己调（通常在 `UGameInstance` 子类里响应本地玩家增删时调用）。

### 可激活控件与输入

`USigilActivatableWidget` 在 `UCommonActivatableWidget` 基础上加了声明式输入配置：`InputConfig`（`ESigilActivatableWidgetInputMode`：`Default` / `GameAndMenu` / `Game` / `Menu`）与 `GameMouseCaptureMode` 决定控件激活期间的 UI 输入模式；`SetIsBackHandler` 切换后退处理。`USigilGameUIFunctionLibrary` 提供输入辅助（`SuspendInputForPlayer` / `ResumeInputForPlayer`，以及 `IsOwningPlayerUsingGamepad` 等输入类型查询）。

### 模态弹窗（Modal）

Modal 的定义方式是**拿类默认值当数据**：蓝图继承 `USigilModalDefinition`（`Abstract, Const`），在类默认值里填 `Header`、`Body`、`ModalWidget`（`USigilGameModalWidget` 类）和 `ModalActions`——一个从 `Sigil.UI.Modal.Action` 类目标签到 `FSigilGameModalAction`（`DisplayText`、`ButtonType`（`USigilButtonBase` 类）、`InputAction`（`CommonInputActionDataBase` 行））的映射。原生动作标签：`Sigil.UI.Modal.Action.Ok` / `Cancel` / `Yes` / `No` / `Unknown`。

`USigilGameModalWidget` 是抽象控件基类，其蓝图**必须**包含以下 `BindWidget` 子控件：名为 **`EntryBox_Buttons`** 的 `UDynamicEntryBox`，以及名为 **`Text_Header`**、**`Text_Body`** 的 `UCommonTextBlock`。显示弹窗用蓝图异步节点 `USigilAsyncAction_ShowModel::ShowModal(WorldContextObject, ModalDefinition)`：它加载定义类的 CDO，把控件推到玩家根布局的 `Sigil.UI.Layer.Modal` 层，点击结果通过 `OnModalAction` 返回动作标签。找不到布局/玩家时会立即返回 `Sigil.UI.Modal.Action.Unknown`——所以"弹窗没反应"多半是 Modal 层没注册、或 Policy/Layout 链路没搭起来。

### 数据驱动 UI 操作（UIAction）

- **`USigilUIAction`**（`Abstract, Const, EditInlineNew, DefaultToInstanced`）—— 一条操作：`DisplayName`、`ActionID`、`InputActionData`（CommonUI 行）、`bShouldDisplayInActionBar`，以及可选确认（`bRequiresConfirmation` + `ConfirmationModalClass`）。重写 `IsCompatibleInternal` / `CanInvokeInternal` / `InvokeActionInternal`（均为 BlueprintNativeEvent）。
- **`USigilUIActionFactory`**（数据资产）—— 持有实例化的 `PotentialActions`，回答 `FindAvailableUIActionsForData(Data)`。
- **`USigilUIActionWidget`**（无视觉的 `UCommonUserWidget`）—— 调 `SetAssociatedData(Data)` 后，它会问 `ActionFactory` 拿可用操作、为每条注册 CommonUI 输入绑定、需要确认时先走确认弹窗，最后广播 `OnHandleUIAction`。
- 子系统还暴露底层绑定注册：`RegisterUIActionBindingForPlayer` / `UnregisterUIActionBindingForPlayer`（旧的 `RegisterUIActionBinding` / `UnregisterBinding` 已标记弃用），以及按玩家/Actor 共享的上下文对象（`USigilGameUIContext`，配套 `RegisterUIContextForPlayer` / `RegisterUIContextForActor` / `FindUIContextForPlayer` / `FindUIContextFromHandle` / `UnregisterUIContextForPlayer`）。

### UI 扩展点（UIExtension）

`USigilExtensionSubsystem`（`UWorldSubsystem`）把"HUD 上这里有个坑位"与"谁要往坑位里塞控件"解耦：

- 布局侧摆一个 **`USigilGameUIExtensionPointWidget`**（`UDynamicEntryBoxBase`），配 `ExtensionPointTag`、`ExtensionPointTagMatch`（`ExactMatch` / `PartialMatch`）、允许的 `DataClasses`，以及处理非控件数据的可绑定事件 `GetWidgetClassForData` / `ConfigureWidgetForData`。该控件的上下文对象是所属 `LocalPlayer`。
- 玩法侧注册扩展：C++ 用 `RegisterExtensionAsWidget(ForContext)` / `RegisterExtensionAsData(ForContext)`，蓝图用 *Register Extension (Widget)* / *(Widget For Context)* / *(Data)* / *(Data For Context)* 与 *Register Extension Point* 节点。句柄（`FSigilGameUIExtHandle` / `FSigilGameUIExtPointHandle`）通过 `Unregister` 或 `USigilExtensionFunctionLibrary` 注销。

### 控件工厂与列表视图

`USigilWidgetFactory`（`Abstract, Blueprintable` 数据资产）回答 `FindWidgetClassForData(Data)`。`USigilListView` / `USigilTileView` 在 `UCommonListView` / `UCommonTileView` 之上加了 `EntryWidgetFactories` 数组，让每个条目在生成时动态选择控件类。`USigilListEntry`（继承 `USigilButtonBase` 并实现 `IUserObjectListEntry`）是条目基类；`USigilGameUIFunctionLibrary::GetTypedListItem` / `GetTypedListItemSafely` 用于从条目取回类型化的 Item。另有构建选中详情面板的辅助类（`USigilListEntryDetailView`、`USigilListEntryDetailSection`、`SigilDetailSectionsBuilder`）。

### 基础与移动端控件

- `USigilButtonBase`（`UCommonButtonBase`）：`SetButtonText`、`bOverride_ButtonText` / `ButtonText`（不勾选则用输入动作控件的文字），蓝图事件 `OnUpdateButtonText` / `OnUpdateButtonStyle`。
- 选项卡系统：`USigilTabListWidgetBase` + `FSigilTabDescriptor`（`TabId`、`TabText`、`IconBrush`、`TabButtonType`——须实现 `SigilTabButtonInterface`——和可选 `TabContentType`），以及 `USigilTabButtonBase`。
- 移动端：`USigilSimulatedInputWidget` 向 Enhanced Input 注入按键（`AssociatedAction`、`FallbackBindingKey`、`InputKeyValue` / `InputKeyValue2D`、`FlushSimulatedInput`），且**必须**有名为 `CommonVisibilityBorder` 的 `BindWidget` 子控件（`UCommonHardwareVisibilityBorder`）；`USigilJoystickWidget` 在其上实现虚拟摇杆（`BindWidget` 图片 `JoystickBackground` / `JoystickForeground`，`StickRange`、`bNegateYAxis`）。

## 前置条件（Prerequisites）

- [ ] **项目已按 CommonUI 要求配置好**——视口类、行类型为 `CommonInputActionDataBase` 的输入动作 DataTable、输入数据等。本插件处处依赖 CommonUI。
- [ ] **在项目设置里配好 `GameUIPolicyClass`**（Project Settings → Generic UI System Settings，即 `USigilUISettings`）。不配 ⇒ 子系统报 `Missing GameUIPolicyClass`，整个 UI 系统不工作。
- [ ] **一个 Policy 蓝图**（继承 `USigilGameUIPolicy`），`LayoutClass` 指向你的布局。
- [ ] **一个 Layout 蓝图**（继承 `USigilGameUILayout`），内含各层的 `CommonActivatableWidgetContainer` 并逐一 `RegisterLayer`——用到弹窗或 UI 操作确认的话，`Sigil.UI.Layer.Modal` 也必须注册。
- [ ] **手动挂玩家**——本地玩家增删时由项目在 GameInstance（或等价位置）调用 `USigilGameUISubsystem::AddPlayer` / `RemovePlayer`。没有人替你做这件事。
- [ ] **自定义层标签**——非 Modal 层的 `Sigil.UI.Layer.*` 标签由项目自行创建。
- [ ] 用弹窗的话：遵守 `EntryBox_Buttons` / `Text_Header` / `Text_Body` BindWidget 约定的控件蓝图，以及一个 `USigilButtonBase` 子类做按钮。

## 快速上手（Quick Start）

1. **搭布局。** 蓝图继承 `USigilGameUILayout`；每层放一个 `CommonActivatableWidgetStack`；构造时对每个容器调 `RegisterLayer`（`Sigil.UI.Layer.Modal` 也注册上）。
2. **建 Policy。** 蓝图继承 `USigilGameUIPolicy`，`LayoutClass` 指向第 1 步的布局。
3. **配设置。** Project Settings → *Generic UI System Settings* → `GameUIPolicyClass`。
4. **接玩家。** 在 GameInstance 里，本地玩家加入时调 `AddPlayer(LocalPlayer)`、移除时调 `RemovePlayer`。到这一步，每个本地玩家的根布局就会出现在视口里。
5. **推内容。** 蓝图用 *Push Content To UI Layer (For Player)* 节点传入 `UCommonActivatableWidget` 子类和层标签；同步路径用 `USigilGameUIFunctionLibrary::PushContentToUILayer_ForPlayer`。弹出用 `PopContentFromUILayer`。
6. **加弹窗。** 继承 `USigilGameModalWidget`（遵守三个 BindWidget 名）与 `USigilModalDefinition`（类默认值里填 Header/Body/ModalWidget/ModalActions），用 *Show Modal* 异步节点显示并按返回的动作标签分支。
7. **可选**——按概述接入 UI 操作、扩展点、控件工厂、移动端摇杆。

## 关键类型（Key Types）

| 类型 | 说明 |
| --- | --- |
| `USigilUISettings` | 开发者设置（"Generic UI System Settings"），持有 `GameUIPolicyClass`。必配项。 |
| `USigilGameUISubsystem` | GameInstance 子系统：持有 Policy，`AddPlayer` / `RemovePlayer`（手动调用），操作绑定与 UI 上下文注册。 |
| `USigilGameUIPolicy` | 抽象 Policy：每玩家布局类、本地多人模式、根布局生命周期钩子。 |
| `USigilGameUILayout` | 抽象玩家根控件：`RegisterLayer`、`PushWidgetToLayerStack(Async)`、`FindAndRemoveWidgetFromLayer`、`GetLayerWidget`。 |
| `USigilActivatableWidget` | 带声明式输入模式（`InputConfig`、`GameMouseCaptureMode`）与后退处理开关的可激活控件。 |
| `USigilModalDefinition` / `USigilGameModalWidget` | 弹窗数据（CDO 即数据）与抽象弹窗控件（BindWidget：`EntryBox_Buttons`、`Text_Header`、`Text_Body`）。 |
| `USigilAsyncAction_ShowModel` | 蓝图异步节点 *Show Modal*；结果标签经 `OnModalAction` 返回。 |
| `USigilAsyncAction_PushContentToUILayer` / `USigilAsyncAction_CreateWidget` | 异步推层（软类引用，可暂停输入）与异步创建控件。 |
| `USigilUIAction` / `USigilUIActionFactory` / `USigilUIActionWidget` | 数据驱动的操作对象、工厂资产，以及为数据对象自动注册可用操作的无视觉控件。 |
| `USigilGameUIContext` | 抽象共享上下文对象，经子系统按玩家/Actor 注册。 |
| `USigilExtensionSubsystem` / `USigilGameUIExtensionPointWidget` | 扩展注册的世界子系统与布局侧的扩展点控件。 |
| `USigilWidgetFactory` / `USigilListView` / `USigilTileView` / `USigilListEntry` | CommonUI 列表视图的按条目控件类选择。 |
| `USigilButtonBase` / `USigilTabListWidgetBase` / `FSigilTabDescriptor` | 带文字覆盖的基础按钮与选项卡系统。 |
| `USigilSimulatedInputWidget` / `USigilJoystickWidget` | Enhanced Input 按键注入基类与移动端虚拟摇杆。 |
| `USigilGameUIFunctionLibrary` | 推/弹辅助、布局查找、输入暂停、输入类型查询、类型化列表项获取。 |

## 配置（Configuration）

- **Project Settings → Generic UI System Settings** —— `GameUIPolicyClass`（唯一的项目设置，实际上就是整个系统的总开关）。
- **游戏标签** —— 插件原生：`Sigil.UI.Layer.Modal` 与 `Sigil.UI.Modal.Action.{Ok,Cancel,Yes,No,Unknown}`；其余 `Sigil.UI.Layer.*` 由项目定义。
- **资产** —— Policy/Layout 蓝图、弹窗定义、`USigilUIActionFactory` 与 `USigilWidgetFactory` 数据资产。工厂类实现了 `IsDataValid`，配错会在编辑器数据校验时报出。

## 网络（Networking）

完全本地（客户端侧）。专用服务器不会创建子系统，模块内没有任何同步。UI 操作若要产生玩法后果，须经由你自己的游戏系统。

## 相关文档（See Also）

- [sigil.input](sigil-input.zh-CN.md) —— 游戏侧输入；本插件的模拟输入控件注入 Enhanced Input，由输入层消费。
- [sigil.interaction](sigil-interaction.zh-CN.md) —— 交互选项就设计为通过这一层的 UI 呈现。
