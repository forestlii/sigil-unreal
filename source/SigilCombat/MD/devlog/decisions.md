# SigilCombat 决策日志

> 本文件只记录 `SigilCombat` 插件内部的审查修复取舍，不记录消费项目玩法、资产或运行时结论。
>
> 最后更新：2026-08-30。

### [2026-08-30] CBT-01 以未消费值副本分发 Attack Result

- 阶段: 审查批次 A 修复
- 面临的选择: 直接把已经消费的存储项传给 Flow，或先固定 FastArray 存储状态并只向回调交付未消费的值副本。
- 定了什么: `AddEntry` 在回调前把存储项设为已消费并调用 `MarkItemDirty`；客户端后续 `ConsumeEntry` 也会先把本地存储项设为已消费，但不会把该客户端消费状态标脏复制。两条路径都向 Flow 传入 `bConsumed = false` 的局部副本；无 Flow 的条目在 Flow 就绪后仍只消费一次，处理器重入 `ConsumePendingEntries` 也不会重复处理。
- 否掉了什么 + 为什么: 否掉改变 FastArray 复制字段、序列化布局或 Flow 的公开合同；本修复只校正回调看到的值，不改变持久化消费语义。
- 复用层🔑: ② 引擎相关
- 来源: Finding `CBT-01`；Sigil commit `e3870be` 及最终整分支复审修正；`SigilCombat.AttackResult` Automation（3/3 通过）。

### [2026-08-30] 审查批次 A 最终验证

- 最终证据: `HostEditor Win64 Development` 构建成功；完整 `Automation RunTests Sigil` 执行 51 项且 51/51 成功；隔离的 ProjectSpecter `37b1a774` + Sigil `59f3375` 组合通过 `ProjectSpecterEditor Win64 Development` 构建。
- 验证边界: 以上是编译与 Automation 证据，不代表 PIE、多人运行时、Win64 Cook 或打包运行时已验证。
