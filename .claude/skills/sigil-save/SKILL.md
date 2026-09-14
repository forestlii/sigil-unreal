---
name: sigil-save
description: 在接入、修改或排查 SigilSave 的 JSON 存档槽写入、读取、删除、JSON 校验或 SaveGame 包装时读取。
---

# Sigil JSON 槽位存储

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilSave/Source/SigilSave/。

- Public/SigilSaveSubsystem.h。
- Private/SigilSaveSubsystem.cpp。
- Private/SigilJsonSaveGame.h。
- 依赖来源：SigilSave.Build.cs。

## 已核实的接口约束

- SaveSubsystem 是 GameInstanceSubsystem，公开
  SaveJson、LoadJson、DeleteJson，参数包含 SlotName 和 UserIndex。
- SaveJson 先进行 JSON 语法校验，再将文本放入
  SigilJsonSaveGame，调用 SaveGameToSlot。
- LoadJson 先清空输出；加载后检查 SaveGame 类型和 JSON 语法，
  只有成功后才返回文本。
- DeleteJson 拒绝空 SlotName，再调用 DeleteGameInSlot。
- 当前使用同步 GameplayStatics 槽位接口；
  这些代码不构成云同步、异步保存或业务数据迁移实现。

## 处理当前任务

分别确认 JSON 生成、语法校验、槽名/UserIndex 和引擎槽位操作的结果。
不要把 JSON 语法有效当作业务 Schema 有效。
调用方必须处理失败返回值，不能读取 LoadJson 失败前的旧输出。
叙事快照的 Schema 与 Catalog 约束读取 sigil-narrative-runtime。

## 范围

仅覆盖 JSON 文本与引擎存档槽之间的存储边界。
业务载荷、自动保存时机、检查点策略和云端存档由消费项目负责。
