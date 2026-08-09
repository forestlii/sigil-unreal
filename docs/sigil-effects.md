[English](sigil-effects.md) | [简体中文](sigil-effects.zh-CN.md)

# sigil.effects

**Plugin:** `SigilEffects` · **Modules:** `SigilEffects` (Runtime) · **Depends on:** Niagara (engine plugin), PhysicsCore, Gameplay Tags

sigil.effects is a context-driven feedback system: instead of hard-referencing a specific sound or particle in an anim notify, gameplay emits an *effect tag* plus *context tags*, and a library asset resolves them to the sounds, Niagara systems, or Cascade particles to play. Contexts can come from the caller, from per-actor defaults, from a gameplay-tag provider, and from the physical surface under a trace — so the same "footstep" notify plays different effects on grass, concrete, or water.

## Overview

### Effect resolution: tag + tag queries

A `USigilContextEffectsLibrary` data asset holds an array of `FSigilContextEffects` entries. Each entry has:

- `EffectTag` — matched **exactly** against the requested effect tag.
- `SourceTagQuery` — a `FGameplayTagQuery` evaluated against the aggregated source context. **An entry whose `SourceTagQuery` is empty is discarded at load time**, so it is not possible to author an unconditional "always play" entry — every entry needs at least one source-context condition.
- `TargetTagQuery` — optional; when empty, target context is ignored.
- `Effects` — soft object paths restricted to `USoundBase`, `UNiagaraSystem`, and `UParticleSystem` assets.

An older per-entry `Context` tag container is deprecated; on save it is automatically converted into an equivalent "has all tags" `SourceTagQuery`.

`GetEffects(Effect, SourceContext, TargetContext, ...)` returns every matching entry's assets. Matching requires a valid effect tag **and a non-empty source context**.

### Loading model (honest note)

`USigilContextEffectsLibrary::LoadEffects` resolves all soft references **synchronously** (`TryLoad`); the source contains explicit `TODO Add Async Loading for Libraries` markers. Libraries are loaded when an actor registers them (see below), which usually happens at `BeginPlay`. Large libraries can therefore hitch on registration, and the first playback after a late registration may stall.

### Registration granularity: the actor

`USigilContextEffectsSubsystem` (a `UWorldSubsystem`) keeps a map from **actor** to its set of loaded libraries (`LoadAndAddContextEffectsLibraries(OwningActor, Libraries)` / `UnloadAndRemoveContextEffectsLibraries(OwningActor)`). Spawning goes through:

- `SpawnContextEffectsExt(SpawningActor, Input, Output)` — resolves against all libraries registered **for that actor**;
- `SpawnContextEffects(WorldContextObject, EffectsLibrary, Input, Output)` — resolves against one explicit library;
- `GetContextFromSurfaceType(PhysicalSurface, OutContext)` — looks up the surface→tag map from settings.

The input/output payloads are `FSigilSpawnContextEffectsInput` (effect tag, attach mode + bone/component/offsets or world location/rotation, source/target contexts, `SourceContextType` Merge/Override, VFX scale, audio volume/pitch, optional `HitResult`) and `FSigilSpawnContextEffectsOutput` (the spawned audio/Niagara/particle components).

### The component: context aggregation

`USigilContextEffectComponent` implements `ISigilContextEffectsInterface` and is the standard receiver on an actor. On `BeginPlay` it registers `DefaultContextEffectsLibraries` with the subsystem for its owner and seeds `CurrentContexts` from `DefaultEffectContexts`. When `PlayContextEffectsWithInput` is called it aggregates the **source context from three places** (when `SourceContextType == Merge`):

1. the caller's `Input.SourceContext`;
2. the component's `CurrentContexts` (defaults, updateable via `UpdateEffectContexts`);
3. tags from an optional `GameplayTagsProvider` — any object implementing `IGameplayTagAssetInterface`; with `bAutoSetupTagsProvider` the owner is used automatically if it implements the interface.

If `bConvertPhysicalSurfaceToContext` is set and the input carries a `HitResult`, the hit physical material's surface type is translated through `USigilContextEffectsSettings::SurfaceTypeToContextMap` and injected into the source context; `FallbackPhysicalSurface` is used when no mapping or physical material is available.

### AnimNotify trigger chain

`USigilAnimNotify_ContextEffects` (display name **Play Context Effects**) is the main gameplay trigger. On `Notify` it: computes the spawn transform (attached to a socket, or via an optional instanced `USigilContextEffectsSpawnParametersProvider` when not attached), optionally performs a line trace (`bPerformTrace` + `FSigilContextEffectAnimNotifyTraceSettings`, with `bReturnPhysicalMaterial` enabled) to feed surface conversion, then finds the mesh owner **and all of its components** that implement `ISigilContextEffectsInterface` and calls `PlayContextEffectsWithInput` on each. With `USigilContextEffectComponent` on the actor, that completes the chain: notify → component → subsystem → libraries → spawned effects.

An editor preview path exists: `USigilContextEffectsSettings::bPreviewInEditor` plus a `USigilContextEffectsPreviewSetting` asset let the notify play effects in animation editors with a chosen preview surface type.

### Gameplay tags (honest note)

The plugin natively declares a single tag: `GES` (comment: "Generic Effects System"). The component's `FallbackPhysicalSurface` property filters its picker to tags under **`Sigil.Effects.SurfaceType`**, but the plugin does **not** declare any tags under that root — your project must create the `Sigil.Effects.SurfaceType.*` tag tree (and all effect/context tags) itself.

## Prerequisites

- [ ] **Niagara enabled** (declared by the `.uplugin`).
- [ ] **A project tag vocabulary**: effect tags (e.g. footstep/impact events) and context tags, including a `Sigil.Effects.SurfaceType.*` tree if you use surface conversion. The plugin declares none of these.
- [ ] **Physical materials assigned** to your environment (and `bReturnPhysicalMaterial`-friendly collision) if you want surface-driven variation, plus the surface→tag map filled in settings.
- [ ] **A `USigilContextEffectsLibrary` asset** with at least one entry — remember every entry needs a non-empty `SourceTagQuery`.
- [ ] **A receiver on each effect-playing actor** — `USigilContextEffectComponent` (or your own `ISigilContextEffectsInterface` implementer).

## Quick Start

1. **Create tags.** Define your effect tags (e.g. an anim-event tag per footstep/impact) and context tags (e.g. surface types under `Sigil.Effects.SurfaceType.*`).
2. **Map surfaces.** In **Project Settings** (the `USigilContextEffectsSettings` section, config `Game`), fill `SurfaceTypeToContextMap` from `EPhysicalSurface` values to your surface tags.
3. **Author a library.** Create a `USigilContextEffectsLibrary`; add one `FSigilContextEffects` entry per (effect, context) combination: set `EffectTag`, a `SourceTagQuery` (e.g. *any tags match: grass surface*), and the sound/Niagara assets in `Effects`.
4. **Add the component.** On your character, add `USigilContextEffectComponent`; set `DefaultContextEffectsLibraries` to your library, `DefaultEffectContexts` to any always-on context tags, and `FallbackPhysicalSurface`.
5. **Place notifies.** In footstep/impact animations, add the **Play Context Effects** notify; set `Effect`, attachment (`bAttached` + `SocketName`) or offsets, and enable `bPerformTrace` with a downward `EndTraceLocationOffset` for surface detection.
6. **Play in game.** The notify calls into the component, contexts are aggregated and surface tags injected, and matching sounds/particles spawn. For non-animation triggers, call `PlayContextEffectsWithInput` on the component (or the subsystem's spawn functions) directly.

## Key Types

| Type | Description |
| --- | --- |
| `USigilContextEffectsLibrary` | Data asset of `FSigilContextEffects` entries; synchronous `LoadEffects`; per-entry tag queries select assets. |
| `FSigilContextEffects` | One entry: `EffectTag` (exact match), `SourceTagQuery` (required non-empty), `TargetTagQuery` (optional), `Effects` (Sound/Niagara/Cascade soft paths). |
| `USigilContextEffectsSubsystem` | World subsystem: per-actor library registration and effect spawning (`SpawnContextEffects`, `SpawnContextEffectsExt`, `GetContextFromSurfaceType`). |
| `USigilContextEffectsSettings` | Developer settings: `SurfaceTypeToContextMap`, editor preview toggle + `PreviewSetting`. |
| `USigilContextEffectComponent` | Actor-side receiver: default libraries/contexts, three-source context aggregation, physical-surface injection, tag provider hookup. |
| `ISigilContextEffectsInterface` | Interface with `PlayContextEffectsWithInput`; implement it to receive notify-driven playback. |
| `USigilAnimNotify_ContextEffects` | "Play Context Effects" anim notify: attachment/trace settings, VFX/audio settings, editor preview. |
| `USigilContextEffectsSpawnParametersProvider` | Instanced, Blueprintable provider for custom spawn location/rotation when the notify is not attached. |
| `FSigilSpawnContextEffectsInput` / `FSigilSpawnContextEffectsOutput` | Spawn request payload (contexts, attachment, VFX/audio params, hit result) and the resulting spawned components. |
| `USigilContextEffectsPreviewSetting` | Editor-only preview data (e.g. preview physical surface). |

## Configuration

- **Project Settings → `USigilContextEffectsSettings`** (config `Game`): `SurfaceTypeToContextMap`; `bPreviewInEditor` + `PreviewSetting` (editor only).
- **Per library** — the entry list; entries with an empty `SourceTagQuery` are silently dropped at load.
- **Per component** — `DefaultContextEffectsLibraries`, `DefaultEffectContexts`, `bConvertPhysicalSurfaceToContext`, `FallbackPhysicalSurface`, `bAutoSetupTagsProvider` / `SetGameplayTagsProvider`. `UpdateLibraries` / `UpdateEffectContexts` swap them at runtime.
- **Per notify** — effect tag, attachment or spawn-parameters provider, trace settings, VFX scale, audio volume/pitch.

## Networking

Entirely local and cosmetic. Nothing replicates: effects spawn on whichever machine runs the trigger (anim notifies fire wherever the animation plays, including on each client). If you need server-authoritative effect events, replicate the trigger through your own gameplay systems and let each client play its effects locally.

## See Also

- [sigil.interaction](sigil-interaction.md) — interaction animations are a natural source of context-effect notifies.
- [sigil.input](sigil-input.md) — input layer for the gameplay that triggers these animations.
