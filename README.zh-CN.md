[English](README.md) | [简体中文](README.zh-CN.md)

# Sigil — Unreal Engine 玩法插件套件

一套模块化、纯代码的 **Unreal Engine 5.8** 玩法框架插件，目标是塞进新工程就能直接搭玩法，而不是先造基建。

## 包列表

| 包 | 插件 | 提供什么 |
|---|---|---|
| `sigil.input` | `SigilInput` | 基于 EnhancedInput 的 Tag 化输入抽象：输入缓冲、准入判定/处理器管线 |
| `sigil.gas` | `SigilGas` | GAS 基建：AbilitySet、激活互斥组、Tag 关系表、游戏阶段、通用属性集（生命/耐力/法力）、Ability/Async Task 工具箱 |
| `sigil.combat` | `SigilCombat` | 基于 GAS 的多人战斗：受击流水线、碰撞扫描、子弹对象池、目标锁定、可预测蒙太奇播放 |
| `sigil.movement` | `SigilMovement` | 数据驱动移动与 locomotion（Lyra 风格分层动画蓝图）：多步态、旋转模式、原地转身、距离匹配、Warping |
| `sigil.inventory` | `SigilInventory` | Fragment 组合式背包：堆叠、槽位、装备、拾取、掉落、商店、合成、存档序列化；完整网络复制、零 GAS 耦合 |
| `sigil.interaction` | `SigilInteraction` | SmartObject 驱动的交互，桥接 GameplayBehaviors 与 GAS |
| `sigil.camera` | `SigilCamera` | 相机模式栈，驱动弹簧臂相机并带穿模规避 |
| `sigil.ui` | `SigilUI` | CommonUI 扩展：分层游戏 UI、数据驱动 UI 动作、弹窗、扩展点、控件工厂 |
| `sigil.effects` | `SigilEffects` | Context 驱动的音效/特效，按 Tag 查询与物理表面选择 |

所有包互相独立——唯一的包间依赖是 `sigil.combat → sigil.gas`。按需取用。

所有 GameplayTag 收拢在 `Sigil.*` 命名空间下；C++ 类型统一 `Sigil` 前缀（如 `USigilAbilitySet`、`USigilItemDefinition`）。

## 文档

完整文档在 [docs/](docs/README.zh-CN.md)：[快速上手](docs/getting-started.zh-CN.md) + 每个包一篇指南（概览、前置要求、快速开始、关键类型、网络），仿 Unreal 官方文档风格，中英双语。

## 快速开始

1. 把 `source/` 下需要的插件文件夹拷进你工程的 `Plugins/` 目录。
2. 在 `.uproject` 里启用对应插件。
3. 按 [docs/](docs/README.zh-CN.md) 里各包的「前置要求」与「快速开始」配置。
4. `Host/` 是用于编译与冒烟验证整套插件的最小宿主工程，可作参考。

## 环境要求

- Unreal Engine **5.8**
- C++ 工程（纯代码插件，`CanContainContent: false`）

## 状态

版本 **0.1.0** —— 刚从内部代码库重构而来（清除死代码、修复缺陷、从旧 `Generic*` 命名彻底更名）。1.0 之前 API 仍可能调整。

## 协议

[MIT](LICENSE) © 2026 Likeon
