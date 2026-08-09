[English](sigil-interaction.md) | [简体中文](sigil-interaction.zh-CN.md)

# sigil.interaction

**Plugin:** `SigilInteraction` · **Modules:** `SigilInteraction` (Runtime) · **Depends on:** SmartObjects, GameplayBehaviors, GameplayBehaviorSmartObjects, GameplayAbilities, TargetingSystem, ModularGameplay (engine plugins)

sigil.interaction is a SmartObject-driven interaction system bridged with Gameplay Behaviors and the Gameplay Ability System. Instead of writing per-actor interaction code, you author interaction *entrances* as data inside SmartObject slot definitions, and a per-player component manages the candidate list, the selected target, and the replicated interaction options. Actual execution is delegated to GAS through one of two paths that your project wires together in Blueprint.

## Overview

### The entrance is data on a SmartObject slot

An actor is interactable if — and only if — one of its SmartObject slots carries a `FSigilSmartObjectInteractionEntranceData` entry (shown as **Interaction Entrance** in the editor) in its slot definition data. That struct holds a single soft reference, `DefinitionDA`, pointing to a `USigilInteractionDefinition` data asset with the static presentation data: `Text`, `SubText`, and `TriggeringInputAction` (a `FDataTableRowHandle` whose row type is locked to CommonUI's `CommonInputActionDataBase`).

`USigilSmartObjectFunctionLibrary::FindSmartObjectsWithInteractionEntranceInActor` is the query primitive that scans an actor's SmartObject slots for such entrances.

### The component manages candidates, selection, and options

`USigilInteractionSystemComponent` (an `ActorComponent`) is the per-pawn brain:

- **Candidates** — `InteractableActors` is the array of potential targets. The plugin does **not** fill it; see Prerequisites. `SetInteractableActors` (authority only) replaces the array, and only the *count* (`NumsOfInteractableActors`) is replicated to the owning client.
- **Selection** — `InteractableActor` is the currently selected target (replicated, owner only). `CycleInteractableActors(bNext)` is a reliable Server RPC to step through candidates; `SetInteractableActor` sets it directly on the authority. With `bNewActorHasPriority` enabled, the first candidate is always auto-selected when the list changes.
- **Options** — on the authority, `RefreshOptionsForActor` resolves the selected actor's slots into `FSigilInteractionOption` entries (`Definition`, `SlotIndex`, `SlotState`, plus non-replicated `RequestResult` and `BehaviorDefinition`) and replicates the `InteractionOptions` array to the owner. The search is filtered through `GetSmartObjectRequestFilter` (a `BlueprintNativeEvent` returning `DefaultRequestFilter` by default).
- **Session state** — `StartInteraction(Index)`, `EndInteraction`, and `InstantInteraction(Index)` (all `BlueprintAuthorityOnly`) drive `InteractingOption` / `bInteracting`; query with `IsInteracting` and `GetInteractingOption`.
- **Events** — Blueprint-assignable delegates: `OnInteractableActorChangedEvent`, `OnInteractableActorNumChangedEvent`, `OnInteractingStateChangedEvent`, `OnInteractionOptionsChangedEvent`, `OnSearchInteractableActorsEvent`.

Note that `SearchInteractableActors` does not itself search anything — it only broadcasts `OnSearchInteractableActorsEvent` so that whatever system owns candidate discovery can respond.

### Two execution paths, glued together by your project

The plugin ships the building blocks for two GAS execution paths, but the glue between "player pressed interact" and "ability runs" is intentionally left to project Blueprints:

1. **Generic interaction ability (user side).** Subclass `USigilGameplayAbility_Interaction`. In its Blueprint graph, call `TryClaimInteraction(Index, out ClaimedHandle)` to claim the SmartObject slot for the currently selected option, then run the ability task `USigilAbilityTask_UseSmartObjectWithGameplayBehavior::UseSmartObjectWithGameplayBehavior(ClaimHandle, ClaimPriority)`, which triggers the slot's Gameplay Behavior and reports `OnSucceeded` / `OnFailed`.
2. **Ability granted by the SmartObject (object side).** Configure the slot's behavior definition with `USigilGameplayBehaviorConfig_InteractionWithAbility` (properties: `AbilityToGrant`, `AbilityLevel`) paired with `USigilGameplayBehavior_InteractionWithAbility`. When the behavior triggers, the ability class is granted to the interacting avatar's Ability System Component and activated; when it ends, the behavior ends. The granted ability must be an instanced, non-LocalOnly ability, and event-triggered abilities are not supported (enforced by `IsDataValid`).

### Supporting pieces

- `ISigilInteractableInterface` — optional interface on interactable actors for cosmetic callbacks: `GetInteractionDisplayName`, `OnInteractionSelected` / `OnInteractionDeselected`, `OnInteractionStarted` / `OnInteractionEnded`, `OnInteractionOptionSelected`.
- `USigilTargetingFilterTask_InteractionSmartObjects` — a Targeting System filter task (display name *(GGS)FilterTask:InteractionSmartObject*) that discards targets without interaction entrances; drop it into a Targeting Preset used for candidate discovery.
- `USigilSmartObjectFunctionLibrary` — Blueprint helpers: `GetGameplayBehaviorConfig`, `FindGameplayBehaviorConfig`, `FindSmartObjectsWithInteractionEntranceInActor`, `FindInteractionDefinitionFromSmartObjectSlot`.
- `USigilSocketRelationshipMapping` — a standalone data asset for per-skeleton socket transform adjustments (`FSigilSocketRelationship` / `FSigilSocketAdjustment`), useful when attaching props during interactions. Not referenced by the rest of the system.

## Prerequisites

This plugin ships mechanisms, not a turnkey feature. Before anything works you need:

- [ ] **Engine plugins enabled** — SmartObjects, GameplayBehaviors, GameplayBehaviorSmartObjects, GameplayAbilities, TargetingSystem, ModularGameplay (declared by the `.uplugin`, enabled automatically when this plugin is enabled).
- [ ] **A candidate discovery system.** The component never scans the world by itself. You must feed `SetInteractableActors` on the authority — typically from a Targeting System preset (optionally using `USigilTargetingFilterTask_InteractionSmartObjects`), an overlap volume, or any Blueprint logic, ideally reacting to `OnSearchInteractableActorsEvent`.
- [ ] **A CommonUI input-action DataTable.** `USigilInteractionDefinition::TriggeringInputAction` expects a row of type `CommonInputActionDataBase`, so your project needs CommonUI set up with such a DataTable if you want per-option input prompts.
- [ ] **A GAS setup.** Both execution paths assume the interacting pawn has an `AbilitySystemComponent`. The project must create the Blueprint subclass of `USigilGameplayAbility_Interaction` (path 1) and/or the interaction abilities granted by SmartObjects (path 2), and grant/trigger them itself.
- [ ] **Push Model replication enabled** (`net.IsPushModelEnabled=1`) — the component registers all replicated properties as push-based; without push model support the owner-side state will not update.

## Quick Start

1. **Author the interaction definition.** Create a `USigilInteractionDefinition` data asset; fill `Text`, `SubText`, and optionally `TriggeringInputAction`.
2. **Mark a SmartObject as interactable.** In the SmartObject Definition asset of the target actor, add a **Interaction Entrance** (`FSigilSmartObjectInteractionEntranceData`) to a slot's definition data and point `DefinitionDA` at your definition asset. Give the slot a behavior definition (for path 2, use one containing `USigilGameplayBehaviorConfig_InteractionWithAbility`).
3. **Add the component.** Add `USigilInteractionSystemComponent` to your pawn (or another replicated actor owned by the player). Configure `DefaultRequestFilter` and `bNewActorHasPriority` as needed.
4. **Feed candidates.** On the server, run your discovery logic (Targeting System preset, overlap, etc.) and call `SetInteractableActors`. Optionally call `SearchInteractableActors` from gameplay code and bind the discovery logic to `OnSearchInteractableActorsEvent`.
5. **Build the interact ability.** Create a Blueprint subclass of `USigilGameplayAbility_Interaction`: on activation, call `TryClaimInteraction` with the desired option index, then `UseSmartObjectWithGameplayBehavior` with the claimed handle. Grant this ability to your pawn and activate it from your input layer.
6. **Show UI.** On the owning client, bind the component's change delegates and read `GetInteractionOptions` / `GetNumOfInteractableActors` to display prompts.

## Key Types

| Type | Description |
| --- | --- |
| `USigilInteractionSystemComponent` | Per-player component managing candidates, selection, replicated options, and interaction session state. Authority-driven. |
| `FSigilSmartObjectInteractionEntranceData` | SmartObject slot definition data ("Interaction Entrance"); its presence is the sole marker of interactability. Holds `DefinitionDA`. |
| `USigilInteractionDefinition` | Data asset with static presentation data: `Text`, `SubText`, `TriggeringInputAction` (CommonUI input action row). |
| `FSigilInteractionOption` | A resolved interaction option: `Definition`, `SlotIndex`, `SlotState`, plus non-replicated `RequestResult` and `BehaviorDefinition`. |
| `USigilGameplayAbility_Interaction` | Base gameplay ability for the user-side path; exposes `TryClaimInteraction` and tracks the owner's `InteractionSystem`. |
| `USigilAbilityTask_UseSmartObjectWithGameplayBehavior` | Ability task that uses a claimed SmartObject slot through its Gameplay Behavior; `OnSucceeded` / `OnFailed`. |
| `USigilGameplayBehavior_InteractionWithAbility` | Gameplay Behavior that grants + activates an ability on the interacting avatar and ends when the ability ends. |
| `USigilGameplayBehaviorConfig_InteractionWithAbility` | Config for the above: `AbilityToGrant` (instanced, non-LocalOnly, no event triggers), `AbilityLevel`. |
| `ISigilInteractableInterface` | Optional cosmetic callbacks on the interactable actor (selection, start/end, option selected, display name). |
| `USigilTargetingFilterTask_InteractionSmartObjects` | Targeting System filter task keeping only actors with interaction entrances. |
| `USigilSmartObjectFunctionLibrary` | Blueprint helpers for querying behavior configs, entrances, and definitions from slots. |
| `USigilSocketRelationshipMapping` | Standalone data asset mapping mesh assets to per-skeleton socket transform adjustments. |

## Configuration

There are no project settings. Configuration lives in assets and on the component:

- **Component** — `DefaultRequestFilter` (`FSmartObjectRequestFilter`, used by the default `GetSmartObjectRequestFilter`), `bNewActorHasPriority`.
- **SmartObject definitions** — where entrances, slots, and behavior configs are authored.
- **Interaction definitions** — one `USigilInteractionDefinition` asset per distinct interaction verb.
- Both behavior config and the interaction ability implement editor data validation (`IsDataValid`), so misconfigured abilities surface in the editor.

## Networking

The component follows an authority-computes, owner-consumes model, using push-model replication with `COND_OwnerOnly` for all replicated properties:

| State | Replication |
| --- | --- |
| `InteractableActors` (full candidate array) | **Not replicated.** Server only. |
| `NumsOfInteractableActors` | Replicated to owner only. |
| `InteractableActor` (selection) | Replicated to owner only. |
| `InteractionOptions` | Replicated to owner only (`RequestResult` and `BehaviorDefinition` fields are `NotReplicated`). |
| `InteractingOption` | Replicated to owner only. |
| `CycleInteractableActors` | Reliable Server RPC (client may call). |
| `SetInteractableActors` / `SetInteractableActor` / `StartInteraction` / `EndInteraction` / `InstantInteraction` / `SearchInteractableActors` | Authority only; not RPCs. Your project must route client input to the server (typically via the GAS ability). |

Ability execution itself replicates through GAS and the SmartObject subsystem as usual.

## See Also

- [sigil.input](sigil-input.md) — tag-driven input layer that can trigger the interact ability.
- [sigil.ui](sigil-ui.md) — UI layer for rendering interaction prompts and option lists.
- [sigil.effects](sigil-effects.md) — context-driven feedback for interaction animations.
