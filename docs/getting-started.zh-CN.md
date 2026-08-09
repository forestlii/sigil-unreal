[English](getting-started.md) | [简体中文](getting-started.zh-CN.md)

# 快速上手

本页介绍如何把 Sigil 插件套件装进 Unreal Engine 工程并完成编译。

## 前置要求

- **Unreal Engine 5.8**
- **C++ 工程**（插件为纯代码，纯蓝图工程无法编译）
- Visual Studio 2022（含 Unreal Engine 工作负载，Windows）

## 安装

1. 克隆仓库：

   ```
   git clone https://github.com/forestlii/sigil-unreal.git
   ```

2. 从 `source/` 把需要的插件文件夹拷进工程的 `Plugins/` 目录：

   ```
   MyProject/
     Plugins/
       SigilGas/
       SigilCombat/
       ...
   ```

   > **注意：**`SigilCombat` 依赖 `SigilGas`；其余包全部独立——用哪个拷哪个。

3. 在 `.uproject` 里启用插件，或在编辑器 **Edit > Plugins > Sigil** 分类下勾选：

   ```json
   "Plugins": [
     { "Name": "SigilGas", "Enabled": true },
     { "Name": "SigilCombat", "Enabled": true }
   ]
   ```

4. 重新生成工程文件并编译。各包依赖的引擎插件（EnhancedInput、GameplayAbilities、CommonUI、SmartObjects 等）会通过插件引用自动启用。

## 验证安装

打开编辑器，确认 **Edit > Plugins > Sigil** 分类下能看到插件。之后每个包各有自己的配置步骤——Settings 类、需要自建的 Tag 树、需要创建的资产，按各包指南的「前置要求」和「快速开始」来：

- [sigil.input](sigil-input.zh-CN.md) · [sigil.gas](sigil-gas.zh-CN.md) · [sigil.combat](sigil-combat.zh-CN.md) · [sigil.movement](sigil-movement.zh-CN.md) · [sigil.inventory](sigil-inventory.zh-CN.md) · [sigil.interaction](sigil-interaction.zh-CN.md) · [sigil.camera](sigil-camera.zh-CN.md) · [sigil.ui](sigil-ui.zh-CN.md) · [sigil.effects](sigil-effects.zh-CN.md)

## 宿主工程

仓库自带 `Host/`——一个把九个插件用目录 junction 全部接入的最小 C++ 工程，用于在没有完整游戏工程的情况下编译与冒烟验证。本地构建：

```bash
<EngineRoot>/Engine/Build/BatchFiles/Build.bat HostEditor Win64 Development -project="<repo>/Host/Host.uproject" -WaitMutex
```

## 包依赖图

```
sigil.combat ──> sigil.gas
（其余包完全独立）
```
