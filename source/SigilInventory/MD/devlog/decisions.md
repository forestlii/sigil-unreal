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
- 面临的选择: 预检查通过后直接移除全部来源物品，或只移除目标集合实际接受的数量。
- 定了什么: 每个来源项只调用一次目标 `AddItem`，不把已保留的来源余量从 overflow 路径再返回；只移除实际加入量，累计加入量大于零才成功并广播。
- 否掉了什么 + 为什么: 否掉重做集合 overflow 合同、公开 API 或拾取事务；本修复只维护来源余量与实际转移量的一致性。
- 复用层🔑: ② 引擎相关
- 来源: Finding `INVG-01`；Sigil commit `b58976d`；`SigilInventory.Pickup.PartialTransfer` Automation（2/2 通过，见审查批次 A Task 3 报告）。

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
