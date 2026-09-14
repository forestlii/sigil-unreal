---
name: sigil-narrative-runtime
description: 在接入、修改或排查 SigilNarrative 的对话会话、Quest/Story 状态、条件与事件、演出宿主协议、NPC 日程数据或叙事快照时读取。
---

# Sigil 叙事运行时

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilNarrative/Source/SigilNarrative/。

- Public/SigilNarrativeSubsystem.h 与 Private/SigilNarrativeSubsystem.cpp。
- Public/SigilDialogueSession.h 与 Private/SigilDialogueSession.cpp。
- 按任务读取 Public 下的 SigilDialogueAsset.h、SigilQuestAsset.h、
  SigilStoryAsset.h、SigilNpcScheduleAsset.h 和对应实现。
- 演出协议：Public/SigilNarrativePresentation.h 与对应 Private 实现。
- 快照：Private/SigilNarrativeSnapshot.cpp，
  配套 Public/SigilNarrativeCatalog.h。
- 依赖来源：SigilNarrative.Build.cs；运行模块不依赖 SigilSave。

## 已核实的接口约束

- DialogueSession 的 Start、Advance 等入口检查会话或回调状态；
  回调分发与 Subsystem 的 Begin/EndDialogueCallbackDispatch 配合。
- PresentationHost 接口的默认 CanBegin/Begin 返回 false；
  消费项目需要提供实际演出宿主。
- Subsystem 负责登记宿主并调用演出协议；
  通用接口本身不包含游戏 LevelSequence 实现。
- Snapshot 实现的 SchemaVersion 当前为 1。
- ImportSnapshotJson 检查回调状态、Catalog、JSON Schema
  以及引用的 Quest/Story 定义；成功路径最后替换
  Flags、QuestStates 和 StoryStates。
- 快照导入导出处理 JSON 文本，不直接写入磁盘槽位。

## 处理当前任务

按对话、任务、故事、演出或快照选择对应调用链，避免全量加载。
状态变更同时检查条件、事件回调与重入限制。
快照兼容性以目标版本 Schema 和 Catalog 为准；
磁盘存储读取 sigil-save，保存时机与恢复策略留在消费项目。
NPC 日程数据不等于游戏侧寻路、Controller 或行为树实现。

## 范围

覆盖通用叙事数据与运行状态。
不包含 Slate 编辑器、游戏剧情正文、具体演出和磁盘保存策略。
