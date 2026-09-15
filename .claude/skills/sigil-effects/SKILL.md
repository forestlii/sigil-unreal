---
name: sigil-effects
description: 在接入、修改或排查 SigilEffects 的上下文标签选效、物理表面映射、效果库加载或声音、Niagara、粒子生成时读取。
---

# Sigil 上下文效果

源码核对基线：9ffca225f3313e93932cadb1ecd56ec7e7fd7ea2。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilEffects/Source/SigilEffects/。

- Public/Feedback/SigilContextEffectsSubsystem.h 与对应 Private 实现。
- Public/Feedback/SigilContextEffectsLibrary.h 与对应 Private 实现。
- 动画通知入口：Public/Feedback/SigilAnimNotify_ContextEffects.h。

## 已核实的接口约束

- ContextEffectsLibrary.GetEffects 要求库已加载；
  EffectTag 使用 MatchesTagExact。
- SourceContext 使用 SourceTagQuery 匹配，并额外比较两者是否为空；
  TargetTagQuery 为空时不限制 TargetContext。
- LoadEffectsInternal 筛选有效 EffectTag 和非空 SourceTagQuery，
  并通过 TryLoad 加载效果资源。
- Subsystem 的库加载路径包含 LoadSynchronous；
  当前实现不能描述成完整异步加载。
- Subsystem 提供 SurfaceType 到 Context 的映射，
  并通过 ActorEffectsMap 维护 Actor 关联的效果库。
- UnloadAndRemoveContextEffectsLibraries 移除 Actor 的库映射；
  这不能证明所有资源或已生成效果实例立即释放。

## 处理当前任务

依次核对效果标签、源/目标上下文、库加载状态和选出的效果，
再追踪声音或粒子生成参数。
区分效果未匹配、资源未加载和生成后不可见；
生命周期任务继续检查调用方的注册、移除及生成实例持有方式。

## 范围

覆盖通用效果选择与生成连接。
不编制美术资源，不定义伤害计算或游戏专有反馈策略。
