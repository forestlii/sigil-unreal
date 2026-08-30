# SigilGas 决策日志

> 本文件只记录 `SigilGas` 插件内部的审查修复取舍，不记录消费项目玩法、资产或运行时结论。
>
> 最后更新：2026-08-30。

### [2026-08-30] GAS-01 为缺失可选 Tag 提供调用局部空容器

- 阶段: 审查批次 A 修复
- 面临的选择: 继续把空的 `OptionalRelevantTags` 解引用后传给蓝图 Cost 事件，或只在该指针为空时提供安全的空容器。
- 定了什么: `USigilAbilityCost::CheckCost` 在空指针分支创建本次调用局部 `FGameplayTagContainer`；非空分支继续直接传递调用方容器。
- 否掉了什么 + 为什么: 否掉修改公开函数签名、扩展 `ActorInfo` 合同或添加持久共享容器；这些都超出只修复可选输入判空的最小范围。
- 复用层🔑: ② 引擎相关
- 来源: Finding `GAS-01`；Sigil commit `e3870be`；`SigilGas.AbilityCost.NullRelevantTagsDoesNotCrash` Automation（1/1 通过，见审查批次 A Task 1 报告）。

### [2026-08-30] 审查批次 A 最终验证

- 最终证据: `HostEditor Win64 Development` 构建成功；完整 `Automation RunTests Sigil` 执行 51 项且 51/51 成功；隔离的 ProjectSpecter `37b1a774` + Sigil `59f3375` 组合通过 `ProjectSpecterEditor Win64 Development` 构建。
- 验证边界: 以上是编译与 Automation 证据，不代表 PIE、多人运行时、Win64 Cook 或打包运行时已验证。
