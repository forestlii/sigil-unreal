---
name: sigil-narrative-authoring
description: 在修改或排查 SigilNarrative 的 Dialogue、Quest、Story 资产编辑器、EditorModel、Slate 界面、编辑一致性或预览模型时读取。
---

# Sigil 叙事资产编辑

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilNarrative/Source/SigilNarrativeEditor/。

- Private/SigilDialogueEditorModel.h 与对应实现。
- 按资产类型读取 Private 中对应的 EditorModel、
  EditorToolkit、AssetDefinition 和 SSigil 编辑界面。
- Story 预览入口：Private/SigilStoryPreviewModel.h 与对应实现。
- Dialogue、Quest 预览分别读取其对应 PreviewModel。
- 依赖来源：SigilNarrativeEditor.Build.cs；
  编辑器依赖 SigilNarrative、Slate 和 UnrealEd 等模块。

## 已核实的接口约束

- DialogueEditorModel 提供节点筛选、查找、新增、复制、
  删除判定、入口节点设置和 ReconcileNodeEdit。
- 该模型使用弱资产引用，并提供 OnModelChanged。
- StoryPreviewModel.Start 先使旧预览失效，再检查资产定义。
- Story 预览的进入条件由用户手工设置结果；
  未明确设为 True 的条件阻止进入。
- Story 预览将进入/完成事件记录到 EventLog；
  所读实现没有在这些路径执行叙事事件。
- 上述 Story 预览事实不能直接类推到 Dialogue、Quest 的实现，
  更不能作为游戏运行验收。

## 处理当前任务

编辑行为从界面追踪到 EditorModel，再追踪资产字段与定义校验。
同时核对修改通知、入口/引用一致性及预览状态失效。
资产 Schema 或运行语义变更需要追加读取 sigil-narrative-runtime；
不能只改编辑界面而遗漏运行消费者。
保持编辑器依赖与 Runtime 模块分离。

## 范围

覆盖现有 Dialogue、Quest、Story 资产编辑与预览。
不负责游戏剧情内容、实际运行演出，也不包含新建 NPC 日程专用编辑器。
