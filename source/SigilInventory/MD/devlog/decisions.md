# SigilInventory 决策日志

> 本文件只记录 `SigilInventory` 插件内部的审查修复取舍，不记录消费项目玩法、资产或运行时结论。
>
> 最后更新：2026-08-30。

### [2026-08-30] INVC-01 跳过坏存档栈，不扩展集合路由

- 阶段: 审查批次 A 修复
- 面临的选择: 使用会插入空项的 Map 访问方式，或只查找物品并跳过无法解析的栈。
- 定了什么: `DeserializeCollection_Implementation` 使用非插入式查找；缺失或无效物品记录包含 `StackId` 与 `ItemId` 的警告、跳过该栈，并继续处理后续有效栈。
- 否掉了什么 + 为什么: 否掉修改有效栈的多集合反序列化路由；该架构问题属于未修复的 `INVC-11`，不应借坏存档修复改变其行为。
- 复用层🔑: ② 引擎相关
- 来源: Finding `INVC-01`；Sigil commit `396b027`；`SigilInventory.Serialization.MissingItem` Automation（1/1 通过，见审查批次 A Task 2 报告）。

### [2026-08-30] INVC-02 Server RPC 仅进入本地装载实现一次

- 阶段: 审查批次 A 修复
- 面临的选择: 在 RPC 实现中再次经 RPC 包装器分发，或直接调用既有本地装载实现。
- 定了什么: `ServerLoadDefaultLoadouts_Implementation` 只调用一次 `LoadDefaultLoadouts()`。
- 否掉了什么 + 为什么: 否掉改动公开 Server RPC 声明、装载数据模型或联网架构；本修复仅移除递归分发。
- 复用层🔑: ② 引擎相关
- 来源: Finding `INVC-02`；Sigil commit `396b027`；`SigilInventory.Loadouts.ServerRpc` Automation（1/1 通过，见审查批次 A Task 2 报告）。

### [2026-08-30] INVG-01 按实际接收量处理部分拾取

- 阶段: 审查批次 A 修复
- 面临的选择: 先向目标添加、再依赖目标 `RemoveItem` 回滚来源短缺并以全集合总量测差，或在双方预检后按同一逻辑道具执行源先移除的局部守恒路径。
- 定了什么: 在 collection restriction 从预检到执行保持稳定的合同下，先用目标 `CanAddItem` 与来源 `RemoveItemCondition` 取得双方允许量并取较小值；随后先从来源移除，以来源同逻辑道具的前后数量确认实际移除量，再以 `ItemCollection = nullptr` 向目标添加该实移量，并以目标同逻辑道具的前后数量确认实际加入量。目标拒收部分只尝试恢复到来源并计量实际恢复；只有恢复后来源净减量等于目标实际增量的非零部分才计入成功，目标 `RemoveItem` 不再属于正常路径。
- 否掉了什么 + 为什么: 否掉重做 collection overflow 合同、`INVC-12` 的多栈返回值合同、公开 API、配置开关、跨集合事务或锁。任意 Blueprint 回调若在操作期间重入并修改同一逻辑道具，或来源在预检后动态拒绝恢复，仍不具备跨集合原子保证；本轮局部修复不把这些对抗性合同描述为已保证。
- 复用层🔑: ② 引擎相关
- 来源: Finding `INVG-01`；Sigil commits `b58976d`、`2da1980` 及最终整分支复审修正；`SigilInventory.Pickup.PartialTransfer` Automation（5/5 通过）。

### [2026-08-30] INVG-02 成功移除全部材料后返回成功

- 阶段: 审查批次 A 修复
- 面临的选择: 所有移除成功后仍返回 `false`，或在每项成功移除后返回 `true`。
- 定了什么: `RemoveItemIngredients` 在全部请求材料移除成功后返回 `true`；第一个不足项仍立即返回 `false`。
- 否掉了什么 + 为什么: 否掉引入回滚或事务框架；原有非事务语义保留，失败时已移除的可用部分不会自动恢复。
- 复用层🔑: ② 引擎相关
- 来源: Finding `INVG-02`；Sigil commit `b58976d`；`SigilInventory.Crafting.RemoveIngredients` Automation（2/2 通过，见审查批次 A Task 3 报告）。

### [2026-08-30] INVG-03 收紧随机掉落的累计权重与边界

- 阶段: 审查批次 A 修复
- 面临的选择: 用未扩容数组的索引写入、允许无效总权重和越界末尾选择，或在既有抽样语义内保证安全边界。
- 定了什么: 累计权重改为 `Add`；空或非正总权重返回空；数量使用 `[MinAmount, MaxAmount]`；权重从 `[1, ProbabilitySum]` 采样并以不会越过末项的下界搜索选择。
- 否掉了什么 + 为什么: 否掉改变有放回抽样、重复实例合并结果或新增配置/公开 API；这些既有行为不属于本 finding。
- 复用层🔑: ② 引擎相关
- 来源: Finding `INVG-03`；Sigil commit `b58976d`；`SigilInventory.Drop.Random` Automation（3/3 通过，见审查批次 A Task 3 报告）。

### [2026-08-30] 审查批次 A 最终验证

- 最终证据: `HostEditor Win64 Development` 构建成功；完整 `Automation RunTests Sigil` 执行 51 项且 51/51 成功；隔离的 ProjectSpecter `37b1a774` + Sigil `59f3375` 组合通过 `ProjectSpecterEditor Win64 Development` 构建。
- 验证边界: 以上是编译与 Automation 证据，不代表 PIE、多人运行时、Win64 Cook 或打包运行时已验证；INVC-11、INVC-12 与已记录的非原子 Blueprint 重入边界仍未解决。
