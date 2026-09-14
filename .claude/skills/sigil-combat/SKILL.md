---
name: sigil-combat
description: 在修改或排查 SigilCombat 的攻击请求与结果、CombatFlow、碰撞追踪、目标选择、战斗能力任务或武器动画连接时读取。
---

# Sigil 战斗机制

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilCombat/Source/SigilCombat/。

- Public/SigilCombatSystemComponent.h 与对应 Private 实现。
- Public/CombatFlow/SigilCombatFlow.h 与对应 Private 实现。
- 追踪任务读取 Public/Collision/SigilCollisionTraceInstance.h、
  Public/AbilitySystem/Tasks/SigilAbilityTask_CollisionTrace.h 及对应实现。
- 目标与武器任务分别读取
  Public/Targeting/SigilTargetingSystemComponent.h、
  Public/Weapon/SigilWeaponActor.h 及对应实现。
- 依赖来源：SigilCombat.Build.cs；本模块依赖 SigilGas。

## 已核实的接口约束

- CombatFlow.Initialize 解析 Owner 的 CombatComponent，
  并据此设置初始化状态。
- HandleAttackResult 在 CombatComponent 无效时返回；
  有效时记录最近结果，再调用有效的结果 Processor。
- HandlePreGameplayEffectSpecApply 与
  HandleGameplayEffectExecute 的默认实现为空扩展点，
  不能描述成已经实现了具体伤害计算。
- CombatFlow 头文件明确提醒：客户端 OnRep 初始化可能晚于
  FastArray 回调，初始化之前不能分发结果。

## 处理当前任务

从请求产生处追踪组件、Flow、结果容器与 Processor。
涉及复制时同时读取初始化和回调路径，区分本地处理与远端处理；
涉及碰撞时同时追踪任务启动、命中回调和停止。
能力通用语义读取 sigil-gas，装备到战斗的连接读取 sigil-arsenal。

## 范围

覆盖可复用战斗机制与连接点。
不定义游戏招式、伤害数值、敌人决策或库存策略。
