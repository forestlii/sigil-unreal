[English](sigil-input.md) | [简体中文](sigil-input.zh-CN.md)

# sigil.input

**Plugin:** `SigilInput` · **Modules:** `SigilInput` (Runtime) · **Depends on:** Enhanced Input (engine plugin), Gameplay Tags

sigil.input is a tag-driven input abstraction layer built on top of Enhanced Input. Instead of binding gameplay code directly to `UInputAction` assets, you map each action to a Gameplay Tag once, then listen for tag-based input events anywhere in your game. The plugin adds a data-driven permission pipeline (checkers), a dispatch pipeline (processors), and an input buffering system for action-game-style queued inputs.

## Overview

### Tag-based input events

`USigilInputSystemComponent` is the core of the system. Attach it to a Pawn or a PlayerController. On registration it adds its `InputMappingContext` to the Enhanced Input local player subsystem and binds every `UInputAction` listed in its `USigilInputConfig`. From that point on, every Enhanced Input trigger event (`Started`, `Triggered`, `Ongoing`, `Canceled`, `Completed`) is converted into a `(FInputActionInstance, FGameplayTag, ETriggerEvent)` tuple and pushed through the control pipeline.

Input tags are expected to live under the `InputTag` or `Sigil.Input.InputTag` tag roots — every tag property in the plugin is filtered with `meta=(Categories="InputTag,Sigil.Input.InputTag")`.

### The control pipeline: Checkers and Processors

Each component holds a stack of `USigilInputControlSetup` data assets (`InputControlSetups`); the last entry is the active setup. A setup runs two phases for every incoming event:

1. **Check** — every instanced `USigilInputChecker` in `InputCheckers` must approve the input. The built-in `USigilInputChecker_TagRelationship` checks the owning actor's tags against a `FGameplayTagQuery` and only allows the inputs listed for the matching relationship.
2. **Process** — if the input passed, the instanced `USigilInputProcessor` objects in `InputProcessors` handle it. Each processor filters by `InputTags` and `TriggerEvents`, and dispatches to per-event Blueprint-implementable handlers (`HandleInputStarted`, `HandleInputTriggered`, `HandleInputOngoing`, `HandleInputCanceled`, `HandleInputCompleted`). `InputProcessorExecutionType` selects whether all matching processors run (`MatchAll`) or only the first (`FirstOnly`).

Use `PushInputSetup` / `PopInputSetup` to swap the whole rule set at runtime — for example a separate setup for menus, vehicles, or cutscenes.

### Input buffering

If the active setup enables `bEnableInputBuffer`, inputs that are *rejected* by the checkers can be stored instead of dropped. Buffer windows are defined in `USigilInputConfig::InputBufferDefinitions` and opened/closed by name with `OpenInputBufferWindow` / `CloseInputBufferWindow` (typically from animation notifies during attack recovery). Each `FSigilInputBufferWindow` declares which inputs it accepts (`AllowedInputs`) and a `ESigilInputBufferType` policy:

| Buffer type | Behavior |
| --- | --- |
| `LastInput` | Only the last stored input fires when the window closes. |
| `Instant` | The input fires immediately while the window is open. |
| `HighestPriority` | Inputs earlier in the `AllowedInputs` list win over later ones; the winner fires when the window closes. |

Fired buffered inputs are broadcast through `OnFireBufferedInput`.

### Diagnostics

The component keeps rolling histories (`PassedInputEntries`, `BlockedInputEntries`, `BufferedInputEntries`, capped by `MaxInputEntriesNum`) and the plugin registers a Gameplay Debugger category named **SigilInput** that visualizes buffer windows and the entry histories. Verbose logging goes to the `LogSigilInput` category; per-setup debug logging (`bEnableInputDebug`) requires `LogSigilInput` to be at `VeryVerbose`.

## Prerequisites

Before using the plugin, make sure you have:

- The **Enhanced Input** plugin enabled (declared as a dependency by `SigilInput.uplugin`).
- Gameplay Tags for your inputs created under `InputTag.*` or `Sigil.Input.InputTag.*` (for example `InputTag.Jump`, `InputTag.Attack`).
- A `UInputMappingContext` and `UInputAction` assets for your keys, as with any Enhanced Input project.
- A Pawn or PlayerController to host `USigilInputSystemComponent`. The component asserts this owner requirement.

## Quick Start

1. **Create an Input Config.** Add a `USigilInputConfig` data asset. In `InputActionMappings`, map each input tag to a `FSigilInputActionSetting` (the `UInputAction` plus `bValueBinding`, which keeps the current action value queryable through `GetInputActionValueOfInputTag`).

2. **Create an Input Control Setup.** Add a `USigilInputControlSetup` data asset. Add instanced checkers to `InputCheckers` (or leave empty to allow everything) and instanced processors to `InputProcessors`. A minimal processor subclass in Blueprint overrides `HandleInputStarted` and, for example, calls an ability or moves the character.

3. **Add the component.** On your Pawn or PlayerController, add `USigilInputSystemComponent` and assign:
   - `InputMappingContext` and `InputPriority`
   - `InputConfig` (from step 1)
   - `InputControlSetups` (at least one entry, from step 2)

4. **React to input.** Either implement processors (recommended, data-driven), bind to the component's `OnReceivedInput` delegate, or use the Blueprint async node **Listen Input Event** (`USigilAsyncAction_ListenInputEvent::ListenInputEvent`), which can also listen for buffered input firings.

5. **Optional — buffering.** Define buffer windows in `USigilInputConfig::InputBufferDefinitions`, set `bEnableInputBuffer` on the setup, then call `OpenInputBufferWindow(Tag)` / `CloseInputBufferWindow(Tag)` around the frames where you want rejected inputs queued. Bind `OnFireBufferedInput` to consume the queued input.

6. **Query values in C++ or Blueprint** as needed:

   ```cpp
   USigilInputSystemComponent* Input = USigilInputSystemComponent::GetInputSystemComponent(Actor);
   FInputActionValue Move = Input->GetInputActionValueOfInputTag(MoveTag);
   bool bAllowed = Input->CheckInputAllowed(JumpTag, ETriggerEvent::Started);
   ```

## Key Types

| Type | Description |
| --- | --- |
| `USigilInputSystemComponent` | Core actor component; binds Enhanced Input actions, runs the check/process pipeline, owns the input buffer. Must live on a Pawn or PlayerController. |
| `USigilInputConfig` | Const data asset: `InputActionMappings` (tag → action) and `InputBufferDefinitions` (buffer windows). |
| `USigilInputControlSetup` | Const data asset bundling `InputCheckers`, `InputProcessors`, `bEnableInputBuffer`, and processor execution order. Stacked via `PushInputSetup`/`PopInputSetup`. |
| `USigilInputChecker` | Abstract, Blueprintable validator. Override `DoCheckInput` to approve or reject an input event. |
| `USigilInputChecker_TagRelationship` | Built-in checker: matches the actor's tags against `FGameplayTagQuery` rules and allows only the listed inputs. Override `GetActorTags` to supply tags. |
| `USigilInputProcessor` | Blueprintable handler; filters by `InputTags`/`TriggerEvents` and exposes per-trigger-event Blueprint events. |
| `FSigilInputActionSetting` | `UInputAction` reference plus `bValueBinding`. |
| `FSigilInputBufferWindow` | Buffer window definition: `Tag`, `BufferType`, `AllowedInputs`. |
| `FSigilAllowedInput` | An input tag plus the trigger events it is allowed for (empty = all events). |
| `FSigilBufferedInput` | A recorded input entry: tag, `FInputActionInstance` data, trigger event. |
| `ESigilInputBufferType` | `LastInput`, `Instant`, `HighestPriority`. |
| `ESigilInputProcessorExecutionType` | `MatchAll` or `FirstOnly` processor dispatch. |
| `USigilAsyncAction_ListenInputEvent` | Blueprint async node to listen for tag-filtered input events or buffered-input firings. |
| `USigilInputFunctionLibrary` | Helpers: `GetInputActionValue` (autocast from `FInputActionInstance`), tag-name utilities, tag query description. |

## Configuration

All configuration is asset-based; there are no project settings or `DeveloperSettings` for this plugin.

- **`USigilInputConfig`** — one per input scheme. Both it and `USigilInputControlSetup` implement editor-time data validation (`IsDataValid`), so configuration mistakes surface in the editor's data validation pass.
- **`USigilInputControlSetup`** — one per input "mode". Notable properties:

  | Property | Description |
  | --- | --- |
  | `InputCheckers` | Instanced checker list; all must pass. |
  | `bEnableInputBuffer` | Store rejected inputs into open buffer windows (off by default; not needed for UI-style setups). |
  | `InputProcessorExecutionType` | `MatchAll` (default) or `FirstOnly`. |
  | `InputProcessors` | Instanced processor list, executed top to bottom. |
  | `bEnableInputDebug`, `DebugInputTags`, `DebugTriggerEvents` | Per-setup debug logging filters; requires `LogSigilInput` at `VeryVerbose`. |

- **`USigilInputSystemComponent`** — per-actor: `InputMappingContext`, `InputPriority`, `InputConfig`, `InputControlSetups`, `MaxInputEntriesNum` (0–10, default 5), and `bProcessingInputExternally` to bypass the built-in pipeline and handle `OnReceivedInput` yourself.

## Networking

This plugin is intentionally **local-only**. Input capture, checking, processing, and buffering all happen on the machine that owns the input; nothing in the module is replicated and no RPCs are defined. If a processor needs to trigger networked gameplay (for example activating a replicated ability), that replication is the responsibility of the system the processor calls into — such as sigil.gas.

## See Also

- [sigil.gas](sigil-gas.md) — ability system layer; a natural consumer of tag-based input events.
- [sigil.combat](sigil-combat.md) — combat layer built on sigil.gas.
