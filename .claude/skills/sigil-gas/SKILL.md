---
name: sigil-gas
description: 在接入、修改或排查 SigilGas 的 ASC 初始化、能力授予撤销、输入激活、属性与 Cost、AbilitySource 活跃门或游戏阶段机制时读取。
---

# Sigil GAS 框架

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilGas/Source/SigilGas/。

- Public/SigilAbilitySystemComponent.h 与对应 Private 实现。
- Public/Abilities/SigilAbilitySet.h 与 Private/Abilities/SigilAbilitySet.cpp。
- Public/Abilities/SigilAbilitySourceInterface.h。
- Private/Abilities/SigilGameplayAbility.cpp。
- 阶段任务再读 Public/Phases/SigilGamePhaseSubsystem.h 及对应实现。

## 已核实的接口约束

- AbilitySet 的 GiveToAbilitySystem 和 GrantedHandles 的
  TakeFromAbilitySystem 都检查 ASC Owner 的 authority。
- 授予能力时 AbilitySpec.SourceObject 来自调用方传入的 SourceObject。
- TakeFromAbilitySystem 按保存的句柄撤销能力、效果及属性集，
  然后清空句柄记录。
- GameplayAbility 的激活检查在 bRequireSourceObjectActive 开启时，
  调用 AbilitySource 活跃判定，并在拒绝时处理失败标签。

## 处理当前任务

1. 初始化问题从 InitAbilityActorInfo 的调用方、Owner/Avatar
   以及输入或被动能力的后续处理追踪，不能只看 ASC 对象是否存在。
2. 授予/撤销问题记录授予者、SourceObject、GrantedHandles 持有者
   和撤销时机；多来源能力不能只按能力类识别归属。
3. Cost、属性或阶段问题只追加读取对应实现及调用点；
   将授权角色、预测路径和最终状态修改分别说明。
4. 涉及装备来源时转到 sigil-arsenal；
   涉及攻击结果处理时转到 sigil-combat。

## 范围

覆盖通用能力基础设施。
具体攻击、武器配置、库存和游戏专有能力由对应框架或消费项目负责；
不向本模块引入它们的业务策略。
