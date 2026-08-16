[English](sigil-camera.md) | [简体中文](sigil-camera.zh-CN.md)

# sigil.camera

**Plugin:** `SigilCamera` · **Modules:** `SigilCamera` (Runtime) · **Depends on:** Gameplay Tags (engine module; no engine plugin dependencies)

sigil.camera is a camera-mode stack in the style of Lyra's camera system, with one architectural difference: instead of producing a final `FMinimalViewInfo` for a custom camera component, the stack **drives the parameters of a standard `USpringArmComponent` + `UCameraComponent` pair** that you already have on your pawn. Camera modes are blended on a stack with per-mode blend curves, and an optional penetration-avoidance base class ports Lyra's feeler-ray logic.

## Overview

### The mode stack drives your SpringArm

`USigilCameraSystemComponent` (an `ActorComponent`) owns a `USigilCameraModeStack`. Every tick it:

1. Polls `DetermineCameraModeDelegate` (a native, C++-only delegate returning a `TSubclassOf<USigilCameraMode>`) and pushes the returned mode, if bound.
2. Evaluates the stack into a blended `FSigilCameraModeView`.
3. Applies the result to the associated components: `ControlRotation` → owning pawn's PlayerController, `FieldOfView` → camera component, `SpringArmLength` → `TargetArmLength`, `SpringArmSocketOffset` → `SocketOffset`, `SpringArmTargetOffset` → `TargetOffset`.

The view struct also carries `Location` and `Rotation` fields which participate in blending, but they are **not applied** to any component by the tick — see Known limitations.

Modes can also be pushed explicitly with `PushCameraMode(Class)` or `PushDefaultCameraMode()` (which pushes the editable `DefaultCameraMode` property). `GetBlendInfo` returns the top layer's `CameraTypeTag` and blend weight, so gameplay code can ask "am I aiming?" without knowing concrete mode classes.

### Camera modes

`USigilCameraMode` is `Abstract, Blueprintable` — **the plugin ships no concrete camera mode**; your project must subclass it (typically in Blueprint). The key override is the `BlueprintNativeEvent` `OnUpdateView(DeltaTime, PivotLocation, PivotRotation)`: write the desired camera state into the `View` property. `GetPivotLocation` / `GetPivotRotation` are also `BlueprintNativeEvent`s; the native defaults use the Character capsule (crouch-aware) or fall back to the pawn's view location/rotation.

Each mode carries: `FieldOfView`, `ViewPitchMin` / `ViewPitchMax`, blending settings (`BlendTime`, `BlendFunction` — Linear / EaseIn / EaseOut / EaseInOut — and `BlendExponent`), a `CameraTypeTag` gameplay tag, and `MaxActiveTime` (when the active time exceeds it, the stack returns to the default camera mode). `OnActivation` / `OnDeactivation` Blueprint events fire when a mode enters or leaves the stack.

The stack keeps an instance pool (`CameraModeInstances`): each mode class is instantiated once per stack and reused, so mode state persists between activations.

### Penetration avoidance

`USigilCameraMode_WithPenetrationAvoidance` (also `Abstract, Blueprintable`) ports Lyra's feeler-based collision logic: an array of `FSigilCameraPenetrationAvoidanceFeeler` rays (feeler 0 is the main ray; feelers 1+ are predictive when `bDoPredictiveAvoidance` is on), `bPreventPenetration`, `PenetrationBlendInTime` / `PenetrationBlendOutTime`, `CollisionPushOutDistance`, and `ReportPenetrationPercent`. The class does **not** run this logic automatically — it exposes `UpdatePreventPenetration(DeltaTime)` as a `BlueprintCallable` for your `OnUpdateView` implementation to call.

Cooperating actors can implement `ISigilCameraAssistInterface` (C++-only virtuals): `GetIgnoredActorsForCameraPentration`, `GetCameraPreventPenetrationTarget`, and `OnCameraPenetratingTarget`. Actors tagged with the actor tag `IgnoreCameraCollision` are skipped by the feeler traces.

### Coordinate ownership and penetration avoidance

The **SpringArm owns the final camera transform**: the system component only writes control rotation, FOV, arm length and the two arm offsets each tick; `View.Location` / `View.Rotation` are the pivot the arm hangs from, not a camera position. `UpdatePreventPenetration` follows the same model — it computes the camera location the arm would produce, feels along the aim line, and **shortens `View.SpringArmLength`** by the blocked fraction. Call it from your `OnUpdateView` after filling the view, and **disable the SpringArm's own `bDoCollisionTest`** so the two systems do not both push the camera in.

`AddFieldOfViewOffset` is a one-frame offset: it is added to the FOV the tick it is applied and then cleared, so callers must re-apply it every frame they want it.

### Known limitations (honest notes)

- Penetration avoidance approximates the corrected position by scaling the arm length along the aim line; large socket/target offsets make the approximation coarser. It has not been validated in PIE (wall corner / doorway / back-to-wall scenes are still to be tested).
- `DetermineCameraModeDelegate` is a plain C++ delegate — it cannot be bound from Blueprint. Blueprint-only projects should call `PushCameraMode` / `PushDefaultCameraMode` instead.

## Prerequisites

- [ ] A pawn with a **`USpringArmComponent` and a `UCameraComponent`** already set up (the plugin does not create them).
- [ ] **A call to `Initialize(CameraComponent, SpringArmComponent)`** on the system component, e.g. in `BeginPlay`. This is mandatory: the tick early-outs unless both associated components are set, so without it the whole system silently does nothing.
- [ ] **At least one project-made camera mode** — a Blueprint (or C++) subclass of `USigilCameraMode` or `USigilCameraMode_WithPenetrationAvoidance` implementing `OnUpdateView`. The plugin contains no usable concrete mode.
- [ ] Optionally, gameplay tags for `CameraTypeTag` values (plain tags; the plugin declares none).

## Quick Start

1. **Create a camera mode.** Make a Blueprint subclass of `USigilCameraMode` (or `USigilCameraMode_WithPenetrationAvoidance`). Implement **OnUpdateView**: at minimum set `View.SpringArmLength`, `View.SpringArmSocketOffset` / `SpringArmTargetOffset`, `View.ControlRotation`, and `View.FieldOfView` from the pivot inputs. Configure `BlendTime` / `BlendFunction`.
2. **Add the component.** Add `USigilCameraSystemComponent` to the pawn that owns the spring arm and camera, and set `DefaultCameraMode` to your mode class.
3. **Initialize.** In the pawn's `BeginPlay`, call `Initialize(Camera, SpringArm)` on the component, then `PushDefaultCameraMode()`.
4. **Switch modes at runtime.** Call `PushCameraMode(OtherModeClass)` (e.g. an aim mode when aiming starts) — the stack cross-fades using the new mode's blend settings. Push the default mode again to return. In C++ you can instead bind `DetermineCameraModeDelegate` and let the component poll it each tick.
5. **Query state.** Use `GetBlendInfo(OutWeight, OutTag)` to drive gameplay logic off the active mode's `CameraTypeTag`, and `GetCameraSystemComponent(Actor)` to locate the component.
6. **Debug.** Enter `showdebug CAMERA` in the console; the component registers a debug drawer showing the mode stack.

## Key Types

| Type | Description |
| --- | --- |
| `USigilCameraSystemComponent` | Actor component owning the mode stack; must be initialized with a camera + spring arm pair; applies the blended view every tick. |
| `USigilCameraMode` | Abstract, Blueprintable base for camera modes; override `OnUpdateView` (plus optionally `GetPivotLocation` / `GetPivotRotation`); holds FOV, pitch limits, blend settings, `CameraTypeTag`, `MaxActiveTime`. |
| `USigilCameraModeStack` | Blending stack with per-class instance pooling; evaluates to a single `FSigilCameraModeView`. |
| `FSigilCameraModeView` | Blended view data: `Location`, `Rotation`, `SpringArmSocketOffset`, `SpringArmTargetOffset`, `SpringArmLength`, `ControlRotation`, `FieldOfView`. |
| `ESigilCameraModeBlendFunction` | `Linear`, `EaseIn`, `EaseOut`, `EaseInOut`. |
| `USigilCameraMode_WithPenetrationAvoidance` | Abstract mode base adding Lyra-style feeler traces; call `UpdatePreventPenetration` from your `OnUpdateView`. |
| `FSigilCameraPenetrationAvoidanceFeeler` | One feeler ray: `AdjustmentRot`, `WorldWeight`, `PawnWeight`, `Extent`, `TraceInterval`. |
| `ISigilCameraAssistInterface` | Optional C++ interface for penetration cooperation (ignored actors, target override, overlap notification). |

## Configuration

All configuration is per-asset / per-component; there are no project settings:

- **On the component** — `DefaultCameraMode`.
- **On each mode class (class defaults)** — `FieldOfView`, `ViewPitchMin` / `ViewPitchMax`, `BlendTime`, `BlendFunction`, `BlendExponent`, `CameraTypeTag`, `MaxActiveTime`; penetration modes add `PenetrationAvoidanceFeelers` and the penetration toggles.
- **Per actor** — add the `IgnoreCameraCollision` actor tag to exclude an actor from penetration feeler hits.

## Networking

Entirely local. Nothing in this module replicates; camera modes evaluate on the machine viewing the pawn. The only cross-system write is `SetControlRotation` on the local PlayerController.

## See Also

- [sigil.input](sigil-input.md) — input layer that typically triggers camera mode switches.
- [sigil.ui](sigil-ui.md) — game UI layer.
