[English](README.md) | [简体中文](README.zh-CN.md)

# Sigil Documentation

Sigil is a suite of modular, code-only gameplay framework plugins for **Unreal Engine 5.8**. Each package solves one domain and can be adopted independently — the only cross-package dependency is `sigil.combat → sigil.gas`.

## Getting Started

- [Getting Started](getting-started.md) — install the plugins, enable them in your project, and compile.

## Packages

| Package | Guide | Summary |
|---|---|---|
| sigil.input | [sigil-input.md](sigil-input.md) | Tag-driven input abstraction over EnhancedInput with input buffering |
| sigil.gas | [sigil-gas.md](sigil-gas.md) | Gameplay Ability System infrastructure: ability sets, activation groups, tag relationships, game phases, attribute sets |
| sigil.combat | [sigil-combat.md](sigil-combat.md) | GAS-based multiplayer combat: combat flow, collision tracing, projectiles, lock-on, predictive montages |
| sigil.movement | [sigil-movement.md](sigil-movement.md) | Data-driven movement & locomotion with a Lyra-style layered AnimBP workflow |
| sigil.inventory | [sigil-inventory.md](sigil-inventory.md) | Fragment-based inventory: stacks, slots, equipment, shops, crafting, save serialization |
| sigil.interaction | [sigil-interaction.md](sigil-interaction.md) | SmartObject-driven interaction bridged with GameplayBehaviors and GAS |
| sigil.camera | [sigil-camera.md](sigil-camera.md) | Camera mode stack driving spring-arm cameras |
| sigil.ui | [sigil-ui.md](sigil-ui.md) | CommonUI extensions: UI layers, data-driven actions, modals, extension points |
| sigil.effects | [sigil-effects.md](sigil-effects.md) | Context-driven SFX/VFX selected by tag queries and physical surfaces |

## Conventions

- **C++ types** use the unified `Sigil` prefix: `USigilAbilitySet`, `USigilItemDefinition`, `FSigilAttackResult`.
- **Gameplay tags** live under the `Sigil.*` namespace. Tags declared natively by the plugins register automatically; tag trees that your project must define itself are listed in each package's *Prerequisites* section.
- **Plugins are code-only** (`CanContainContent: false`): all assets referenced by the docs (AnimBPs, DataTables, DataAssets, widgets) are created in your project.

## Source Layout

```
source/
  SigilInput/  SigilGas/  SigilCombat/  SigilMovement/  SigilInventory/
  SigilInteraction/  SigilCamera/  SigilUI/  SigilEffects/
Host/            # minimal host project used to compile & smoke-test the suite
```
