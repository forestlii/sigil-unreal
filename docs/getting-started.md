[English](getting-started.md) | [简体中文](getting-started.zh-CN.md)

# Getting Started

This page walks you through installing the Sigil plugin suite into an Unreal Engine project and compiling it.

## Prerequisites

- **Unreal Engine 5.8**
- A **C++ project** (the plugins are code-only; a Blueprint-only project cannot compile them)
- Visual Studio 2022 with the Unreal Engine workload (Windows)

## Installation

1. Clone the repository:

   ```
   git clone https://github.com/forestlii/sigil-unreal.git
   ```

2. Copy the plugin folders you need from `source/` into your project's `Plugins/` directory:

   ```
   MyProject/
     Plugins/
       SigilGas/
       SigilCombat/
       ...
   ```

   > **Note:** `SigilCombat` requires `SigilGas`. All other packages are independent — copy only what you use.

3. Enable the plugins in your `.uproject` file, or in the editor via **Edit > Plugins > Sigil**:

   ```json
   "Plugins": [
     { "Name": "SigilGas", "Enabled": true },
     { "Name": "SigilCombat", "Enabled": true }
   ]
   ```

4. Regenerate project files and build your project. Engine plugins the packages depend on (EnhancedInput, GameplayAbilities, CommonUI, SmartObjects, and so on) are enabled automatically through plugin references.

## Verify the Installation

Open the editor and confirm the plugins are listed under **Edit > Plugins > Sigil**. Each package then has its own setup steps — settings classes to configure, tag trees to define, and assets to create. Follow the *Prerequisites* and *Quick Start* sections of each package guide:

- [sigil.input](sigil-input.md) · [sigil.gas](sigil-gas.md) · [sigil.combat](sigil-combat.md) · [sigil.movement](sigil-movement.md) · [sigil.inventory](sigil-inventory.md) · [sigil.interaction](sigil-interaction.md) · [sigil.camera](sigil-camera.md) · [sigil.ui](sigil-ui.md) · [sigil.effects](sigil-effects.md)

## The Host Project

The repository ships with `Host/`, a minimal C++ project that links all nine plugins via directory junctions. It is used to compile and smoke-test the suite without a full game project. To build it locally:

```bash
<EngineRoot>/Engine/Build/BatchFiles/Build.bat HostEditor Win64 Development -project="<repo>/Host/Host.uproject" -WaitMutex
```

## Package Dependency Graph

```
sigil.combat ──> sigil.gas
(all other packages are fully independent)
```
