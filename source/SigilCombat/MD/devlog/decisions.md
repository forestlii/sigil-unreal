# SigilCombat 决策日志

> 本文件只记录 `SigilCombat` 插件内部的审查修复取舍，不记录消费项目玩法、资产或运行时结论。
>
> 最后更新：2026-08-30。

### [2026-08-30] CBT-01 以未消费值副本分发 Attack Result

- 阶段: 审查批次 A 修复
- 面临的选择: 直接把已经消费的存储项传给 Flow，或保持存储状态先落盘并只向回调交付未消费的值副本。
- 定了什么: `AddEntry` 与后续 `ConsumeEntry` 都先写入 `bConsumed` 并标脏存储项，再向 Flow 传入 `bConsumed = false` 的局部副本；无 Flow 的条目在 Flow 就绪后仍只消费一次。
- 否掉了什么 + 为什么: 否掉改变 FastArray 复制字段、序列化布局或 Flow 的公开合同；本修复只校正回调看到的值，不改变持久化消费语义。
- 复用层🔑: ② 引擎相关
- 来源: Finding `CBT-01`；Sigil commit `e3870be`；`SigilCombat.AttackResult` Automation（2/2 通过，见审查批次 A Task 1 报告）。
