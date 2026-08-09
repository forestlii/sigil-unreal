[English](sigil-ui.md) | [简体中文](sigil-ui.zh-CN.md)

# sigil.ui

**Plugin:** `SigilUI` · **Modules:** `SigilUI` (Runtime) · **Depends on:** CommonUI, Enhanced Input, ModularGameplay (engine plugins)

sigil.ui is a CommonUI extension layer providing a Lyra-style layered game UI: a Policy/Layout pair managed by a game-instance subsystem, gameplay-tag-addressed widget layers, data-driven UI actions with confirmation modals, UI extension points, widget factories for ListView/TileView, and mobile virtual-stick widgets. It is a framework, not a widget library — nearly every visual piece is an `Abstract` base your project must subclass.

## Overview

### Policy / Layout / Layers

Three cooperating pieces form the backbone:

- **`USigilGameUISubsystem`** (`UGameInstanceSubsystem`) — owns the current **policy**. On `Initialize` it loads `USigilUISettings::GameUIPolicyClass` from project settings; if that setting is unset it logs an error and the entire UI system stays inert. The subsystem is skipped on dedicated servers, and the base class steps aside automatically if your project defines a derived subsystem class.
- **`USigilGameUIPolicy`** (`Abstract, Blueprintable`, `Within=SigilGameUISubsystem`) — decides which **`LayoutClass`** to spawn per local player and manages one root layout per player (`RootViewportLayouts`), including local multiplayer behavior (`ESigilLocalMultiplayerInteractionMode`: `PrimaryOnly` / `SingleToggle` / `Simultaneous`). Blueprint hooks: `OnRootLayoutAddedToViewport`, `OnRootLayoutRemovedFromViewport`, `OnRootLayoutReleased`.
- **`USigilGameUILayout`** (`Abstract`, a `UCommonUserWidget`) — the per-player root widget. Your layout Blueprint places `UCommonActivatableWidgetContainerBase` widgets (stacks/queues) and calls **`RegisterLayer(LayerTag, LayerWidget)`** for each, with tags under the `Sigil.UI.Layer` category. Content is then pushed by tag: `PushWidgetToLayerStack` / `PushWidgetToLayerStackAsync` (C++ templates), the Blueprint async node `USigilAsyncAction_PushContentToUILayer::PushContentToUILayer(ForPlayer)`, or `USigilGameUIFunctionLibrary::PushContentToUILayer_ForPlayer` / `PopContentFromUILayer_ForPlayer`.

The plugin natively declares only **one** layer tag: `Sigil.UI.Layer.Modal`. All other layer tags (HUD, menu, etc.) are defined and registered by your project.

**There is no automatic player hookup.** `AddPlayer(LocalPlayer)` / `RemovePlayer(LocalPlayer)` on the subsystem create/remove the per-player layout, but nothing in the plugin calls them — your project must call them itself (typically from a `UGameInstance` subclass reacting to local players being added/removed).

### Activatable widgets and input

`USigilActivatableWidget` extends `UCommonActivatableWidget` with a declarative input config: `InputConfig` (`ESigilActivatableWidgetInputMode`: `Default` / `GameAndMenu` / `Game` / `Menu`) and `GameMouseCaptureMode` decide the UI input mode applied while the widget is active; `SetIsBackHandler` toggles back-action handling. `USigilGameUIFunctionLibrary` adds input helpers (`SuspendInputForPlayer` / `ResumeInputForPlayer`, input-type queries such as `IsOwningPlayerUsingGamepad`).

### Modals

Modals are defined as **class default objects**: subclass `USigilModalDefinition` (`Abstract, Const`) in Blueprint and fill its defaults — `Header`, `Body`, `ModalWidget` (a `USigilGameModalWidget` class), and `ModalActions`, a map from tags under `Sigil.UI.Modal.Action` to `FSigilGameModalAction` (`DisplayText`, `ButtonType` — a `USigilButtonBase` class — and an `InputAction` row of type `CommonInputActionDataBase`). Native action tags: `Sigil.UI.Modal.Action.Ok` / `Cancel` / `Yes` / `No` / `Unknown`.

`USigilGameModalWidget` is the abstract widget base; its Blueprint **must** contain these `BindWidget` children: a `UDynamicEntryBox` named **`EntryBox_Buttons`**, and `UCommonTextBlock`s named **`Text_Header`** and **`Text_Body`**. Show a modal with the Blueprint async node `USigilAsyncAction_ShowModel::ShowModal(WorldContextObject, ModalDefinition)`; it loads the definition's CDO, pushes the widget to the `Sigil.UI.Layer.Modal` layer of the player's root layout, and reports the clicked action tag via `OnModalAction`. If no layout/player is available it immediately reports `Sigil.UI.Modal.Action.Unknown` — so a modal that "does nothing" usually means the Modal layer was never registered or the policy/layout is missing.

### Data-driven UI actions

- **`USigilUIAction`** (`Abstract, Const, EditInlineNew, DefaultToInstanced`) — one action: `DisplayName`, `ActionID`, `InputActionData` (CommonUI row), `bShouldDisplayInActionBar`, and optional confirmation (`bRequiresConfirmation` + `ConfirmationModalClass`). Override `IsCompatibleInternal` / `CanInvokeInternal` / `InvokeActionInternal` (BlueprintNativeEvents).
- **`USigilUIActionFactory`** (data asset) — holds instanced `PotentialActions` and answers `FindAvailableUIActionsForData(Data)`.
- **`USigilUIActionWidget`** (invisible `UCommonUserWidget`) — call `SetAssociatedData(Data)`; it consults its `ActionFactory`, registers CommonUI input bindings for each available action, funnels invocations through the confirmation modal when required, and broadcasts `OnHandleUIAction`.
- The subsystem also exposes raw binding registration: `RegisterUIActionBindingForPlayer` / `UnregisterUIActionBindingForPlayer` (older `RegisterUIActionBinding` / `UnregisterBinding` are deprecated), plus shareable per-player context objects (`USigilGameUIContext`, `RegisterUIContextForPlayer` / `RegisterUIContextForActor` / `FindUIContextForPlayer` / `FindUIContextFromHandle` / `UnregisterUIContextForPlayer`).

### UI extension points

`USigilExtensionSubsystem` (`UWorldSubsystem`) decouples "here is a slot in the HUD" from "someone wants to inject a widget there":

- Layouts place a **`USigilGameUIExtensionPointWidget`** (a `UDynamicEntryBoxBase`) configured with `ExtensionPointTag`, `ExtensionPointTagMatch` (`ExactMatch` / `PartialMatch`), allowed `DataClasses`, and the bindable events `GetWidgetClassForData` / `ConfigureWidgetForData` for non-widget data. The widget's context object is the owning `LocalPlayer`.
- Gameplay code registers extensions: `RegisterExtensionAsWidget(ForContext)` / `RegisterExtensionAsData(ForContext)` (C++), or the Blueprint nodes *Register Extension (Widget)* / *(Widget For Context)* / *(Data)* / *(Data For Context)* and *Register Extension Point*. Handles (`FSigilGameUIExtHandle` / `FSigilGameUIExtPointHandle`) unregister via `Unregister` or `USigilExtensionFunctionLibrary`.

### Widget factories and list views

`USigilWidgetFactory` (`Abstract, Blueprintable` data asset) answers `FindWidgetClassForData(Data)`. `USigilListView` / `USigilTileView` extend `UCommonListView` / `UCommonTileView` with an `EntryWidgetFactories` array so the entry widget class is chosen per item at generation time. `USigilListEntry` (a `USigilButtonBase` implementing `IUserObjectListEntry`) is the entry base; `USigilGameUIFunctionLibrary::GetTypedListItem` / `GetTypedListItemSafely` fetch the typed item from an entry. Detail-view helpers (`USigilListEntryDetailView`, `USigilListEntryDetailSection`, `SigilDetailSectionsBuilder`) support building selection-detail panes.

### Foundation and mobile widgets

- `USigilButtonBase` (`UCommonButtonBase`): `SetButtonText`, `bOverride_ButtonText` / `ButtonText` (otherwise text comes from the input action widget), Blueprint events `OnUpdateButtonText` / `OnUpdateButtonStyle`.
- Tab system: `USigilTabListWidgetBase` with `FSigilTabDescriptor` (`TabId`, `TabText`, `IconBrush`, `TabButtonType` — must implement `SigilTabButtonInterface` — and optional `TabContentType`), plus `USigilTabButtonBase`.
- Mobile: `USigilSimulatedInputWidget` injects keys into Enhanced Input (`AssociatedAction`, `FallbackBindingKey`, `InputKeyValue` / `InputKeyValue2D`, `FlushSimulatedInput`) and **requires** a `BindWidget` child `CommonVisibilityBorder` (`UCommonHardwareVisibilityBorder`); `USigilJoystickWidget` builds a virtual analog stick on top (`BindWidget` images `JoystickBackground` / `JoystickForeground`, `StickRange`, `bNegateYAxis`).

## Prerequisites

- [ ] **CommonUI configured for your project** — viewport class, input action DataTable(s) of row type `CommonInputActionDataBase`, and input data per CommonUI's own setup. The plugin builds on CommonUI everywhere.
- [ ] **`GameUIPolicyClass` set** in **Project Settings → Generic UI System Settings** (`USigilUISettings`). Unset ⇒ the subsystem logs `Missing GameUIPolicyClass` and the whole UI system does nothing.
- [ ] **A policy Blueprint** (subclass of `USigilGameUIPolicy`) with `LayoutClass` pointing at your layout.
- [ ] **A layout Blueprint** (subclass of `USigilGameUILayout`) that contains your `CommonActivatableWidgetContainer` stacks and calls `RegisterLayer` for each layer tag — including `Sigil.UI.Layer.Modal` if you use modals or UI-action confirmations.
- [ ] **Manual player hookup** — call `USigilGameUISubsystem::AddPlayer` / `RemovePlayer` from your GameInstance (or equivalent) when local players are added/removed. Nothing does this for you.
- [ ] **Your own layer tags** under `Sigil.UI.Layer.*` for any non-modal layers.
- [ ] For modals: widget Blueprints honoring the `EntryBox_Buttons` / `Text_Header` / `Text_Body` BindWidget contract, and a `USigilButtonBase` subclass for buttons.

## Quick Start

1. **Create the layout.** Blueprint subclass of `USigilGameUILayout`; add one `CommonActivatableWidgetStack` per layer; on construct, call `RegisterLayer` with your `Sigil.UI.Layer.*` tag for each (register `Sigil.UI.Layer.Modal` too).
2. **Create the policy.** Blueprint subclass of `USigilGameUIPolicy`; set `LayoutClass` to the layout from step 1.
3. **Point settings at the policy.** Project Settings → *Generic UI System Settings* → `GameUIPolicyClass`.
4. **Wire players.** In your GameInstance, call `AddPlayer(LocalPlayer)` when a local player is added and `RemovePlayer` when removed. At this point each local player gets a root layout in the viewport.
5. **Push content.** From Blueprint use *Push Content To UI Layer (For Player)* with a `UCommonActivatableWidget` subclass and a layer tag; or `USigilGameUIFunctionLibrary::PushContentToUILayer_ForPlayer` for the synchronous path. Pop with `PopContentFromUILayer`.
6. **Add a modal.** Subclass `USigilGameModalWidget` (respecting the three BindWidget names) and `USigilModalDefinition` (fill Header/Body/ModalWidget/ModalActions in class defaults). Show it with the *Show Modal* async node and branch on the returned action tag.
7. **Optional — UI actions, extension points, factories, mobile sticks** as described in Overview.

## Key Types

| Type | Description |
| --- | --- |
| `USigilUISettings` | Developer settings ("Generic UI System Settings"); holds `GameUIPolicyClass`. Mandatory configuration. |
| `USigilGameUISubsystem` | GameInstance subsystem owning the policy; `AddPlayer` / `RemovePlayer` (manual), action-binding and UI-context registration. |
| `USigilGameUIPolicy` | Abstract policy: layout class per local player, local-multiplayer mode, root-layout lifecycle hooks. |
| `USigilGameUILayout` | Abstract per-player root widget; `RegisterLayer`, `PushWidgetToLayerStack(Async)`, `FindAndRemoveWidgetFromLayer`, `GetLayerWidget`. |
| `USigilActivatableWidget` | Activatable widget with declarative input mode (`InputConfig`, `GameMouseCaptureMode`) and back-handler toggle. |
| `USigilModalDefinition` / `USigilGameModalWidget` | Modal data (CDO-as-data) and abstract modal widget (BindWidget: `EntryBox_Buttons`, `Text_Header`, `Text_Body`). |
| `USigilAsyncAction_ShowModel` | Blueprint async node *Show Modal*; result tag via `OnModalAction`. |
| `USigilAsyncAction_PushContentToUILayer` / `USigilAsyncAction_CreateWidget` | Async push-to-layer (soft class, optional input suspension) and async widget creation. |
| `USigilUIAction` / `USigilUIActionFactory` / `USigilUIActionWidget` | Data-driven action object, factory asset, and the invisible widget that binds available actions for a data object. |
| `USigilGameUIContext` | Abstract shared context object registered per player/actor through the subsystem. |
| `USigilExtensionSubsystem` / `USigilGameUIExtensionPointWidget` | World subsystem for extension registration and the layout-side extension point widget. |
| `USigilWidgetFactory` / `USigilListView` / `USigilTileView` / `USigilListEntry` | Per-item widget class selection for CommonUI list views. |
| `USigilButtonBase` / `USigilTabListWidgetBase` / `FSigilTabDescriptor` | Foundation button with text override and the tab-list system. |
| `USigilSimulatedInputWidget` / `USigilJoystickWidget` | Enhanced-Input key injection base and the mobile virtual joystick. |
| `USigilGameUIFunctionLibrary` | Push/pop helpers, layout lookup, input suspension, input-type queries, typed list-item getters. |

## Configuration

- **Project Settings → Generic UI System Settings** — `GameUIPolicyClass` (the only project setting, and the system's on/off switch in practice).
- **Gameplay tags** — plugin-native: `Sigil.UI.Layer.Modal` and `Sigil.UI.Modal.Action.{Ok,Cancel,Yes,No,Unknown}`; everything else is project-defined under `Sigil.UI.Layer.*`.
- **Assets** — policy/layout Blueprints, modal definitions, `USigilUIActionFactory` and `USigilWidgetFactory` data assets. Factories and action factories implement `IsDataValid` for editor-time validation.

## Networking

Entirely local (client-side). The subsystem is not created on dedicated servers, and nothing in the module replicates. Any gameplay consequences of UI actions must go through your own game systems.

## See Also

- [sigil.input](sigil-input.md) — game-side input; sigil.ui's simulated-input widgets inject into Enhanced Input, which that layer consumes.
- [sigil.interaction](sigil-interaction.md) — interaction options are designed to surface through UI built on this layer.
