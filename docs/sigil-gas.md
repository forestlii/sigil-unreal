[English](sigil-gas.md) | [简体中文](sigil-gas.zh-CN.md)

# sigil.gas

**Plugin:** `SigilGas` · **Modules:** `SigilGas` (Runtime), `SigilGasEditor` (Editor) · **Depends on:** Gameplay Abilities, Enhanced Input, Targeting System, Modular Gameplay (engine plugins)

sigil.gas is the Gameplay Ability System (GAS) infrastructure layer of the Sigil suite. It extends the engine's `UAbilitySystemComponent` and `UGameplayAbility` with an explicit initialization lifecycle, data-driven ability sets, three-state activation groups, tag relationship mappings, a game phase subsystem, a global ability system, reusable ability/async tasks, reusable target actors, and common attribute sets (Health, Stamina, Mana) with a component-based change-notification pipeline.

## Overview

### Extended Ability System Component

`USigilAbilitySystemComponent` extends `UAbilitySystemComponent` with:

- **Explicit lifecycle** — call `InitializeAbilitySystem(OwnerActor, AvatarActor)` / `UninitializeAbilitySystem()`. Initialization grants every asset in `DefaultAbilitySets`, initializes attributes from the configured `AttributeSetInitializeGroupName` / `AttributeSetInitializeLevel`, registers with `USigilGlobalAbilitySystem`, and broadcasts `OnAbilitySystemInitialized` / `OnAbilitySystemUninitialized`.
- **Activation groups** — `IsActivationGroupBlocked`, `AddAbilityToActivationGroup`, `CanChangeActivationGroup`, `ChangeActivationGroup`, `CancelActivationGroupAbilities` (see below).
- **Tag relationship mapping** — a `USigilAbilityTagRelationshipMapping` asset (settable at runtime via `SetTagRelationshipMapping`) that augments block/cancel/required/blocked tags during ability activation.
- **Activation events** — Blueprint-assignable `OnAbilityActivated`, `OnAbilityActivationFailed` (failure reasons are relayed to the owning client via an unreliable client RPC), and `AbilityEndedEvent`.
- **Batched RPC activation** — `ShouldDoServerAbilityRPCBatch()` returns true; `BatchRPCTryActivateAbility` can compress activate + target data + end into fewer RPCs.
- **Replicated gameplay events** — `SendGameplayEventToActor_Replicated` routes a gameplay event through a server RPC and multicast so all clients receive it.
- **Replication mode** — exposed as the `AbilitySystemReplicationMode` property (`EGameplayEffectReplicationMode`).

### Ability Sets

`USigilAbilitySet` is a const `UPrimaryDataAsset` bundling `GrantedGameplayAbilities`, `GrantedGameplayEffects`, and `GrantedAttributes`. Grant it with `GiveToAbilitySystem` (or the static, authority-only `GiveAbilitySetToAbilitySystem`) and revoke everything later through the returned `FSigilAbilitySet_GrantedHandles` (`TakeFromAbilitySystem`). Each ability entry carries `Ability`, `AbilityLevel`, `InputID`, and `DynamicTags`.

### Experimental ability-entitlement projection

> **Spike API, not a Final L2 or production commitment.** This surface exists to validate a server-authoritative projection model. Projects must not treat it as a durable save schema or a stable integration contract until a separate Final L2 decision accepts the evidence.

`USigilAbilitySystemComponent::ReconcileAbilityEntitlements` projects a session-local `FSigilAbilityEntitlementSnapshot` into runtime ability specs. Each stable `EntitlementTag` maps to one already-loaded `USigilAbilitySet`. The experimental path:

- accepts **ability-only** sets; effects and attribute sets are rejected;
- never calls `LoadSynchronous` and fails closed when a referenced set or ability class is not already loaded;
- performs full preflight before mutation, including unique entitlement tags, concrete ability classes, levels, dynamic tags, and incompatible identities;
- canonicalizes grant identities and the desired snapshot so a higher revision applies, the same revision and digest is a no-op, the same revision with a different digest conflicts, and a lower revision is stale;
- shares one runtime grant among multiple entitlement contributors and removes it only after the final contributor disappears;
- compensates newly created `FGameplayAbilitySpec` handles when a grant in the same reconciliation fails, while leaving the previously accepted projection intact;
- owns only its recorded handles and activation-gate contribution. `ResetAbilityEntitlementProjection` does not remove default sets or grants owned by another system;
- routes `AbilityInputTagPressed` / `AbilityInputTagReleased` by an **exact** DynamicSpecSourceTag match; and
- exposes idempotent `(GateTag, SourceId)` activation-gate contributions through `SetAbilityActivationGateSource`. Every exact gate tag passed to this API is dedicated to it: the entire explicit count, including contributions that could otherwise come from effects, abilities, or direct loose-tag calls, must be owned by this API. All producers use distinct `SourceId` values and no other system grants or removes the same exact tag.

The snapshot revision is local to one ASC projection epoch. Persist stable entitlement tags in the game-owned durable snapshot, then construct a fresh desired snapshot for a replacement ASC; never persist ability spec handles, ASC pointers, UObject pointers, contributor sets, or the session revision.

The compensation guarantee is intentionally narrow: preflight failures and failures while adding a new identity remove the handles created by that attempt and preserve the previously accepted projection. GAS can synchronously invoke ability `OnGive` / `OnRemove` hooks. If one of those hooks changes the ASC context or an existing projection-owned spec, reconciliation fails closed with `RuntimeStateMismatch`, retains ownership bookkeeping, and blocks further reconciliation until an authoritative `ResetAbilityEntitlementProjection`. This Spike does **not** claim that an already-fired lifecycle hook, arbitrary Blueprint event, or external side effect can be transactionally reversed.

### Sigil Gameplay Ability

`USigilGameplayAbility` (abstract) is the base ability class:

- **`ActivationGroup`** (`ESigilAbilityActivationGroup`): `Independent`, `Exclusive_Replaceable`, or `Exclusive_Blocking`. Only one exclusive ability runs at a time; a blocking exclusive prevents other exclusives from starting, a replaceable one is canceled when another exclusive activates.
- **Additional costs** — instanced `USigilAbilityCost` objects in `AdditionalCosts` (e.g. ammo/charges), checked in `CheckCost` and applied in `ApplyCost`; each cost can opt into `bOnlyApplyCostOnHit`.
- **Effect containers** — `EffectContainerMap` maps a tag to a `FSigilGameplayEffectContainer` (a `UTargetingPreset` plus gameplay effect classes). Use `MakeEffectContainerSpec` / `ApplyEffectContainer` at runtime.
- **Loose tags while active** — `ActivationOwnedLooseTags` (`FSigilGameplayTagCount` entries) are applied to the owner during activation.
- **Optional tick** — set `bEnableTick` to receive the `AbilityTick` event while active (instanced-per-actor abilities).
- **Trait tags** — granting an ability tagged `Sigil.Ability.Trait.ActivationOnSpawn` activates it immediately (`TryActivateAbilityOnSpawn`); `Sigil.Ability.Trait.Persistent` marks abilities that should persist during gameplay.
- **Blueprint hooks** — `K2_OnGiveAbility`, `K2_OnRemoveAbility`, `K2_OnAvatarSet`, `K2_OnInputPressed`, `K2_OnInputReleased`, `K2_OnCheckCost`, `K2_OnApplyCost`, `K2_ShouldActivateAbility`, `OnActivationFailed`, and `SendTargetDataToServer` for client-predicted target data.

### Tag Relationship Mapping

`USigilAbilityTagRelationshipMapping` is a data asset of `FSigilAbilityTagRelationship` rows: for one `AbilityTag`, declare `AbilityTagsToBlock`, `AbilityTagsToCancel`, `ActivationRequiredTags`, and `ActivationBlockedTags`. A `Layered` list of `FSigilAbilityTagRelationshipsWithQuery` applies additional relationship sets only while the actor's tags satisfy an `ActorTagQuery`. This centralizes cross-ability interactions instead of hardcoding tags on each ability asset.

### Game Phases

`USigilGamePhaseSubsystem` (world subsystem, server authority) manages nested game phases expressed as gameplay tags. Parent and child phases coexist; siblings do not — starting `GamePhase.Playing.SuddenDeath` ends `GamePhase.Playing.NormalPlay` but leaves `GamePhase.Playing` active. Start a phase with `StartPhase` (or the Blueprint node **Start Phase**) using a `USigilGamePhaseAbility` subclass whose `GamePhaseTag` defines the phase, and observe phases with `WhenPhaseStartsOrIsActive` / `WhenPhaseEnds` (`ExactMatch` or `PartialMatch`) or query with `IsPhaseActive`.

### Global Ability System

`USigilGlobalAbilitySystem` (world subsystem) tracks every registered `USigilAbilitySystemComponent` (registration happens automatically during ASC initialization) and can `ApplyAbilityToAll` / `ApplyEffectToAll` / `RemoveAbilityFromAll` / `RemoveEffectFromAll` — including retroactively granting to ASCs that register later.

### Attributes

- **Attribute sets** — `USigilHealthSet` (`Health`, `MaxHealth`, plus non-replicated meta attributes `IncomingHealing`, `IncomingDamage`), `USigilStaminaSet` (`Stamina`, `MaxStamina`, `IncomingHealing`, `IncomingDamage`), and `USigilManaSet` (`Mana`, `MaxMana`). Current values are clamped to their max and adjust proportionally when the max changes. Every attribute registers a gameplay tag under `Sigil.Attribute.<Set>.<Attribute>` (e.g. `Sigil.Attribute.HealthSet.Health`), usable through the tag↔attribute helpers.

  > **Implemented by your project:** `IncomingDamage` and `IncomingHealing` are *meta attributes*. The plugin declares them and clamps/broadcasts the backing values, but it does **not** convert them into `-Health`/`+Health`. Your project must provide the GameplayEffect and/or `UGameplayEffectExecutionCalculation` that writes to and consumes these attributes (sigil.combat writes `IncomingDamage` from its combat flow, and the final damage application is still project-side).

- **Change notifications** — `USigilAttributeSystemComponent` on the same actor receives forwarded callbacks from the Sigil attribute sets: `OnPostAttributeChange`, `OnAttributeChanged` (fires on server and clients), and `OnPostGameplayEffectExecute`, each with a Blueprint-native `Handle...` counterpart for subclasses. Use it to drive UI/reactions without subclassing attribute sets.
- **Tag ↔ attribute registry** — `USigilGameplayAttributesHelper` maintains a global tag-to-attribute registry (`RegisterTagToAttribute`, `TagToAttribute`, `AttributeToTag`, `SetFloatAttribute`, percentage getters, etc.).
- **Data-driven defaults** — `USigilAbilitySystemGlobals` extends `UAbilitySystemGlobals` with `InitAttributeSetDefaults` / `ApplyAttributeDefault` keyed by `FSigilAttributeGroupName` (`MainName` + optional `SubName`). Curve table rows follow the engine's `Group.SetName.Attribute` layout; a sub-group is encoded in the group segment as `Main->Sub` (for example `Hero->Warrior.SigilHealthSet.MaxHealth`) because the engine splits rows on `.`. It also exposes a global pre-effect-apply event via `ISigilAbilitySystemGlobalsEventReceiver`. Attribute group initialization only works when your project's `AbilitySystemGlobals` class is `USigilAbilitySystemGlobals` or a subclass (see Configuration); otherwise a warning is logged.
- The `SigilGasEditor` module supplies a property customization for `FSigilAttributeGroupName` in the editor.

### Ready-made ASC host actors

| Actor | ASC location |
| --- | --- |
| `ASigilCharacter` | No ASC of its own; implements `IAbilitySystemInterface` + `IGameplayTagAssetInterface` and defers to `CustomGetAbilitySystemComponent` (Blueprint) — use when the ASC lives on the PlayerState. |
| `ASigilCharacterWithAbilities` | Owns a `USigilAbilitySystemComponent` subobject directly. |
| `ASigilPlayerState` | Owns a `USigilAbilitySystemComponent`; minimal PlayerState intended for game-feature extension. |
| `ASigilGameStateBase` / `ASigilGameState` | Own a game-wide `USigilAbilitySystemComponent` (primarily for gameplay cues). |

### Tasks and target actors

**Ability tasks** (usable inside abilities):

- `USigilAbilityTask_PlayMontageAndWaitForEvent` (`PlayMontageAndWaitForEvent` / `...Ext`) — play a montage and receive gameplay events during it.
- `USigilAbilityTask_WaitTargetDataUsingActor` (`WaitTargetDataWithReusableActor`) — target data confirmation using an already-spawned, reusable target actor.
- `USigilAbilityTask_ServerWaitForClientTargetData` — server-side wait for client-predicted target data.
- `USigilAbilityTask_WaitInputPressWithTags` — wait for input press, gated by required/ignored tag containers (also respects `Sigil.State.Interacting` vs `Sigil.State.InteractingRemoval` counts).
- `USigilAbilityTask_WaitGameplayEvents` — listen to multiple event tags at once.
- `USigilAbilityTask_WaitDelayOneFrame` — resume next frame.

**Async tasks** (usable from any Blueprint):

- `USigilAsyncTask_AttributeChanged` (`ListenForAttributeChange` / `ListenForAttributesChange`)
- `USigilAsyncTask_GameplayTagAddedRemoved` (`ListenForGameplayTagAddedOrRemoved`)
- `USigilAsyncTask_WaitGameplayAbilityActivated` (`WaitGameplayAbilityActivated`)
- `USigilAsyncTask_WaitGameplayAbilityEnded` (`WaitGameplayAbilityEnded` / `WaitAbilitySpecHandleEnded`)

**Target actors** — `ASigilAbilityTargetActor_Trace` (abstract-style base with `MaxRange`, `TraceProfile`, spread/aiming support, `NumberOfTraces`, `MaxHitResultsPerTrace`) and its shape subclasses `ASigilAbilityTargetActor_LineTrace` and `ASigilAbilityTargetActor_SphereTrace`, designed to pair with `WaitTargetDataWithReusableActor`.

### Native gameplay tags

| Tag | Meaning |
| --- | --- |
| `Sigil.Ability.ActivateFail.Cooldown` | Activation failed: on cooldown. |
| `Sigil.Ability.ActivateFail.Cost` | Activation failed: cost check. |
| `Sigil.Ability.ActivateFail.TagsBlocked` | Activation failed: blocked by tags. |
| `Sigil.Ability.ActivateFail.TagsMissing` | Activation failed: required tags missing. |
| `Sigil.Ability.ActivateFail.Networking` | Activation failed: network checks. |
| `Sigil.Ability.ActivateFail.ActivationGroup` | Activation failed: activation group blocked. |
| `Sigil.Ability.Trait.ActivationOnSpawn` | Ability activates right after being granted. |
| `Sigil.Ability.Trait.Persistent` | Ability should persist during gameplay. |
| `Sigil.State.Interacting` | Owner is interacting. |
| `Sigil.State.InteractingRemoval` | Pending removals; compared against `Sigil.State.Interacting` counts. |

## Prerequisites

- Engine plugins **Gameplay Abilities**, **Enhanced Input**, **Targeting System**, and **Modular Gameplay** enabled (declared by `SigilGas.uplugin`).
- For attribute-group initialization, your project must use `USigilAbilitySystemGlobals` (or a subclass such as sigil.combat's) as the ability system globals class — see Configuration.
- Gameplay tags for your abilities/phases; the plugin's own native tags (table above) register automatically.

## Quick Start

1. **Pick an ASC host.** Derive from `ASigilCharacterWithAbilities` (ASC on the pawn), or `ASigilPlayerState` + `ASigilCharacter` (ASC on the player state), or add a `USigilAbilitySystemComponent` to your own actor.

2. **Create ability set assets.** Add a `USigilAbilitySet` and fill in abilities (subclasses of `USigilGameplayAbility`), startup effects, and attribute sets (e.g. `USigilHealthSet`). Assign it to the ASC's `DefaultAbilitySets`.

3. **Initialize the ASC.** When possession/replication is settled (e.g. `PossessedBy` and `OnRep_PlayerState`), call:

   ```cpp
   AbilitySystemComponent->InitializeAbilitySystem(/*Owner*/ PlayerState, /*Avatar*/ Pawn);
   ```

4. **Configure activation rules.** Set each ability's `ActivationGroup`, and author a `USigilAbilityTagRelationshipMapping` asset assigned to the ASC's `TagRelationshipMapping` for cross-ability block/cancel rules.

5. **Optional — attribute defaults from curve tables.** Set the globals class (see Configuration), register attribute default curve tables in `UAbilitySystemGlobals`, set `AttributeSetInitializeGroupName` / `AttributeSetInitializeLevel` on the ASC, and the values are applied during initialization.

6. **Optional — react to attributes.** Add a `USigilAttributeSystemComponent` to the avatar actor and bind `OnAttributeChanged` (fires on server and clients) for UI and gameplay reactions.

7. **Optional — game phases.** Create `USigilGamePhaseAbility` subclasses with `GamePhaseTag` set (e.g. `GamePhase.Playing`), then on the server call **Start Phase** and subscribe with **When Phase Starts or Is Active**.

## Key Types

| Type | Description |
| --- | --- |
| `USigilAbilitySystemComponent` | Extended ASC: lifecycle, ability sets, activation groups, tag relationships, batched RPCs, replicated gameplay events. |
| `USigilGameplayAbility` | Base ability: activation group, additional costs, effect containers, optional tick, Blueprint hooks. |
| `USigilAbilitySet` | Data asset granting abilities/effects/attribute sets; revocable via `FSigilAbilitySet_GrantedHandles`. |
| `FSigilAbilityEntitlementSnapshot` | Experimental, session-local desired ability projection; maps stable entitlement tags to already-loaded, ability-only sets. |
| `FSigilAbilityReconcileResult` | Experimental reconciliation status, accepted revision, canonical digest, and diagnostic error. |
| `USigilAbilityCost` | Instanced, Blueprintable extra activation cost (ammo, charges…). |
| `ESigilAbilityActivationGroup` | `Independent` / `Exclusive_Replaceable` / `Exclusive_Blocking`. |
| `USigilAbilityTagRelationshipMapping` | Data asset of block/cancel/required/blocked tag relationships, plus tag-query-layered rules. |
| `USigilGamePhaseSubsystem` | World subsystem for nested, tag-based game phases (authority only). |
| `USigilGamePhaseAbility` | Base ability that represents one game phase (`GamePhaseTag`). |
| `USigilGlobalAbilitySystem` | World subsystem applying abilities/effects to all registered ASCs. |
| `USigilHealthSet` / `USigilStaminaSet` / `USigilManaSet` | Common attribute sets with `Sigil.Attribute.*` tags; Health/Stamina include `IncomingDamage`/`IncomingHealing` meta attributes (consumed by your project). |
| `USigilAttributeSystemComponent` | Actor component receiving attribute change / effect-execute callbacks on server and clients. |
| `USigilAbilitySystemGlobals` | Extended `UAbilitySystemGlobals`: attribute defaults by group name, global pre-apply event receivers. |
| `USigilGameplayAttributesHelper` | Tag↔attribute registry and attribute utility function library. |
| `ASigilAbilityTargetActor_Trace` (+`_LineTrace`, `_SphereTrace`) | Reusable, configurable trace target actors for `WaitTargetDataWithReusableActor`. |
| `USigilAnimNotify_SendGameplayEvent` | Anim notify that sends a gameplay event (`EventTag`) to the owner. |

## Configuration

- **Ability system globals class** (required for attribute group initialization). In `DefaultGame.ini`:

  ```ini
  [/Script/GameplayAbilities.AbilitySystemGlobals]
  AbilitySystemGlobalsClassName=/Script/SigilGas.SigilAbilitySystemGlobals
  ```

  If you also use sigil.combat, set the class to sigil.combat's subclass instead (see that page). Without a Sigil globals class, `InitializeAttributes` logs a warning and does nothing.

- **Per-ASC defaults** — `DefaultAbilitySets`, `AttributeSetInitializeGroupName` (`MainName`/`SubName`), `AttributeSetInitializeLevel`, `TagRelationshipMapping`, `AbilitySystemReplicationMode`.
- **Data validation** — `USigilAbilitySet`, `USigilGameplayAbility`, and `USigilGamePhaseAbility` implement editor-time validation/presave passes; ability set entries can be toggled with editor-only enable flags for debugging.
- There are no `UDeveloperSettings` in this plugin; everything is asset- or ini-driven.

## Networking

- **Server-authoritative GAS flow** — ability granting (`GiveAbilitySetToAbilitySystem` is `BlueprintAuthorityOnly`), game phases, and the global ability system all run on the server; attributes replicate through standard GAS attribute replication (`Health`, `MaxHealth`, `Stamina`, `MaxStamina`, `Mana`, `MaxMana` are replicated; `IncomingDamage` / `IncomingHealing` are not replicated, by design of meta attributes).
- **Client feedback** — activation failure reasons reach the owning client via an unreliable client RPC and surface through `OnAbilityActivationFailed` / `OnActivationFailed`.
- **Replicated gameplay events** — `SendGameplayEventToActor_Replicated` uses a reliable server RPC plus multicast.
- **Prediction support** — batched ability RPCs (`BatchRPCTryActivateAbility`), `SendTargetDataToServer` on the ability, and `USigilAbilityTask_ServerWaitForClientTargetData` for client-predicted targeting.
- **Known gaps** — `USigilAttributeSystemComponent` callbacks depend on where attribute-set callbacks run: `OnAttributeChanged` fires on both server and clients, while `OnPostGameplayEffectExecute` only fires where effects execute (the server). The game phase subsystem exposes `BlueprintAuthorityOnly` APIs and has no built-in client-side phase replication.

## See Also

- [sigil.input](sigil-input.md) — tag-driven input; pair its input events with ability activation.
- [sigil.combat](sigil-combat.md) — combat layer built directly on this plugin.
