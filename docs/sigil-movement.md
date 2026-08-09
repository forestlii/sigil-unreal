[English](sigil-movement.md) | [简体中文](sigil-movement.zh-CN.md)

# sigil.movement

**Plugin:** `SigilMovement` · **Modules:** `SigilMovement` (Runtime), `SigilMovementEditor` (UncookedOnly) · **Depends on:** ModularGameplay, AnimationWarping, AnimationLocomotionLibrary, PoseSearch, Chooser (engine plugins), Gameplay Tags

sigil.movement (logged and categorized as **GMS** — Generic Movement System) is a data-driven movement control and locomotion animation framework in the Lyra style: a traditional animation state machine driven by distance matching, stride/orientation warping, and Gameplay Tags. It is **not** a Motion Matching pipeline — PoseSearch and Chooser are declared dependencies used only by side utilities (`USigilUtility::EvaluatePoseSearchDatabasesChooser` and friends); the main locomotion path never requires a PoseSearch database.

## Overview

### Component layer

`USigilMovementSystemComponent` is the abstract, non-Blueprintable base. It owns the replicated core state — `MovementSet`, `DesiredMovementState`/`MovementState`, `DesiredRotationMode`/`RotationMode`, `LocomotionMode`, `OverlayMode`, `OwnedTags` — and the RPC plumbing that keeps that state in sync between server, owning client, and simulated proxies. Every state change fires a matching `BlueprintAssignable` event (`OnMovementSetChangedEvent`, `OnMovementStateChangedEvent`, `OnRotationModeChangedEvent`, `OnLocomotionModeChangedEvent`, `OnOverlayModeChangedEvent`).

`USigilCharacterMovementSystemComponent` (DisplayName *"GMS Movement System Component(Character)"*) is the concrete implementation for `ACharacter` + `UCharacterMovementComponent`. It applies the selected movement state's speeds and acceleration to the CMC, runs the grounded/in-air rotation system, refreshes the view state, and maps CMC movement modes to `Sigil.Movement.LocomotionMode.*` tags via `MovementModeToTagMapping` / `CustomMovementModeToTagMapping`.

`USigilMoverMovementSystemComponent` exists but is an empty WIP shell — its own header says *"You should not use this class."* Mover support is not functional.

### Data asset chain

```
USigilMovementDefinition (Const DataAsset)
  └─ MovementSets : TMap<FGameplayTag, FSigilMovementSetSetting>
       ├─ ControlSetting : USigilMovementControlSetting_Default   (logic: speeds, jump params, rotation modes)
       ├─ ControlSettings per OverlayMode (optional)
       ├─ AnimDataSetting_General : FSigilAnimDataSetting_General (shared anim settings)
       └─ 5 anim layer settings:
            States  → USigilAnimLayerSetting_States   (instanced or shared DataAsset)
            Overlay → USigilAnimLayerSetting_Overlay  (instanced or shared DataAsset)
            View    → USigilAnimLayerSetting_View
            Additive → USigilAnimLayerSetting_Additive
            SkeletalControls → USigilAnimLayerSetting_SkeletalControls

USigilAnimGraphSetting (DataAsset, one per unique skeleton)
  ├─ AnimLayerSettingToInstanceMapping : TMap<setting class, USigilAnimLayer class>
  └─ OrientationWarping : FSigilOrientationWarpingSettings (spine/IK bone references)
```

The component holds a runtime **stack** of definitions (`MovementDefinitions`, replicated). `PushAvailableMovementDefinition` / `PopAvailableMovementDefinition` let you overlay a weapon-specific definition at runtime (e.g. equip a greatsword → push its definition); when `MovementSet` changes, the list is searched bottom-up, so the most recently pushed definition wins.

### Animation layer system

`USigilMainAnimInstance` is the required main AnimInstance. On every core-state change it calls `RefreshLayerSettings()`, which resolves each anim layer setting through `USigilAnimGraphSetting::AnimLayerSettingToInstanceMapping` and links/unlinks the matching `USigilAnimLayer` (a linked-layer `UAnimInstance`) onto itself, then feeds the setting object in through `ApplySetting`.

Shipped layer families:

- **States** — `USigilAnimLayerSetting_States_Default` + `USigilAnimLayer_States_DefaultLocomotion`: the Lyra-like ground locomotion state machine (Idle/IdleBreaks, Start, Cycle, Stop, Pivot, TurnInPlace, Jump/Fall/Land) implemented in C++ for performance. The class is `Abstract`: you subclass it with an Animation Blueprint that contains the actual state machine graph and calls its `*_AnimUpdate` / `*_StateUpdate` functions from node bindings.
- **Overlay** — two native implementations: `USigilAnimLayerSetting_Overlay_Stack` / `USigilAnimLayer_Overlay_Stack` (up to 10 parallel body-part stacks, each choosing the first `FSigilAnimData_Overlay` whose `TagQuery` matches the component's tags) and `USigilAnimLayerSetting_Overlay_PoseBased` / `USigilAnimLayer_Overlay_PoseBased` (arm/idle/moving pose blending).
- **View** — `USigilAnimLayerSetting_View_Default` / `USigilAnimLayer_View_Default`: aim-offset blend space with yaw limits and smoothing.
- **Additive / SkeletalControls** — `USigilAnimLayerSetting_Additive` and `USigilAnimLayerSetting_SkeletalControls` are **empty abstract base classes**. They are extension points only; no native implementation ships. Wire your own subclass through `AnimLayerSettingToInstanceMapping`.

`USigilAnimGraph_Layering` is a helper AnimInstance that reads per-body-part layering curves (`LayerHead`, `LayerArmLeft`, `LayerSpineAdditive`, …) into a layering state for custom layered graphs.

### Custom anim graph nodes

The runtime module ships four anim nodes (editor-side graph nodes live in `SigilMovementEditor`):

| Node | Purpose |
| --- | --- |
| `FSigilAnimNode_GameplayTagsBlend` | Blend list keyed by a Gameplay Tag instead of an enum/int. |
| `FSigilAnimNode_CurvesBlend` | Blends only the curves of a second pose into the source pose (modes mirroring `ECurveBlendOption`). |
| `FSigilAnimNode_LayeredBoneBlend` | Layered per-bone blend whose branch filters can be fed **dynamically** via the `ExternalLayerSetup` pin. |
| `FSigilAnimNode_OrientationWarping` | Orientation warping variant whose spine/IK bone references come from an `FSigilOrientationWarpingBoneReference` pin (fed from `USigilAnimGraphSetting`), so one graph serves multiple skeletons. |

### Gameplay Tags as the state vocabulary

Tags drive everything. The component's `GetGameplayTags()` merges `OwnedTags` with tags from an optional `GameplayTagsProvider` (any object implementing `IGameplayTagAssetInterface`). Setting your Ability System Component as the provider lets GAS tags drive overlay selection and rotation blocking (`GroundedRotationBlockingTags`, `InAirRotationBlockingTags`) — this is the intended GAS bridge; the plugin itself has no GAS dependency.

## Prerequisites

The plugin contains **no content** (`CanContainContent: false`): no skeleton, no sample Animation Blueprints, no pre-made data assets. Everything below must exist in your project.

- **Owner actor**: the base component `check`s that its owner is an `APawn`; `USigilCharacterMovementSystemComponent` additionally requires an `ACharacter` with a `UCharacterMovementComponent` (a non-Character owner leaves the component non-functional and will fail `ensure`/null-access at `BeginPlay`).
- **Controller rotation flags off**: `bUseControllerRotationPitch/Yaw/Roll` must all be `false` — enforced by `ensureMsgf` at `BeginPlay`. The rotation system owns actor yaw.
- **Main AnimInstance**: the mesh's anim class must derive from `USigilMainAnimInstance`. Every linked `USigilAnimLayer` runs `checkf(Parent != nullptr, TEXT("Parent is not SigilMainAnimInstance!"))` — linking any Sigil anim layer under a different AnimInstance **crashes**.
- **Gameplay Tags**: the plugin natively registers:
  - `Sigil.Movement.LocomotionMode.{None, Grounded, InAir, Flying, Swimming}`
  - `Sigil.Movement.RotationMode.{VelocityDirection, ViewDirection}`
  - `Sigil.Movement.State.{Walk, Jog, Sprint}`
  - `Sigil.Movement.OverlayMode.{None, Default}`
  - `Sigil.Movement.SM` and `Sigil.Movement.SM.{InAir, InAir.Jump, InAir.Fall, Grounded, Grounded.Idle, Grounded.Start, Grounded.Cycle, Grounded.Stop, Grounded.Pivot, Grounded.Land}`

  **`Sigil.Movement.Set.*` has no native tags** — you must create your own set tags (e.g. `Sigil.Movement.Set.Unarmed`) in the project's tag settings; every `SetMovementSet` call and `MovementSets` map key expects them.
- **Animation curves** on your animation assets:
  - `Distance` — required by distance matching in Start, Stop and Pivot animations (`UAnimDistanceMatchingLibrary` calls with `FName("Distance")`).
  - `GroundDistance` — required on the optional `JumpFallLand` animation (distance-matched against the ground-prediction trace).
  - `RotationYawSpeed`, `RotationYawOffset` — read by `USigilCharacterMovementSystemComponent` to let animation drive/offset actor yaw (turn-in-place etc.).
  - `AllowTurnInPlace`, `AllowAiming` — name constants exposed by `USigilConstants` for use in your animation graphs; the runtime module itself does not sample them.
- **Montage slot**: a slot named `TurnInPlace` (`USigilConstants::TurnInPlaceSlotName()`).
- **Animation Blueprints**: a main ABP deriving from `USigilMainAnimInstance`, plus one ABP per anim layer you use (e.g. a states ABP deriving from `USigilAnimLayer_States_DefaultLocomotion`), registered in `USigilAnimGraphSetting`.

## Quick Start

1. **Create the tag vocabulary.** Add at least one movement set tag, e.g. `Sigil.Movement.Set.Default`, in **Project Settings → Gameplay Tags**.

2. **Create a Control Setting.** Add a `USigilMovementControlSetting_Default` data asset (*GMS Movement Control Setting*). Fill `MovementStates` sorted by speed — each `FSigilMovementStateSetting` carries a `Sigil.Movement.State.*` tag, `SpeedLevel`, `Speed` / `StrafeSpeed` / `BackwardsSpeed`, `Acceleration`, `BrakingDeceleration`, allowed rotation modes and per-mode rotation settings. `MovementStates` **must not be empty** (`checkf` otherwise). Optionally fill `JumpStates`.

3. **Create anim layer settings + Animation Blueprints.**
   - Subclass `USigilAnimLayer_States_DefaultLocomotion` with an ABP implementing the ground state machine; create a `USigilAnimLayerSetting_States_Default` and assign idle/turn/jump/land data plus per-movement-state Start/Cycle/Stop/Pivot animations (`MovingStates`, keyed by `Sigil.Movement.State.*`). `MovingStates` must not be empty.
   - Optionally add an overlay setting (`USigilAnimLayerSetting_Overlay_Stack` or `_PoseBased`) and a view setting (`USigilAnimLayerSetting_View_Default`) with their ABPs.

4. **Create a `USigilAnimGraphSetting`** for your skeleton and map each setting class to its anim layer (ABP) class in `AnimLayerSettingToInstanceMapping`. Configure `OrientationWarping` bone references (spine chain, IK foot root, IK feet).

5. **Create a `USigilMovementDefinition`.** Add an entry to `MovementSets` keyed by your set tag; inside the `FSigilMovementSetSetting`, assign the control setting from step 2 and the anim layer settings from step 3.

6. **Set up the Character.**
   - Add `USigilCharacterMovementSystemComponent`; set `MovementDefinitions` (at least one entry), `MovementSet` to your set tag, and `AnimGraphSetting`.
   - Set the mesh's anim class to your main ABP (derived from `USigilMainAnimInstance`).
   - Turn off `bUseControllerRotationYaw` (and Pitch/Roll); set `bOrientRotationToMovement` on the CMC to `false` — the component drives rotation.

7. **Drive it from gameplay:**

   ```cpp
   USigilMovementSystemComponent* MSC =
       USigilMovementSystemComponent::GetMovementSystemComponent(Character);

   MSC->SetDesiredMovement(SigilMovementStateTags::Sprint);          // walk/jog/sprint
   MSC->SetDesiredRotationMode(SigilRotationModeTags::VelocityDirection);
   MSC->SetOverlayMode(OverlayTag);                                  // Sigil.Movement.OverlayMode.*
   MSC->SetMovementSet(WeaponSetTag);                                // Sigil.Movement.Set.* (project tag)
   MSC->PushAvailableMovementDefinition(GreatswordDefinition);       // runtime definition stack
   ```

## Key Types

| Type | Description |
| --- | --- |
| `USigilMovementSystemComponent` | Abstract base component: replicated core state (set/state/rotation/locomotion/overlay), tag container + `GameplayTagsProvider`, definition stack, change events. |
| `USigilCharacterMovementSystemComponent` | Character/CMC implementation: applies movement state settings to the CMC, grounded & in-air rotation system, view smoothing, distance-matching prediction params. |
| `USigilMoverMovementSystemComponent` | WIP placeholder for the Mover plugin. Do not use. |
| `USigilMovementDefinition` | Const data asset: `MovementSets` map (set tag → `FSigilMovementSetSetting`). |
| `FSigilMovementSetSetting` | One movement set: control setting (optionally per overlay mode), general anim settings, five anim layer settings, and extensible `UserSettings` (`USigilMovementSetUserSetting`). |
| `USigilMovementControlSetting_Default` | Logic data asset: `MovementStates` (speed table), `JumpStates`, moving threshold, in-air rotation mode, `OnMovementStatesUpdated` / `OnJumpStatesUpdated` broadcast delegates. |
| `FSigilMovementStateSetting` | One gait: tag, speed level, directional speeds, acceleration/braking, allowed rotation modes, rotation interpolation parameters. |
| `USigilAnimGraphSetting` | Per-skeleton data asset: anim layer setting → anim layer class mapping, orientation-warping bone references. |
| `USigilMainAnimInstance` | Required main AnimInstance: refreshes and links anim layers, exposes thread-safe locomotion/view/lean/in-air states and node-relevance tags. |
| `USigilAnimLayer` | Base class for all linked anim layers; `ApplySetting`/`ResetSetting`, `OnLinked`/`OnUnlinked`. Must be linked under `USigilMainAnimInstance`. |
| `USigilAnimLayer_States_DefaultLocomotion` | Native Lyra-like ground locomotion layer (idle, start, cycle, stop, pivot, turn-in-place, jump/fall/land) — subclass in an ABP. |
| `USigilAnimLayer_Overlay_Stack` / `USigilAnimLayer_Overlay_PoseBased` | The two native overlay layer implementations. |
| `USigilAnimLayer_View_Default` | Native aim-offset view layer. |
| `USigilConstants` | Canonical names: `TurnInPlace` slot, `RotationYawSpeed`, `RotationYawOffset`, `AllowTurnInPlace`, `AllowAiming` curves. |
| `USigilUtility` | Blueprint utilities, including the PoseSearch/Chooser side helpers (`EvaluatePoseSearchDatabasesChooser`, `IsValidPoseSearchDatabasesChooser`). |

## Configuration

Configuration is entirely asset-based (no `DeveloperSettings`). The chain, from coarse to fine:

1. **`USigilMovementDefinition`** — which movement sets exist. Multiple definitions can be stacked at runtime; lower (later-pushed) definitions override earlier ones for the same set tag.
2. **`FSigilMovementSetSetting`** — per set: the control setting (optionally one per overlay mode via `bControlSettingPerOverlayMode` + `ControlSettings`), shared anim data (`FSigilAnimDataSetting_General`: root-bone rotation offset, lean speeds, ground-prediction channels), and the five layer settings. States/Overlay settings can be either instanced inline (`bUseInstancedStatesSetting` / `bUseInstancedOverlaySetting`) or referenced as shared data assets to reuse across sets.
3. **`USigilAnimGraphSetting`** — per skeleton. If you add custom layer settings/implementations, register them in `AnimLayerSettingToInstanceMapping`.
4. **`USigilMovementSetUserSetting`** — subclass to attach arbitrary custom data to a movement set; read it from your layers with `USigilUtility::GetMovementSetUserSetting`.

Data assets implement editor-time validation (`IsDataValid`, `PreSave` acceleration maps), so most wiring mistakes surface in the editor.

## Networking

- **Replicated:** `MovementDefinitions` (stack), `MovementSet`, `DesiredRotationMode`, `DesiredMovementState`, `OwnedTags`, `OverlayMode`, `InputDirection` (quantized), `DesiredVelocityYawAngle`, `ReplicatedViewRotation`. Setters route through paired `Server*`/`Client*` reliable RPCs so any role can initiate a change; view rotation uses unreliable RPCs with network smoothing.
- **Local only:** `LocomotionMode`, `MovementState`, `RotationMode` (transient — derived on each machine), the whole locomotion/view state block, and everything inside the AnimInstance/anim layers.
- There is **no movement prediction integration**: the component does not extend `FSavedMove`/CMC prediction; it configures the standard `UCharacterMovementComponent`, which keeps its own engine-side prediction.

## Known gaps and extension points

Be aware of these before shipping on top of the plugin — they are intentional or current limitations verified in source:

- **Jump parameters are broadcast, not applied.** `FSigilJumpStateSetting` (gravity scale, air control, etc.) is only forwarded via `USigilMovementControlSetting_Default::OnJumpStatesUpdated`; nothing writes these values to the CMC. Bind the delegate and apply them yourself.
- **Mover support is an empty shell** (`USigilMoverMovementSystemComponent`).
- **Crouch, swimming and climbing are not built-in states.** `Sigil.Movement.LocomotionMode.Swimming/Flying` tags exist and CMC modes can map to them, but no swimming/flying/crouch control or animation logic ships; the idle anim data only reserves optional `CrouchEntry`/`CrouchExit` sequences.
- **Additive and SkeletalControls layers are empty bases** — extension points only.
- `bAllowRefreshCharacterMovementSettings = false` plus `SpeedToMovementStateCurve` lets an external system (e.g. AI) own CMC speed while GMS derives the movement state from actual speed.

## See Also

- [sigil.input](sigil-input.md) — tag-driven input that pairs naturally with `SetDesiredMovement` / `SetDesiredRotationMode`.
- [sigil.inventory](sigil-inventory.md) — equipment events are a natural trigger for `PushAvailableMovementDefinition` / `SetMovementSet`.
