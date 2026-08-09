[English](sigil-inventory.md) | [简体中文](sigil-inventory.zh-CN.md)

# sigil.inventory

**Plugin:** `SigilInventory` · **Modules:** `SigilInventory` (Runtime) · **Depends on:** ModularGameplay (engine plugin), Gameplay Tags, Developer Settings

sigil.inventory (logged and categorized as **GIS** — Generic Inventory System) is a fragment-based inventory framework covering stacks, slot and multi-stack collections, equipment, pickups, drops, currencies, shops, crafting, and save-game serialization. Item data is split into three cleanly separated layers: **const definitions** (data assets), **stateless const fragments** (composition), and **externalized runtime state** (`FInstancedStruct` mixins) — so a single `USigilItemDefinition` asset can be shared by any number of live item instances without duplication.

## Overview

### Item model: Definition + Fragments + Mixins

`USigilItemDefinition` is a `Const` primary data asset: display name/description/icon, a `bUnique` flag (unique items never stack), `ItemTags`, static tag→float / tag→int attribute tables, and an `Fragments` array (no duplicate fragment types allowed).

`USigilItemFragment` is the composition unit — a `Const`, stateless object attached to a definition. Built-in fragments: `USigilItemFragment_DynamicAttributes`, `USigilItemFragment_Equippable`, `USigilItemFragment_Shoppable`, `USigilItemFragment_CraftingRecipe`. A fragment cannot store per-item state itself; instead it declares a **compatible mixin data type** (`GetCompatibleMixinDataType` / `MakeDefaultMixinData` / `IsMixinDataSerializable`).

`USigilItemInstance` is the runtime object created from a definition (replicated as a subobject, identified by a `FGuid` item id). Its `FSigilMixinContainer` — a FastArray of `FSigilMixin` entries — holds one `FInstancedStruct` of runtime state per fragment. This is the mixin pattern: *const object + externally attached runtime struct*. Fragment-state changes surface through `FSigilItemFragmentStateEventSignature` delegates and the `USigilAsyncAction_WaitItemFragmentDataChanged` Blueprint node; instances also carry replicated dynamic tag-attribute containers (`FSigilGameplayTagFloatContainer` / `FSigilGameplayTagIntegerContainer`) with change events.

### Three container tiers

1. **Stack** — `FSigilItemStack` (item instance + amount) inside a `FSigilItemStackContainer` FastArray.
2. **Collection** — `USigilItemCollection` instances created from `USigilItemCollectionDefinition` data assets (tag, instanced `USigilItemRestriction` list, overflow options). Two specialized shapes:
   - `USigilItemSlotCollectionDefinition` / slot collection — slot→item layout for equipment bars and quick bars; slots are defined by `FSigilItemSlotDefinition` entries with `Sigil.Inventory.Slots.*` tags, support slot groups, and `bNewItemPriority` controls replace-on-conflict.
   - `USigilItemMultiStackCollectionDefinition` / `USigilItemMultiStackCollection` — bag-style storage where the same item may occupy several stacks; stack size comes from `DefaultStackSizeLimit` (99) or a per-item integer attribute named by `StackSizeLimitAttribute`.

   Built-in restrictions: `USigilItemRestriction_StackSizeLimit`, `_StacksNumLimit`, `_TagRequirements`, `_UniqueOnly`.
3. **Inventory** — `USigilInventorySystemComponent` owns a `FSigilCollectionContainer` of collections and exposes the transactional API: `CanAddItem`/`AddItem`/`AddItems`/`AddItemByDefinition`, `CanMoveItem`/`MoveItem`, `CanRemoveItem`/`RemoveItem`, plus collection queries by `Sigil.Inventory.Collection.*` tag. Add/remove/stack-update events are broadcast as message structs (`FSigilInventoryStackUpdateMessage` fires on both server and clients; add/remove/rejected messages are server-side only).

### Equipment

`USigilEquipmentSystemComponent` (a `UPawnComponent`) watches one slot collection (its tag is configurable, `Sigil.Inventory.Collection.Equipped` by convention). When an item with a `USigilItemFragment_Equippable` fragment lands in a monitored slot, the component calls `CreateEquipmentInstance` to build an equipment object from the fragment's `InstanceType` — any class implementing `ISigilEquipmentInterface` (object- or actor-based; `bActorBased` is derived automatically). `USigilEquipmentInstance` is the ready-made default implementation and can spawn the fragment's `ActorsToSpawn` (weapon meshes etc.) on the pawn.

Lifecycle callbacks on the interface: `OnEquipmentBeginPlay` / `OnEquipmentEndPlay` (the older `OnEquipped` / `OnUnequipped` still exist but are **deprecated**), `OnActiveStateChanged` plus group-based active-index switching (`FSigilEquipment_GroupActiveIndexChangedSignature`) for weapon-swap flows, and `ReceiveOwningPawn` / `ReceiveSourceItem` so the equipment knows its pawn and source item. Equipment entries replicate through the `FSigilEquipmentContainer` FastArray.

### World items, pickups, drops

- `USigilWorldItemComponent` represents an item lying in the level: it holds a replicated `FSigilItemInfo` and registers the item instance as a replicated subobject when it created it.
- `USigilPickupComponent` (abstract) → `USigilItemPickupComponent`, `USigilInventoryPickupComponent`, `USigilCurrencyPickupComponent`; pickup actors can implement `ISigilPickupActorInterface`.
- `USigilDropperComponent` (abstract) → `USigilItemDropperComponent`, `USigilRandomItemDropperComponent`, `USigilCurrencyDropper` for loot spawning.

### Currency, shop, crafting

- **Currency**: `USigilCurrencyDefinition` data assets; balances live in a `FSigilCurrencyContainer` FastArray on `USigilCurrencySystemComponent`. The inventory component auto-discovers a currency component on the same owner (`GetCurrencySystem`).
- **Shop**: `USigilShopSystemComponent` implements `BuyItem` / `SellItem` against a backing inventory + the customer's currency system, with overridable gates (`CanBuyerBuyItem`, `CanSellerSellItem`, `IsItemBuyable`, …) and condition interfaces (`ISigilShopBuyCondition` / `ISigilShopSellCondition`); prices come from `USigilItemFragment_Shoppable`.
- **Crafting**: `USigilCraftingSystemComponent` consumes recipes declared as `USigilItemFragment_CraftingRecipe` fragments (`InputItems`, `InputCurrencies`, `OutputItems`).

### Serialization and the factory

`USigilInventorySubsystem` (a `UGameInstanceSubsystem`) is the façade for item creation and save games: `CreateItem`, `DuplicateItem`, `SerializeItem`/`DeserializeItem`, `SerializeCollection`/`DeserializeCollection` and whole-inventory serialization into `FSigilItemRecord` / `FSigilCollectionRecord` / `FSigilInventoryRecord` (defined in `SigilSerializationStructLibrary.h`). All low-level operations delegate to a **replaceable** `USigilInventoryFactory` (Blueprint-overridable `BlueprintNativeEvent`s), selected via project settings — subclass it to take full control of item construction and serialization. `USigilInventorySystemComponent::InitializeInventorySystemWithRecord` restores a saved inventory.

### Schema validation

`USigilItemDefinitionSchema` is an editor-time validation asset: per `FSigilItemDefinitionValidationEntry` it matches item definitions by tag query and enforces an instance class, required/forbidden fragments, required float/int attributes, and optionally `bUnique`. Schemas are bound to content paths in project settings, so `IsDataValid` flags malformed item definitions during editing.

## Prerequisites

The plugin ships **no content** (`CanContainContent: false`) — no sample items, collections, or UI. You need:

- **Subobject replication on the owner actor.** Every item instance and collection replicates as a subobject; the owning actor **must** enable `bReplicateUsingRegisteredSubObjectList`. `USigilInventorySystemComponent` and `USigilEquipmentSystemComponent` log an Error at `InitializeComponent` — *"requires enable bReplicateUsingRegisteredSubObjectList."* — if the owner has it off (the components set their own component-level flag themselves).
- **Gameplay Tags.** Native tags registered by the plugin:
  - `Sigil.Inventory.Collection.{Main, Equipped, Hidden, QuickBar}`
  - `Sigil.Inventory.Attribute.StackSizeLimit`

  **`Sigil.Inventory.Slots.*` has no native tags** — slot tags (e.g. `Sigil.Inventory.Slots.Weapon.Primary`) must be created by the project; slot collection properties filter on that prefix. Additional collection tags beyond the four built-ins are also project-defined.
- **Project settings** (see Configuration): at minimum the `InventoryFactoryClass` default is fine, but schema validation only works once you assign schemas.
- **An equipment host**: `USigilEquipmentSystemComponent` is a `UPawnComponent` — it must live on a `APawn`.
- **Your own equipment/gameplay integration** — see *Known gaps* below; notably there is **no GAS coupling**: granting abilities or attribute bonuses on equip is deliberately left to the project.

## Quick Start

1. **Create tags.** Add your slot tags under `Sigil.Inventory.Slots.*` and any extra collection tags under `Sigil.Inventory.Collection.*` in **Project Settings → Gameplay Tags**.

2. **Create item definitions.** Add `USigilItemDefinition` assets: name, icon, `ItemTags`, static attributes (e.g. `Sigil.Inventory.Attribute.StackSizeLimit → 20`), and fragments (`Equippable Settings`, `Crafting Recipe Settings`, …).

3. **Create collection definitions.**
   - A `USigilItemMultiStackCollectionDefinition` tagged `Sigil.Inventory.Collection.Main` for the backpack (set `DefaultStackSizeLimit`, optionally `StackSizeLimitAttribute`).
   - A `USigilItemSlotCollectionDefinition` tagged `Sigil.Inventory.Collection.Equipped` with one `FSigilItemSlotDefinition` per equipment slot.
   - Add `USigilItemRestriction` instances (tag requirements, stack limits, unique-only) as needed.

4. **Add components to your character/PlayerState.** Enable `bReplicateUsingRegisteredSubObjectList` on the actor. Add `USigilInventorySystemComponent` and register your collection definitions; add `USigilCurrencySystemComponent` and `USigilEquipmentSystemComponent` (pointing at the equipped collection tag) if you use those features. Call `InitializeInventorySystem()` on the server (or use the `USigilAsyncAction_WaitInventorySystem` / `...Initialized` Blueprint nodes to wait for readiness).

5. **Move items around** (server-authoritative; client calls go through the provided `Server*` RPCs):

   ```cpp
   USigilInventorySystemComponent* Inv =
       USigilInventorySystemComponent::GetInventorySystemComponent(Actor);

   // Give 5 potions to the main bag
   Inv->AddItemByDefinition(SigilCollectionTags::Main, PotionDefinition, 5);

   // Equip: move an item into the slot collection
   FSigilItemInfo Move = Info;                       // from a query/event
   Move.CollectionTag = SigilCollectionTags::Equipped;
   if (Inv->CanMoveItem(Move)) { Inv->MoveItem(Move); }
   ```

6. **Implement equipment behavior.** Subclass `USigilEquipmentInstance` (or implement `ISigilEquipmentInterface` on an actor), set it as `InstanceType` in the item's `Equippable Settings` fragment, and override `OnEquipmentBeginPlay` / `OnEquipmentEndPlay` / `OnActiveStateChanged` — this is where you attach meshes, grant GAS abilities, apply stat bonuses, or push a movement set (see sigil.movement).

7. **Save / load.**

   ```cpp
   USigilInventorySubsystem* Sub = USigilInventorySubsystem::Get(this);
   FSigilInventoryRecord Record;
   Sub->SerializeInventory(Inv, Record);             // → store in your SaveGame
   // later:
   Inv->InitializeInventorySystemWithRecord(Record); // server only
   ```

   Custom fragment state participates automatically if its mixin struct's fields are `SaveGame`-marked and `IsStateSerializable` returns true.

## Key Types

| Type | Description |
| --- | --- |
| `USigilItemDefinition` | Const primary data asset: display data, `bUnique`, `ItemTags`, static float/int attributes, fragment list. |
| `USigilItemFragment` | Abstract const fragment; declares a compatible runtime mixin struct type and its default value/serializability. |
| `USigilItemInstance` | Replicated runtime item (GUID id, definition pointer, mixin container, dynamic tag attributes, change delegates). |
| `FSigilMixinContainer` / `FSigilMixin` | FastArray attaching `FInstancedStruct` runtime state to const targets (the fragment-state store). |
| `FSigilItemInfo` / `FSigilItemStack` | Value structs: item + amount (+ collection tag / slot addressing) used across the whole API. |
| `USigilItemCollectionDefinition` / `USigilItemCollection` | Base collection: tag, instanced restrictions, overflow options. |
| `USigilItemSlotCollectionDefinition` | Slot-based collection (equipment/quick bars): slot definitions, slot groups, replace policy. |
| `USigilItemMultiStackCollectionDefinition` | Bag-style multi-stack collection: default stack size, stack-size attribute. |
| `USigilItemRestriction` (+ 4 built-ins) | Pluggable add/remove gate objects on a collection. |
| `USigilInventorySystemComponent` | The inventory hub: collection container, add/move/remove API, init/reset, record restore, events. |
| `USigilEquipmentSystemComponent` | Pawn component monitoring a slot collection; creates/destroys equipment instances, group active-index switching. |
| `ISigilEquipmentInterface` / `USigilEquipmentInstance` | Equipment contract and default implementation (actor spawning, lifecycle callbacks). |
| `USigilItemFragment_Equippable` | Fragment: equipment `InstanceType`, auto-activate, actors to spawn. |
| `USigilCurrencySystemComponent` / `USigilCurrencyDefinition` | Replicated currency wallet and currency assets. |
| `USigilShopSystemComponent` | Buy/sell against an inventory + currency system with overridable conditions. |
| `USigilCraftingSystemComponent` / `USigilItemFragment_CraftingRecipe` | Recipe-driven crafting (input items/currencies → output items). |
| `USigilWorldItemComponent` + pickup/dropper components | Items in the level, pickup flows, loot drops. |
| `USigilInventorySubsystem` / `USigilInventoryFactory` | Creation & serialization façade / replaceable low-level factory. |
| `USigilItemDefinitionSchema` | Editor-time validation schema for item definitions. |
| `USigilInventorySystemSettings` | Developer settings: factory class, schema map, default schema. |

## Configuration

**Project Settings → Sigil Inventory System Settings** (`USigilInventorySystemSettings`, `Config=Game`):

| Property | Description |
| --- | --- |
| `InventoryFactoryClass` | Soft class of the `USigilInventoryFactory` used for all item/collection construction and (de)serialization. Point at your subclass to change low-level behavior globally. |
| `ItemDefinitionSchemaMap` | Path-prefix → schema entries (e.g. `/Game/ActionRPG` → `Schema_ARPG`); assets under a prefix validate against that schema. |
| `DefaultItemDefinitionSchema` | Fallback schema when no path prefix matches. |

Everything else is data assets: item definitions (+ fragments), collection definitions (+ restrictions), currency definitions, schemas. Definitions implement `IsDataValid`/`PreSave`, so validation runs in the editor.

## Networking

- **Model:** strictly **server-authoritative, no client prediction**. All mutating APIs are `BlueprintAuthorityOnly`; clients request changes through the provided reliable `Server*` RPCs (`ServerAddItemByDefinition`, `ServerMoveItem`, `ServerRemoveItem`, `ServerLoadDefaultLoadouts`, …) and see results when state replicates back.
- **Replication machinery:** seven FastArray serializers — `FSigilItemStackContainer`, `FSigilCollectionContainer`, `FSigilMixinContainer`, `FSigilEquipmentContainer`, `FSigilCurrencyContainer`, `FSigilGameplayTagFloatContainer`, `FSigilGameplayTagIntegerContainer` — plus registered-subobject replication for `USigilItemInstance` / collection objects, and Push Model dirty-marking (`MARK_PROPERTY_DIRTY_FROM_NAME`). The build script calls `SetupIrisSupport`, so Iris builds are supported.
- **Owner requirement:** the owning actor must have `bReplicateUsingRegisteredSubObjectList` enabled (Error-logged otherwise; see Prerequisites).
- **Local/server-only:** `OnInventoryAddItemInfo`, `OnInventoryAddItemInfo_Rejected` and `OnInventoryRemoveItemInfo` fire on the server only; `OnInventoryStackUpdate` fires everywhere. Editor-time schema validation is not replicated (editor-only).

## Known gaps and design boundaries

Verified in source; plan around these:

- **Zero GAS coupling — by design.** Nothing in the module references the Ability System. Equipping an item does **not** grant abilities or attribute bonuses. The intended integration points are `ISigilEquipmentInterface::OnEquipmentBeginPlay` / `OnEquipmentEndPlay` (legacy `OnEquipped`/`OnUnequipped` are deprecated) and overriding `USigilEquipmentSystemComponent::CreateEquipmentInstance` — wire your ASC calls there.
- **No UI layer.** Messages and delegates are UI-ready (stack updates, rejections, collection add/remove), but no widgets ship.
- **No client prediction** for inventory operations; latency-sensitive flows (rapid slot swapping) will feel server round-trip latency.
- **Slot tags are project-owned** (`Sigil.Inventory.Slots.*` has no native entries), as are any collection tags beyond `Main` / `Equipped` / `Hidden` / `QuickBar`.
- `OnEquipped` / `OnUnequipped` remain for backward compatibility but are marked `DeprecatedFunction` — new code should use `OnEquipmentBeginPlay` / `OnEquipmentEndPlay`.

## See Also

- [sigil.movement](sigil-movement.md) — push weapon movement definitions from equipment callbacks.
- [sigil.input](sigil-input.md) — tag-driven input for use-item / quick-bar actions.
