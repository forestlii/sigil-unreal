[English](README.md) | [简体中文](README.zh-CN.md)

# Sigil — Unreal Engine Gameplay Plugin Suite

A suite of modular, code-only gameplay framework plugins for **Unreal Engine 5.8**, designed to be dropped into a new project so you can start building gameplay instead of infrastructure.

## Packages

| Package | Plugin | What it provides |
|---|---|---|
| `sigil.input` | `SigilInput` | Tag-driven input abstraction over EnhancedInput with input buffering and checker/processor pipeline |
| `sigil.gas` | `SigilGas` | GAS infrastructure: ability sets, activation groups, tag relationships, game phases, common attribute sets (Health/Stamina/Mana), ability/async task toolbox |
| `sigil.combat` | `SigilCombat` | GAS-based multiplayer combat: combat flow pipeline, collision tracing, pooled projectiles, target lock-on, predictive montage playback |
| `sigil.movement` | `SigilMovement` | Data-driven movement & locomotion (Lyra-style layered AnimBP): multi-gait, rotation modes, turn-in-place, distance matching, warping |
| `sigil.inventory` | `SigilInventory` | Fragment-based inventory: stacks, slots, equipment, pickups, drops, shops, crafting, save serialization; fully replicated, zero GAS coupling |
| `sigil.interaction` | `SigilInteraction` | SmartObject-driven interaction bridged with GameplayBehaviors and GAS |
| `sigil.camera` | `SigilCamera` | Camera mode stack driving spring-arm cameras with penetration avoidance |
| `sigil.ui` | `SigilUI` | CommonUI extensions: layered game UI, data-driven UI actions, modals, extension points, widget factories |
| `sigil.effects` | `SigilEffects` | Context-driven SFX/VFX selected by gameplay tag queries and physical surfaces |

All packages are mutually independent — the only cross-package dependency is `sigil.combat → sigil.gas`. Take only what you need.

All gameplay tags live under the `Sigil.*` namespace; C++ types use the unified `Sigil` prefix (e.g. `USigilAbilitySet`, `USigilItemDefinition`).

## Documentation

Full documentation lives in [docs/](docs/README.md) — a [Getting Started](docs/getting-started.md) guide plus a per-package guide (overview, prerequisites, quick start, key types, networking) in the style of the official Unreal Engine documentation.

## Getting started

1. Copy the plugin folders from `source/` into your project's `Plugins/` directory (or the whole repo and reference them).
2. Enable the plugins you need in your `.uproject`.
3. Follow each package's *Prerequisites* and *Quick Start* in [docs/](docs/README.md).
4. See `Host/` for a minimal host project used to compile and smoke-test the suite.

## Requirements

- Unreal Engine **5.8**
- C++ project (these are code-only plugins, `CanContainContent: false`)

## Status

Version **0.1.0** — freshly refactored from an internal codebase (dead code purged, defects fixed, renamed from legacy `Generic*` naming). APIs may still shift before 1.0.

## License

[MIT](LICENSE) © 2026 Likeon
