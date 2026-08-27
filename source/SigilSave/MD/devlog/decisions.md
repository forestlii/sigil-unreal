# SigilSave 决策日志

> 本文件只记录 `SigilSave` 插件内部的设计取舍，不记录 ProjectSpecter 的玩法、UI、镜头或内容资产。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-08-27。

### [2026-08-25] 首版只提供最小 JSON Save / Load / Delete

- 阶段: 垂直切片
- 面临的选择: 延续双代提交、跨进程恢复和自定义 Runner，或先提供最小可复用存档 API。
- 定了什么: `USigilSaveSubsystem` 只公开 `SaveJson`、`LoadJson`、`DeleteJson`，底层复用 Unreal `USaveGame` 与 `UGameplayStatics`。
- 否掉了什么 + 为什么: 否掉版本迁移、双代提交、自动恢复、加密和自定义进程 Runner；当前需求只是可靠保存和读取一段 JSON，这些机制成本失衡。
- 复用层🔑: ② 引擎相关
- 踩坑 / 反思: 插件验证应优先使用标准 Unreal Build / Automation；额外基础设施必须单独说明价值并获得确认。
- 来源: Likeon 原话“就简单地存一个json再读取都行”；Sigil commit `2e21d5c`；`SigilSaveSubsystem.h/.cpp`。

### [2026-08-25] 写入前和读取后都验证 JSON

- 阶段: 迭代
- 面临的选择: 把任意字符串原样落盘，或在 API 边界拒绝语法无效的 JSON。
- 定了什么: `SaveJson` 在写盘前严格解析；`LoadJson` 在返回前再次解析，失败时清空输出并返回 `false`。
- 否掉了什么 + 为什么: 否掉“存什么都算成功”；它会把损坏延迟到业务层，并让调用者误把旧输出当成新存档。
- 复用层🔑: ② 引擎相关
- 来源: `SigilSaveSubsystem.cpp` 的 `IsStrictJson`、`SaveJson`、`LoadJson`；`SigilSaveJsonTest.cpp` 的 malformed / trailing-comma / invalid-stored-json 用例。

### [2026-08-25] 插件只保存字符串载荷，不理解游戏 Schema

- 阶段: 选型
- 面临的选择: 让插件认识 Quest、Story、玩家属性等项目字段，或只保存调用方提供的 JSON 文本。
- 定了什么: `USigilJsonSaveGame` 只持有 JSON 字符串；Schema、字段版本和业务迁移由消费项目定义。
- 否掉了什么 + 为什么: 否掉把任一消费项目的业务模型写进通用插件；这会破坏跨项目复用边界。
- 复用层🔑: ② 引擎相关
- 来源: `SigilJsonSaveGame.h`、`SigilSaveSubsystem.h/.cpp`；Sigil commit `2e21d5c`。
