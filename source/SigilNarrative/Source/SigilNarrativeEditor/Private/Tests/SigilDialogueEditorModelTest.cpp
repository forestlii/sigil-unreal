// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "SSigilDialogueEditor.h"
#include "SigilDialogueAsset.h"
#include "SigilDialogueEditorModel.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilDialogueEditorNodeModelTest,
	"SigilNarrative.DialogueEditor.NodeModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilDialogueEditorNodeModelTest::RunTest(const FString& Parameters)
{
	if (!TestNotNull(TEXT("必须存在 Editor 事务系统"), GEditor))
	{
		return false;
	}

	USigilDialogueAsset* Asset = NewObject<USigilDialogueAsset>(
		GetTransientPackage(), NAME_None, RF_Transactional);
	Asset->DialogueId = TEXT("EditorModelTest");
	Asset->EntryNodeId = TEXT("Start");

	FSigilDialogueNode StartNode;
	StartNode.NodeId = TEXT("Start");
	StartNode.NodeType = ESigilDialogueNodeType::Line;
	StartNode.SpeakerId = TEXT("Guide");
	StartNode.NextNodeId = TEXT("End");

	FSigilDialogueNode ChoiceNode;
	ChoiceNode.NodeId = TEXT("Pick");
	ChoiceNode.NodeType = ESigilDialogueNodeType::Choice;
	FSigilDialogueOption& ChoiceOption = ChoiceNode.Options.AddDefaulted_GetRef();
	ChoiceOption.OptionId = TEXT("Accept");
	ChoiceOption.TargetNodeId = TEXT("End");

	FSigilDialogueNode EndNode;
	EndNode.NodeId = TEXT("End");
	EndNode.NodeType = ESigilDialogueNodeType::End;

	Asset->Nodes = {StartNode, ChoiceNode, EndNode};

	FText ValidationError;
	TestTrue(TEXT("初始定义必须有效"), Asset->ValidateDefinition(ValidationError));

	FSigilDialogueEditorModel Model(Asset);
	const int32 StartNodeIndex = Model.FindNodeIndex(TEXT("Start"));
	const TSharedRef<FDialogueNodeStructOnScope> StartNodeScope =
		MakeShared<FDialogueNodeStructOnScope>(Asset, StartNodeIndex);
	TestTrue(TEXT("节点结构提供器必须有效"), StartNodeScope->IsValid());
	TestTrue(
		TEXT("节点结构类型必须正确"),
		StartNodeScope->GetStruct() == FSigilDialogueNode::StaticStruct());
	FSigilDialogueNode* ScopedStartNode = reinterpret_cast<FSigilDialogueNode*>(StartNodeScope->GetStructMemory());
	ScopedStartNode->SpeakerId = TEXT("EditedGuide");
	TestEqual(
		TEXT("结构提供器必须直接编辑资产节点"),
		Asset->Nodes[StartNodeIndex].SpeakerId,
		FName(TEXT("EditedGuide")));

	int32 ChangeCount = 0;
	const FDelegateHandle ChangeHandle = Model.OnModelChanged().AddLambda([&ChangeCount]()
	{
		++ChangeCount;
	});

	const FName AddedNodeId = Model.AddNode(ESigilDialogueNodeType::Line);
	TestEqual(TEXT("新增 ID 必须稳定"), AddedNodeId, FName(TEXT("Node_001")));
	TestTrue(TEXT("新增节点必须存在"), Model.FindNodeIndex(AddedNodeId) != INDEX_NONE);
	TestFalse(TEXT("未连接的新 Line 必须使定义无效"), Asset->ValidateDefinition(ValidationError));

	TestTrue(TEXT("Add 必须可以撤销"), GEditor->UndoTransaction());
	TestEqual(TEXT("撤销 Add 必须移除节点"), Model.FindNodeIndex(AddedNodeId), INDEX_NONE);
	TestTrue(TEXT("撤销 Add 后定义恢复有效"), Asset->ValidateDefinition(ValidationError));

	TestTrue(TEXT("Add 必须可以重做"), GEditor->RedoTransaction());
	TestTrue(TEXT("重做 Add 必须恢复节点"), Model.FindNodeIndex(AddedNodeId) != INDEX_NONE);
	TestFalse(TEXT("重做未连接 Line 后定义再次无效"), Asset->ValidateDefinition(ValidationError));

	Asset->Nodes[Model.FindNodeIndex(AddedNodeId)].NextNodeId = TEXT("End");
	TestTrue(TEXT("补齐新增节点目标后定义有效"), Asset->ValidateDefinition(ValidationError));

	const FName FirstCopyId = Model.DuplicateNode(TEXT("Start"));
	const FName SecondCopyId = Model.DuplicateNode(TEXT("Start"));
	TestEqual(TEXT("第一次复制 ID 必须稳定"), FirstCopyId, FName(TEXT("Start_Copy")));
	TestEqual(TEXT("第二次复制 ID 必须稳定"), SecondCopyId, FName(TEXT("Start_Copy_2")));

	FText DeleteReason;
	TestFalse(TEXT("被引用节点不可删除"), Model.CanDeleteNode(TEXT("End"), DeleteReason));
	TestTrue(TEXT("删除失败原因必须指出引用节点"), DeleteReason.ToString().Contains(TEXT("Start")));
	TestFalse(TEXT("Delete 必须执行同一引用保护"), Model.DeleteNode(TEXT("End"), DeleteReason));

	TestTrue(TEXT("可以设置新入口"), Model.SetEntryNode(FirstCopyId));
	TestEqual(TEXT("入口必须更新"), Asset->EntryNodeId, FirstCopyId);
	TestTrue(TEXT("SetEntry 必须可以撤销"), GEditor->UndoTransaction());
	TestEqual(TEXT("撤销 SetEntry 必须恢复原入口"), Asset->EntryNodeId, FName(TEXT("Start")));
	TestTrue(TEXT("撤销 SetEntry 后定义有效"), Asset->ValidateDefinition(ValidationError));
	TestTrue(TEXT("SetEntry 必须可以重做"), GEditor->RedoTransaction());
	TestEqual(TEXT("重做 SetEntry 必须恢复新入口"), Asset->EntryNodeId, FirstCopyId);
	TestTrue(TEXT("重做 SetEntry 后定义有效"), Asset->ValidateDefinition(ValidationError));

	TestTrue(TEXT("End 可以成为入口"), Model.SetEntryNode(TEXT("End")));
	const int32 EndNodeIndex = Model.FindNodeIndex(TEXT("End"));
	Asset->Nodes[EndNodeIndex].NodeId = TEXT("Finish");
	TestTrue(
		TEXT("合法改名必须成功"),
		Model.ReconcileNodeEdit(
			EndNodeIndex,
			TEXT("End"),
			ESigilDialogueNodeType::End,
			ValidationError));
	TestEqual(TEXT("入口引用必须随改名更新"), Asset->EntryNodeId, FName(TEXT("Finish")));
	TestEqual(TEXT("Line 目标必须随改名更新"), Asset->FindNode(TEXT("Start"))->NextNodeId, FName(TEXT("Finish")));
	TestEqual(TEXT("新增 Line 目标必须随改名更新"), Asset->FindNode(AddedNodeId)->NextNodeId, FName(TEXT("Finish")));
	TestEqual(TEXT("复制 Line 目标必须随改名更新"), Asset->FindNode(FirstCopyId)->NextNodeId, FName(TEXT("Finish")));
	TestEqual(TEXT("Option 目标必须随改名更新"), Asset->FindNode(TEXT("Pick"))->Options[0].TargetNodeId, FName(TEXT("Finish")));

	const int32 FinishNodeIndex = Model.FindNodeIndex(TEXT("Finish"));
	Asset->Nodes[FinishNodeIndex].NodeId = NAME_None;
	TestFalse(
		TEXT("空 NodeId 必须被拒绝"),
		Model.ReconcileNodeEdit(
			FinishNodeIndex,
			TEXT("Finish"),
			ESigilDialogueNodeType::End,
			ValidationError));
	TestEqual(TEXT("空 NodeId 被拒后必须恢复"), Asset->Nodes[FinishNodeIndex].NodeId, FName(TEXT("Finish")));

	Asset->Nodes[FinishNodeIndex].NodeId = TEXT("Start");
	TestFalse(
		TEXT("重复 NodeId 必须被拒绝"),
		Model.ReconcileNodeEdit(
			FinishNodeIndex,
			TEXT("Finish"),
			ESigilDialogueNodeType::End,
			ValidationError));
	TestEqual(TEXT("重复 NodeId 被拒后必须恢复"), Asset->Nodes[FinishNodeIndex].NodeId, FName(TEXT("Finish")));

	const int32 FirstCopyIndex = Model.FindNodeIndex(FirstCopyId);
	Asset->Nodes[FirstCopyIndex].NodeType = ESigilDialogueNodeType::End;
	TestTrue(
		TEXT("类型切换必须成功"),
		Model.ReconcileNodeEdit(
			FirstCopyIndex,
			FirstCopyId,
			ESigilDialogueNodeType::Line,
			ValidationError));
	TestTrue(TEXT("切换为 End 必须清理 NextNodeId"), Asset->Nodes[FirstCopyIndex].NextNodeId.IsNone());
	TestTrue(TEXT("切换为 End 必须清理 Options"), Asset->Nodes[FirstCopyIndex].Options.IsEmpty());

	const TArray<FName> FilteredNodeIds = Model.GetFilteredNodeIds(TEXT("copy"));
	TestEqual(TEXT("过滤必须忽略大小写并返回两个复制节点"), FilteredNodeIds.Num(), 2);
	TestTrue(TEXT("过滤结果必须包含第一次复制"), FilteredNodeIds.Contains(FirstCopyId));
	TestTrue(TEXT("过滤结果必须包含第二次复制"), FilteredNodeIds.Contains(SecondCopyId));

	TestTrue(TEXT("模型变化必须广播"), ChangeCount >= 7);
	Model.OnModelChanged().Remove(ChangeHandle);
	return !HasAnyErrors();
}

#endif
