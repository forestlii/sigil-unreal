[English](README.md) | [简体中文](README.zh-CN.md)

# Sigil — Unreal Engine Gameplay Plugin Suite

A set of modular, code-only gameplay framework plugins for **Unreal Engine 5.6**, designed to be dropped into a new project so you can start building gameplay instead of infrastructure.

## Plugins

| Plugin | What it provides |
|---|---|
| `GenericCombatSystem` | GAS-based multiplayer combat framework: ability sets, activation groups, tag relationships, game phases, input buffering, collision tracing, pooled projectiles, target lock-on, predictive montage playback |
| `GenericMovementSystem` | Data-driven movement & locomotion (Lyra-style layered AnimBP): multi-gait movement, rotation modes, turn-in-place, distance matching, orientation/stride warping |
| `GenericInventorySystem` | Fragment-based inventory framework: stacks, slots, equipment, pickups, drops, shops, crafting, save serialization; fully replicated (FastArray + push model), zero GAS coupling |
| `GenericGameSystem` | Four independent subsystems: SmartObject-driven interaction, camera mode stack, CommonUI extensions (layers/actions/modals), context-driven SFX/VFX |

All plugins are mutually independent — take only what you need.

## Status

⚠️ **Pre-refactor baseline.** This is the original codebase published as-is. It is being refactored into domain packages under the `sigil.*` namespace (`sigil.combat`, `sigil.movement`, `sigil.inventory`, …) with unified naming and cleanup. Expect breaking changes; legacy copyright headers in source files will be normalized during the refactor.

## Requirements

- Unreal Engine **5.6**
- C++ project (these are code-only plugins, `CanContainContent: false`)

## License

[MIT](LICENSE) © 2026 Likeon
