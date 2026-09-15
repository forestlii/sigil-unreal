---
name: sigil-interaction
description: 在接入、修改或排查 SigilInteraction 的候选搜索、交互选项、SmartObject 占用、GameplayBehavior 或 GAS 交互任务时读取。
---

# Sigil 交互机制

源码核对基线：9ffca225f3313e93932cadb1ecd56ec7e7fd7ea2。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilInteraction/Source/SigilInteraction/。

- Public/Interaction/SigilInteractionSystemComponent.h 与对应 Private 实现。
- Public/Interaction/Tasks/SigilAbilityTask_UseSmartObjectWithGameplayBehavior.h
  与对应 Private 实现。
- Public/Interaction/Behaviors/SigilGameplayBehavior_InteractionWithAbility.h
  与对应 Private 实现。
- 依赖来源：SigilInteraction.Build.cs；
  使用引擎 GameplayAbilities，不直接依赖 SigilGas。

## 已核实的接口约束

- UseSmartObjectWithGameplayBehavior.Activate 检查有效 ClaimHandle、
  Pawn、SmartObjectSubsystem 和仍有效的已认领对象。
- 激活时注册 Slot 失效回调；StartInteraction 将 Slot 标为占用，
  获取配置的 GameplayBehavior 并触发行为。
- 仅在行为仍运行时注册完成回调；
  源码明确考虑行为同步结束的情况。
- OnDestroy 对有效 ClaimHandle 释放 Slot、
  注销失效回调并使句柄失效。
- Slot 失效路径会处理未完成行为的中止，再结束任务。

## 处理当前任务

分别追踪候选发现、选项过滤、认领、占用、行为执行和释放。
候选可见不等于认领或执行成功。
同步结束、异步完成、中止及失败要分别核对回调与最终任务结果；
不要把这些路径合并成未经验证的成功保证。
游戏侧能力如何使用结果，由消费项目的调用点说明。

## 范围

覆盖通用交互和 SmartObject/GameplayBehavior 连接。
对话、任务推进及具体可交互物规则留在游戏仓。
