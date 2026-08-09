[English](sigil-combat.md) | [简体中文](sigil-combat.zh-CN.md)

# sigil.combat

**Plugin:** `SigilCombat` · **Modules:** `SigilCombat` (Runtime) · **Depends on:** [sigil.gas](sigil-gas.md), Gameplay Abilities, Modular Gameplay, Targeting System, Motion Warping, Niagara

sigil.combat is a GAS-based multiplayer combat framework built directly on sigil.gas. It provides a replicated hit-reaction pipeline (Combat Flow), data-table-driven attack and bullet definitions, pooled collision trace instances for melee sweeps, a pooled projectile (bullet) subsystem with penetration and multi-shot patterns, lock-on targeting built on the engine Targeting System, predictable montage playback, team affiliation, weapon actors, combat attribute sets (Combat, Poise), and a custom gameplay effect context that carries the attack definition across the network.

> **This plugin is a framework, not a finished combat game.** Several deliberate extension points must be implemented by your project — they are marked "Implemented by your project" throughout this page.

## Overview

### Combat Flow: the hit-reaction pipeline

`USigilCombatSystemComponent` is the per-character combat hub. It instantiates a **`USigilCombatFlow`** from its `CombatFlowClass` property; the flow instance is replicated to clients (`OnRep_CombatFlow`).

- `USigilCombatFlow` is **Abstract** — *implemented by your project*: you must create a (typically Blueprint) subclass per character archetype (human, quadruped, mechanical…) and assign it to `CombatFlowClass`, otherwise the component has no flow to run.
- The flow participates in the global effect pipeline as a `ISigilAbilitySystemGlobalsEventReceiver`: `HandlePreGameplayEffectSpecApply` lets it append dynamic tags to incoming effect specs (this requires the Sigil globals class — see Configuration), and `HandleGameplayEffectExecute` reacts to executed effects on the defender.
- When an attack lands, `RegisterAttackResult` adds a `FSigilAttackResult` to the replicated **`FSigilAttackResultContainer`** (a `FFastArraySerializer`). On both server and clients (via `PostReplicatedAdd`), the flow's `HandleAttackResult` runs; the default implementation forwards the result to the instanced `USigilAttackResultProcessor` list (`AttackResultProcessors`) — small, reusable reaction steps (hit stun, knockback, cue playback…) you author per flow.
- `FSigilAttackResult` carries an `ImpactResult` tag, `TaggedValues`, an `EffectContextHandle`, and aggregated source/target tag containers.

> **Implemented by your project:** the actual damage numbers. sigil.combat writes attack data (tags, SetByCaller magnitudes, effect context) into gameplay effect specs, but the `GameplayEffect` / `UGameplayEffectExecutionCalculation` that consumes `CombatSet` attributes and the `IncomingDamage` meta attribute from sigil.gas — and finally subtracts Health — is project-side.

### Attack definitions and requests

- **`FSigilAttackDefinition`** (`FTableRowBase`) — author one DataTable row per attack: `AttackTags` (added as dynamic asset tags to the effect spec), `SetByCallerMagnitudes` (tag→float), `TargetEffectClass`/`TargetEffectClassLevel`, `TargetEffectContainer`, `TargetGameplayCues`, hit-reaction values (`KnockbackDistance`, `KnockbackMultiplier`), feedback values (`HitStallingDuration`, `HitPlayRateFactor`), and an extensible `UserSettings` map of `FInstancedStruct` (base struct `SigilUserSetting`).
- **`USigilAttackRequest_Base`** → `USigilAttackRequest_Melee` (with `TracesToControl` tags and an `AttackDefinitionHandle` row handle) and `USigilAttackRequest_Bullet` (with an `ESigilAbilityTargetingSourceType` aiming source: camera/pawn/weapon/custom). Requests are instanced objects embedded in anim notify states.
- **Anim notify states** — `USigilANS_AttackTrace` (melee) and `USigilANS_BulletTrace` (ranged) each hold an instanced `AttackRequest`. *Implemented by your project*: both classes are `HideDropdown` containers with **no C++ notify logic** — subclass them in Blueprint and implement the NotifyBegin/End behavior (enabling the matching traces, spawning bullets) yourself. `USigilANS_MovementCancellation` (disables montage root motion while the character moves) is fully implemented in C++ but is Abstract, so it also needs a subclass to be placed.
- **`USigilAbilityActionSetSettings`** — a const data asset mapping ability tags (+source/target tag conditions) to `FSigilAbilityAction` lists via `SelectBestAbilityActions`; query it through `ISigilCombatInterface::QueryAbilityActions`.

### Collision scanning (melee)

`USigilCollisionSystemComponent` (a `UPawnComponent`) owns pooled **`USigilCollisionTraceInstance`** objects created from `FSigilCollisionTraceDefinition` entries (`TraceDefinitions` on the component, or passed to `CreateTraceInstances`). Each instance traces from a `UPrimitiveComponent`'s sockets, optionally filters candidates through a `UTargetingPreset`, and broadcasts `OnHit` / `OnTraceStateChangedEvent`. Instances are cached and reused (`CachedTraceInstances`). Drive them from:

- `USigilAbilityTask_CollisionTrace::HandleCollisionTraces` — inside a gameplay ability, or
- `USigilAsyncAction_CollisionTrace::SetupAndListenForCollisionTraceHit` — from any Blueprint.

### Bullets (ranged)

`USigilBulletSubsystem` (world subsystem) spawns and pools **`ASigilBulletInstance`** actors (`SpawnBullets` / `TakeBulletFromPool` / `DestroyBullet`). **`FSigilBulletDefinition`** (`FTableRowBase`) drives everything: bullet class, `Duration`, multi-shot patterns (`BulletCount`, `LaunchAngle`, `LaunchAngleInterval`, `LaunchElevationAngle`), speed/gravity/radius over an `AttenuationRange`, penetration flags (`bPenetrateCharacter`, `bPenetrateMap`), VFX/SFX slots, a linked `AttackDefinition` row, chained bullets on hit/expiry (`HitBulletDefinition` gated by a `LaunchCondition` tag), and a `UserSettings` extension map. Launch-condition native tags: `Sigil.Combat.Bullet.LaunchCond.Always` / `.DidNotHitPawn` / `.HitPawn`. `ASigilSphereBulletInstance` is the provided shape implementation.

### Lock-on targeting

`USigilTargetingSystemComponent` (a `UPawnComponent`) maintains a replicated `TargetedActor` and a server-side `PotentialTargets` list refreshed through a `UTargetingPreset` (`bAutoUpdatePotentialTargets`). API includes `SearchForActorToTarget`, `StaticSwitchToNewTarget(bRightDirection)`, `SelectClosestActorFromPotentialTargets`, `FilterActorsWithPreset`, and the overridable `CanBeTargeted`. The plugin ships targeting tasks for the engine Targeting System: filters `USigilTargetingFilterTask_Affiliation`, `_IsDead`, `_TagsRequirements`, `_TraceInstance`, and selections `USigilTargetingSelectionTask_LineTrace`, `_TraceExt`, `_TraceExt_BindShape`, plus `ISigilTargetingSourceInterface` and `USigilTargetingFunctionLibrary`.

### Predictable montages

`USigilCombatSystemComponent::PlayPredictableMontageForTarget` plays a hit-reaction montage on a target: the instigating client plays it locally right away (`PredictedMontageInfo`), sends `ServerPlayPredictableMontageForTarget`, and the server replicates `ReplicatedMontageInfo` (with trigger time) so other clients join at the right position (`OnRep_ReplicatedMontageInfo`). `FSigilPlayMontageRequest` carries montage, `PlayRate`, `StartSectionName`, `RootTranslationScale`, `StartTimeSeconds`.

### Teams, weapons, interfaces

- **Teams** — `ISigilCombatTeamAgentInterface` + `USigilCombatTeamAgentComponent` hold a replicated `FGenericTeamId` (`CombatTeamId`, optionally pushed to the controller via `bAssignTeamIdToController`) and broadcast `OnTeamIdChangedEvent`. The affiliation targeting filter uses team identity; `bDisableAffiliationCheck` in settings turns it off for debugging.
- **Weapons** — `ISigilWeaponInterface` with the abstract default implementation `ASigilWeaponActor` (owner pawn, weapon tags, active state, primitive component, source object).
- **`ISigilCombatInterface`** — *implemented by your project* on the character: combat target accessors, `QueryAbilityActions`, `QueryWeapon` / `SigilGetWeapon`, block-input state, rotation/movement mode and state accessors, and the `StartDeath` / `FinishDeath` / `IsDead` lifecycle. Access it via `USigilCombatFunctionLibrary::GetCombatInterface`.

### Combat attributes and effect context

- **`USigilCombatSet`** — `Damage`, `DamageNegation`, `GuardDamageNegation`, `StaminaDamage`, `StaminaDamageNegation` (tags `Sigil.Attribute.CombatSet.*`).
- **`USigilPoiseSet`** — `Poise`, `MaxPoise`, `PoiseRecover` (tags `Sigil.Attribute.PoiseSet.*`).
- **`FSigilGameplayEffectContext`** — custom effect context that net-serializes the attack definition row handle (`SetAttackDefinitionHandle` / `GetAttackDefinitionHandle`), so the defender's Combat Flow can read the full attack row. Allocated by `USigilCombatAbilitySystemGlobals::AllocGameplayEffectContext` — which is why the globals class configuration below is mandatory.

## Prerequisites

- **sigil.gas** enabled (plus engine plugins Gameplay Abilities, Modular Gameplay, Targeting System, Motion Warping, Niagara — all declared by `SigilCombat.uplugin`).
- **`AbilitySystemGlobalsClassName` must be the sigil.combat globals class** (see Configuration) — without it the custom effect context is never allocated and attack definitions cannot travel inside effect specs.
- Your character class **implements `ISigilCombatInterface`**.
- The character's main mesh component carries the **component tag `Main`** (configurable — see Configuration) so `USigilCombatFunctionLibrary::GetMainCharacterMeshComponent` can find it.
- DataTables with row types **`FSigilAttackDefinition`** and (for ranged combat) **`FSigilBulletDefinition`**.
- A Blueprint subclass of **`USigilCombatFlow`** assigned to each `USigilCombatSystemComponent`'s `CombatFlowClass`.

## Quick Start

1. **Configure the globals class.** In `DefaultGame.ini`:

   ```ini
   [/Script/GameplayAbilities.AbilitySystemGlobals]
   AbilitySystemGlobalsClassName=/Script/SigilCombat.SigilCombatAbilitySystemGlobals
   ```

2. **Prepare the character.** Implement `ISigilCombatInterface`, tag the main mesh component `Main`, and add `USigilCombatSystemComponent`, `USigilCollisionSystemComponent` (melee), `USigilTargetingSystemComponent` (lock-on), and `USigilCombatTeamAgentComponent` (teams) as needed. The character also needs a sigil.gas ASC with `USigilCombatSet` (and optionally `USigilPoiseSet`) granted.

3. **Author attack data.** Create a DataTable with row type `FSigilAttackDefinition`; fill in attack tags, SetByCaller magnitudes, target effect class, cues, and knockback values.

4. **Create the Combat Flow.** Subclass `USigilCombatFlow` in Blueprint, add `USigilAttackResultProcessor` subclasses to `AttackResultProcessors` for hit reactions, and set the subclass as `CombatFlowClass` on the combat component.

5. **Wire melee attacks.** On the attack montage, place a Blueprint subclass of `USigilANS_AttackTrace` whose `AttackRequest` (melee) references your attack row and names the traces to enable; implement the notify logic to toggle the matching `USigilCollisionTraceInstance`s (via `USigilAbilityTask_CollisionTrace::HandleCollisionTraces` in the attacking ability). On hit, build the effect spec from the attack definition and apply it; register a `FSigilAttackResult` on the defender's combat component so its flow reacts.

6. **Wire ranged attacks.** Create a `FSigilBulletDefinition` DataTable row, then spawn from a Blueprint subclass of `USigilANS_BulletTrace` (or directly) with `USigilBulletSubsystem::SpawnBullets`, passing a `FSigilBulletSpawnParameters` with owner, definition row, transform, and the bullet attack request.

7. **Implement damage settlement** (project-side): a GameplayEffect/ExecutionCalculation that reads `CombatSet` attributes and SetByCaller magnitudes, computes final damage, and writes the defender's `IncomingDamage` (sigil.gas) and Health.

## Key Types

| Type | Description |
| --- | --- |
| `USigilCombatSystemComponent` | Per-character combat hub: owns the replicated Combat Flow, attack-result fast array, predictable montage playback. |
| `USigilCombatFlow` | Abstract, replicated hit-reaction pipeline object; subclass per character archetype (**required**). |
| `USigilAttackResultProcessor` | Instanced, Blueprintable reaction step run for each incoming `FSigilAttackResult`. |
| `FSigilAttackResult` / `FSigilAttackResultContainer` | Attack outcome payload and its replicated FastArray container. |
| `FSigilAttackDefinition` | DataTable row describing one attack (tags, SetByCaller, effects, cues, knockback, feedback, user settings). |
| `USigilAttackRequest_Melee` / `USigilAttackRequest_Bullet` | Instanced request objects embedded in notify states; resolve to an attack definition row. |
| `USigilANS_AttackTrace` / `USigilANS_BulletTrace` / `USigilANS_MovementCancellation` | Anim notify states; the first two carry request data and expect Blueprint notify logic. |
| `USigilCollisionSystemComponent` / `USigilCollisionTraceInstance` | Pooled socket-based melee trace scanning with `OnHit` events. |
| `USigilAbilityTask_CollisionTrace` / `USigilAsyncAction_CollisionTrace` | Ability-task and async-action front ends for the collision system. |
| `USigilBulletSubsystem` / `ASigilBulletInstance` / `FSigilBulletDefinition` | Pooled projectile system with multi-shot, penetration, chained bullets. |
| `USigilTargetingSystemComponent` | Lock-on targeting with replicated target, potential-target refresh, switch-target API. |
| `USigilTargetingFilterTask_*` / `USigilTargetingSelectionTask_*` | Targeting System tasks (affiliation, is-dead, tag requirements, trace-instance filters; line/ext trace selections). |
| `USigilCombatTeamAgentComponent` / `ISigilCombatTeamAgentInterface` | Replicated `FGenericTeamId` team affiliation. |
| `ASigilWeaponActor` / `ISigilWeaponInterface` | Weapon abstraction and default actor implementation. |
| `USigilCombatSet` / `USigilPoiseSet` | Combat/poise attribute sets (`Sigil.Attribute.CombatSet.*`, `Sigil.Attribute.PoiseSet.*`). |
| `FSigilGameplayEffectContext` / `USigilCombatAbilitySystemGlobals` | Custom effect context carrying the attack definition row handle; globals class that allocates it. |
| `ISigilCombatInterface` | Project-implemented character interface (targets, weapons, movement state, death lifecycle). |

## Configuration

- **`USigilCombatSystemSettings`** (`UDeveloperSettings`, `Config=Game`, shown in Project Settings):

  | Property | Default | Description |
  | --- | --- | --- |
  | `CharacterMeshLookupTag` | `Main` | Component tag used to find the character's main skeletal mesh. |
  | `bDisableAffiliationCheck` | `false` | Debug switch allowing cross-team damage/targeting. |

- **`DefaultGame.ini`** — `AbilitySystemGlobalsClassName=/Script/SigilCombat.SigilCombatAbilitySystemGlobals` (mandatory; subclass of sigil.gas's globals, so sigil.gas attribute-group initialization keeps working).
- **DataTables** — attacks: row type `FSigilAttackDefinition`; bullets: row type `FSigilBulletDefinition`. Both rows expose a `UserSettings` map (`FInstancedStruct`, base `SigilUserSetting`) for project extensions without modifying the plugin.
- **Per-component** — `CombatFlowClass` + `AttackResultProcessors` (combat), `TraceDefinitions` + `bAutoInitialize` (collision), `TargetingPreset` + `bAutoUpdatePotentialTargets` (targeting), `CombatTeamId` + `bAssignTeamIdToController` (team).

## Networking

**Replicated:**

- Combat Flow instance and the `FSigilAttackResultContainer` FastArray — attack results reach all clients, and the flow's processors run on each end via `PostReplicatedAdd`.
- Predictable montages — instigator-predicted locally, then server-replicated `ReplicatedMontageInfo` with trigger time for late-join alignment.
- `TargetedActor` (lock-on) and `CombatTeamId` (teams).
- `FSigilGameplayEffectContext` net-serializes the attack definition row handle inside effect specs.

**Local / not replicated:**

- **Collision trace scanning is not replicated** — trace instances are transient local objects; run them on whichever machine your ability flow designates (typically the server, or the owning client feeding target data to the server).
- The targeting component's `PotentialTargets` list is server-side only.

**Known gaps (as of this source):**

- **Bullet prediction reconciliation is not implemented.** Bullets support local prediction (`bIsLocalPredicting`, predicted bullet IDs in `FSigilBulletSpawnParameters`), and the server-spawned bullet detects its matching predicted client bullet — but the reconciliation hook `ASigilBulletInstance::FoundLocalPredictedBullet` has an empty default implementation. Projects that need smooth client-predicted projectiles must implement the hand-off themselves.
- Damage settlement is project-side (see above), so its network behavior is whatever your GameplayEffect setup does.

## See Also

- [sigil.gas](sigil-gas.md) — the ability-system foundation this plugin requires.
- [sigil.input](sigil-input.md) — tag-driven input for triggering combat abilities.
