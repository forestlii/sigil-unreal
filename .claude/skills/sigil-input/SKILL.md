---
name: sigil-input
description: 在接入、修改或排查 SigilInput 的输入配置、Checker/Processor 路由、输入缓冲，以及 Pawn/Controller 输入建立与清理问题时读取。
---

# Sigil 输入框架

源码核对基线：9ffca225f3313e93932cadb1ecd56ec7e7fd7ea2。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilInput/Source/SigilInput/。

- Public/SigilInputSystemComponent.h 与对应 Private 实现：
  Owner、MappingContext、配置、输入建立/清理及缓冲入口。
- Public/SigilInputControlSetup.h 与对应 Private 实现：
  Checker 判定和 Processor 筛选。

## 已核实的接口约束

- InputSystemComponent 的头文件要求 Owner 为 Pawn 或 PlayerController。
- ControlSetup 的 InternalCheckInput 遍历有效 Checker；
  任一检查失败即拒绝，没有 Checker 时返回允许。
- FilterInputProcessors 使用 HasTagExact 与 TriggerEvents.Contains；
  不能假定父标签自动匹配子标签。
- InputSystemComponent 在 Pawn 重启或 Controller 改变时执行清理再建立。
  CleanupInputComponent 包含清理通知、移除 MappingContext、
  清理 InputActionValue 绑定及释放组件引用。

## 处理当前任务

1. 从实际 Owner、InputMappingContext、InputConfig 和 ControlSetups
   追踪到当前输入的 Checker 与 Processor，先区分未绑定、被过滤和未处理。
2. 缓冲任务继续读取组件中对应窗口的开启、消费和结束实现；
   不把枚举名称当作完整优先级或时序契约。
3. 生命周期修改同时检查建立与清理路径；
   不把清理 Value 绑定表述为已经清理所有类型的事件绑定。
4. 若 Processor 调用 GAS，只追加读取对应能力入口和 sigil-gas；
   不在这里复制 GAS 激活规则。

## 范围

覆盖通用输入配置、路由和生命周期。
游戏按键方案、角色动作含义及 UI 输入策略由消费项目或相关 Skill 负责。
