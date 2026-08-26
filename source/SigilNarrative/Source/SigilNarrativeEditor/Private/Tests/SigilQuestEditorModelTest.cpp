// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "SigilQuestAsset.h"
#include "SigilQuestEditorModel.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilQuestEditorModelTest,
	"SigilNarrative.QuestEditor.StateModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilQuestEditorModelTest::RunTest(const FString& Parameters)
{
	USigilQuestAsset* Asset = NewObject<USigilQuestAsset>(
		GetTransientPackage(), NAME_None, RF_Transactional);
	Asset->QuestId = TEXT("Quest.EditorModel");
	Asset->InitialStateId = TEXT("Work");

	FSigilQuestState WorkState;
	WorkState.StateId = TEXT("Work");
	FSigilQuestTaskDefinition& CollectTask = WorkState.Tasks.AddDefaulted_GetRef();
	CollectTask.TaskId = TEXT("Collect");
	CollectTask.ObjectiveText = FText::FromString(TEXT("Collect one item"));
	CollectTask.RequiredCount = 1;
	FSigilQuestTransition& FinishTransition = WorkState.Transitions.AddDefaulted_GetRef();
	FinishTransition.TransitionId = TEXT("Finish");
	FinishTransition.TargetStateId = TEXT("Done");
	FinishTransition.RequiredTaskIds.Add(TEXT("Collect"));

	FSigilQuestState DoneState;
	DoneState.StateId = TEXT("Done");
	DoneState.StateType = ESigilQuestStateType::Success;
	Asset->States = {WorkState, DoneState};

	FText Error;
	TestTrue(TEXT("初始 Quest 定义必须有效"), Asset->ValidateDefinition(Error));

	FSigilQuestEditorModel Model(Asset);
	const int32 WorkIndex = Model.FindStateIndex(TEXT("Work"));
	const TSharedRef<FQuestStateStructOnScope> WorkScope =
		MakeShared<FQuestStateStructOnScope>(Asset, WorkIndex);
	TestTrue(TEXT("状态结构提供器必须有效"), WorkScope->IsValid());
	TestTrue(
		TEXT("状态结构类型必须正确"),
		WorkScope->GetStruct() == FSigilQuestState::StaticStruct());
	FSigilQuestState* ScopedWork = reinterpret_cast<FSigilQuestState*>(WorkScope->GetStructMemory());
	ScopedWork->Tasks[0].ObjectiveText = FText::FromString(TEXT("Edited objective"));
	TestEqual(
		TEXT("结构提供器必须直接编辑真实 Quest 状态"),
		Asset->States[WorkIndex].Tasks[0].ObjectiveText.ToString(),
		FString(TEXT("Edited objective")));

	int32 ChangeCount = 0;
	const FDelegateHandle ChangeHandle = Model.OnModelChanged().AddLambda([&ChangeCount]()
	{
		++ChangeCount;
	});

	const FName AddedStateId = Model.AddState(ESigilQuestStateType::Regular);
	TestEqual(TEXT("新增状态 ID 必须稳定"), AddedStateId, FName(TEXT("State_001")));
	TestTrue(TEXT("新增状态必须存在"), Model.FindStateIndex(AddedStateId) != INDEX_NONE);
	TestTrue(TEXT("新增普通状态后定义仍有效"), Asset->ValidateDefinition(Error));
	TestTrue(TEXT("新增状态必须可以撤销"), GEditor->UndoTransaction());
	TestEqual(TEXT("撤销新增必须移除状态"), Model.FindStateIndex(AddedStateId), INDEX_NONE);
	TestTrue(TEXT("新增状态必须可以重做"), GEditor->RedoTransaction());
	TestTrue(TEXT("重做新增必须恢复状态"), Model.FindStateIndex(AddedStateId) != INDEX_NONE);

	const FName FirstCopyId = Model.DuplicateState(TEXT("Work"));
	const FName SecondCopyId = Model.DuplicateState(TEXT("Work"));
	TestEqual(TEXT("第一次复制 ID 必须稳定"), FirstCopyId, FName(TEXT("Work_Copy")));
	TestEqual(TEXT("第二次复制 ID 必须稳定"), SecondCopyId, FName(TEXT("Work_Copy_2")));

	FText DeleteReason;
	TestFalse(TEXT("被转移引用的状态不可删除"), Model.CanDeleteState(TEXT("Done"), DeleteReason));
	TestTrue(TEXT("删除失败原因必须指出转移"), DeleteReason.ToString().Contains(TEXT("Finish")));
	TestFalse(TEXT("Delete 必须执行同一引用保护"), Model.DeleteState(TEXT("Done"), DeleteReason));

	TestTrue(TEXT("可以设置新的初始状态"), Model.SetInitialState(FirstCopyId));
	TestEqual(TEXT("初始状态必须更新"), Asset->InitialStateId, FirstCopyId);
	TestTrue(TEXT("设置初始状态必须可以撤销"), GEditor->UndoTransaction());
	TestEqual(TEXT("撤销后必须恢复原初始状态"), Asset->InitialStateId, FName(TEXT("Work")));
	TestTrue(TEXT("设置初始状态必须可以重做"), GEditor->RedoTransaction());
	TestEqual(TEXT("重做后必须恢复新初始状态"), Asset->InitialStateId, FirstCopyId);

	const int32 DoneIndex = Model.FindStateIndex(TEXT("Done"));
	Asset->States[DoneIndex].StateId = TEXT("Completed");
	TestTrue(
		TEXT("合法状态改名必须成功"),
		Model.ReconcileStateEdit(
			DoneIndex,
			TEXT("Done"),
			ESigilQuestStateType::Success,
			Error));
	TestEqual(
		TEXT("原状态的转移目标必须随改名更新"),
		Asset->FindState(TEXT("Work"))->Transitions[0].TargetStateId,
		FName(TEXT("Completed")));
	TestEqual(
		TEXT("复制状态的转移目标必须随改名更新"),
		Asset->FindState(FirstCopyId)->Transitions[0].TargetStateId,
		FName(TEXT("Completed")));

	const int32 CompletedIndex = Model.FindStateIndex(TEXT("Completed"));
	Asset->States[CompletedIndex].StateId = NAME_None;
	TestFalse(
		TEXT("空 StateId 必须被拒绝"),
		Model.ReconcileStateEdit(
			CompletedIndex,
			TEXT("Completed"),
			ESigilQuestStateType::Success,
			Error));
	TestEqual(
		TEXT("空 StateId 被拒后必须恢复"),
		Asset->States[CompletedIndex].StateId,
		FName(TEXT("Completed")));

	const int32 FirstCopyIndex = Model.FindStateIndex(FirstCopyId);
	Asset->States[FirstCopyIndex].StateType = ESigilQuestStateType::Success;
	TestTrue(
		TEXT("切换为终态必须成功"),
		Model.ReconcileStateEdit(
			FirstCopyIndex,
			FirstCopyId,
			ESigilQuestStateType::Regular,
			Error));
	TestTrue(TEXT("终态必须清理任务"), Asset->States[FirstCopyIndex].Tasks.IsEmpty());
	TestTrue(TEXT("终态必须清理转移"), Asset->States[FirstCopyIndex].Transitions.IsEmpty());

	TestTrue(TEXT("未被引用的新增状态可以删除"), Model.DeleteState(AddedStateId, DeleteReason));
	TestEqual(TEXT("删除后状态必须不存在"), Model.FindStateIndex(AddedStateId), INDEX_NONE);

	const TArray<FName> FilteredStateIds = Model.GetFilteredStateIds(TEXT("copy"));
	TestEqual(TEXT("过滤必须忽略大小写并返回两个复制状态"), FilteredStateIds.Num(), 2);
	TestTrue(TEXT("过滤结果必须包含第一次复制"), FilteredStateIds.Contains(FirstCopyId));
	TestTrue(TEXT("过滤结果必须包含第二次复制"), FilteredStateIds.Contains(SecondCopyId));
	TestTrue(TEXT("模型变化必须广播"), ChangeCount >= 7);

	Model.OnModelChanged().Remove(ChangeHandle);
	return !HasAnyErrors();
}

#endif
