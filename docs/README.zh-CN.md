[English](README.md) | [简体中文](README.zh-CN.md)

# Sigil 文档

Sigil 是一套模块化、纯代码的 **Unreal Engine 5.8** 玩法框架插件。每个包只解决一个领域、可独立取用——唯一的包间依赖是 `sigil.combat → sigil.gas`。

## 入门

- [快速上手](getting-started.zh-CN.md) —— 安装插件、在工程中启用并编译。
- [变更日志](CHANGELOG.zh-CN.md) —— 各版本公开 API 变化与迁移说明。

## 包指南

| 包 | 指南 | 一句话 |
|---|---|---|
| sigil.input | [sigil-input.zh-CN.md](sigil-input.zh-CN.md) | 基于 EnhancedInput 的 Tag 化输入抽象，带输入缓冲 |
| sigil.gas | [sigil-gas.zh-CN.md](sigil-gas.zh-CN.md) | GAS 基建：AbilitySet、激活互斥组、Tag 关系表、游戏阶段、属性集 |
| sigil.combat | [sigil-combat.zh-CN.md](sigil-combat.zh-CN.md) | 基于 GAS 的多人战斗：受击流水线、碰撞扫描、子弹、锁定、可预测蒙太奇 |
| sigil.movement | [sigil-movement.zh-CN.md](sigil-movement.zh-CN.md) | 数据驱动移动与 locomotion，Lyra 风格分层动画蓝图工作流 |
| sigil.inventory | [sigil-inventory.zh-CN.md](sigil-inventory.zh-CN.md) | Fragment 组合式背包：堆叠、槽位、装备、商店、合成、存档 |
| sigil.interaction | [sigil-interaction.zh-CN.md](sigil-interaction.zh-CN.md) | SmartObject 驱动的交互，桥接 GameplayBehaviors 与 GAS |
| sigil.camera | [sigil-camera.zh-CN.md](sigil-camera.zh-CN.md) | 驱动弹簧臂相机的相机模式栈 |
| sigil.ui | [sigil-ui.zh-CN.md](sigil-ui.zh-CN.md) | CommonUI 扩展：UI 分层、数据驱动动作、弹窗、扩展点 |
| sigil.effects | [sigil-effects.zh-CN.md](sigil-effects.zh-CN.md) | Context 驱动的音效/特效，按 Tag 查询与物理表面选择 |

## 约定

- **C++ 类型**统一 `Sigil` 前缀：`USigilAbilitySet`、`USigilItemDefinition`、`FSigilAttackResult`。
- **GameplayTag** 全部在 `Sigil.*` 命名空间下。插件原生声明的 Tag 自动注册；需要项目自建的 Tag 树在各包文档的「前置要求」一节列出。
- **插件均为纯代码**（`CanContainContent: false`）：文档中提到的资产（动画蓝图、DataTable、DataAsset、控件）都在你的工程里创建。

## 源码布局

```
source/
  SigilInput/  SigilGas/  SigilCombat/  SigilMovement/  SigilInventory/
  SigilInteraction/  SigilCamera/  SigilUI/  SigilEffects/
Host/            # 用于编译与冒烟验证整套插件的最小宿主工程
```
