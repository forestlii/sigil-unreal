[English](README.md) | [简体中文](README.zh-CN.md)

# Sigil — Unreal Engine 玩法插件套件

一套模块化、纯代码的 **Unreal Engine 5.6** 玩法框架插件，目标是塞进新工程就能直接搭玩法，而不是先造基建。

## 插件列表

| 插件 | 提供什么 |
|---|---|
| `GenericCombatSystem` | 基于 GAS 的多人战斗框架：AbilitySet、激活互斥组、Tag 关系表、游戏阶段、输入缓冲、碰撞扫描、子弹对象池、目标锁定、可预测蒙太奇播放 |
| `GenericMovementSystem` | 数据驱动的移动与 locomotion（Lyra 风格分层动画蓝图）：多步态、旋转模式、原地转身、距离匹配、朝向/步幅 Warping |
| `GenericInventorySystem` | Fragment 组合式背包框架：堆叠、槽位、装备、拾取、掉落、商店、合成、存档序列化；完整网络复制（FastArray + Push Model）、零 GAS 耦合 |
| `GenericGameSystem` | 四个独立子系统：SmartObject 驱动的交互、相机模式栈、CommonUI 扩展（层/动作/弹窗）、Context 驱动的音效/特效 |

所有插件互相零依赖——按需取用。

## 状态

⚠️ **重构前基线。** 这是原始代码库的原样发布，正在重构为 `sigil.*` 命名空间下的领域包（`sigil.combat`、`sigil.movement`、`sigil.inventory` 等），统一命名并做清理。会有破坏性变更；源码里的旧版权头也会在重构中统一规范。

## 环境要求

- Unreal Engine **5.6**
- C++ 工程（纯代码插件，`CanContainContent: false`）

## 协议

[MIT](LICENSE) © 2026 Likeon
