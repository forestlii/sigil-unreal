---
name: sigil-arsenal
description: 在修改或排查 SigilArsenal 的武器装备与能力授予、活跃来源门、弹药整数属性 Cost、半自动连发节奏或武器动画连接时读取。
---

# Sigil 武器装备连接

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilArsenal/Source/SigilArsenal/。

- Public/Fragments/SigilItemFragment_WeaponLoadout.h。
- Private/Equipping/SigilWeaponEquipmentInstance.cpp。
- Private/Abilities/SigilAbilityCost_ItemIntegerAttribute.cpp。
- Private/Abilities/SigilGameplayAbility_FireCadence.cpp。
- 依赖来源：SigilArsenal.Build.cs；
  连接 SigilInventory、SigilGas 与 SigilCombat。

## 已核实的接口约束

- WeaponLoadout 区分 WhileEquipped 与 WhileActive 授予策略。
- WeaponEquipmentInstance 在 authority 上授予 AbilitySet，
  并把装备实例自身作为 SourceObject，保存句柄用于撤销。
- 整数属性 Cost 经能力 SourceObject 找到装备实例与源物品；
  CheckCost 检查属性、正数量和余额，ApplyCost 在 authority 上
  再检查余额后扣除。
- FireCadence 使用 InstancedPerActor、LocalOnly，
  并要求来源处于活跃状态；它是本地节奏控制。
- 单发能力按能力类和 SourceWeapon 查找；
  通过 BatchRPCTryActivateAbility(handle, false) 激活，
  单发能力自己负责 Commit 和 End。
- FireCadence 使用激活序号处理回调重入及延迟 End；
  结束时清理 Timer 和来源状态委托。

## 处理当前任务

先核对同类多武器的 SourceObject 隔离，再追踪授予策略、
活跃变化、Cost、单发激活和节奏结束。
修改 Timer 或 End 路径时同时检查旧激活回调与新激活的隔离。
LocalOnly 节奏不能作为服务器射速裁决已经成立的证据。

## 范围

覆盖装备与三类通用机制的连接。
基础库存、通用 GAS 和战斗结果分别读取关联 Skill；
不定义游戏枪械数值，也不扩展多 Mesh 动画方案。
