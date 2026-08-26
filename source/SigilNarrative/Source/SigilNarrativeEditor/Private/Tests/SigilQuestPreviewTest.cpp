// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "SigilQuestAsset.h"
#include "SigilQuestPreviewModel.h"
#include "Tests/SigilDialogueEditorTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilQuestSafePreviewTest,
	"SigilNarrative.QuestEditor.SafePreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilQuestSafePreviewTest::RunTest(const FString& Parameters)
{
	USigilDialogueEditorTestCondition::CallCount = 0;
	USigilDialogueEditorTestEvent::CallCount = 0;

	USigilQuestAsset* Asset = NewObject<USigilQuestAsset>(GetTransientPackage());
	Asset->QuestId = TEXT("Quest.SafePreview");
	Asset->InitialStateId = TEXT("Work");

	FSigilQuestState WorkState;
	WorkState.StateId = TEXT("Work");
	WorkState.EntryEvents.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));
	FSigilQuestTaskDefinition& CollectTask = WorkState.Tasks.AddDefaulted_GetRef();
	CollectTask.TaskId = TEXT("Collect");
	CollectTask.ObjectiveText = FText::FromString(TEXT("Collect two items"));
	CollectTask.RequiredCount = 2;
	FSigilQuestTransition& FinishTransition = WorkState.Transitions.AddDefaulted_GetRef();
	FinishTransition.TransitionId = TEXT("Finish");
	FinishTransition.TargetStateId = TEXT("Done");
	FinishTransition.RequiredTaskIds.Add(TEXT("Collect"));
	FinishTransition.Conditions.Add(NewObject<USigilDialogueEditorTestCondition>(Asset));
	FinishTransition.Events.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));

	FSigilQuestState DoneState;
	DoneState.StateId = TEXT("Done");
	DoneState.StateType = ESigilQuestStateType::Success;
	DoneState.EntryEvents.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));
	Asset->States = {WorkState, DoneState};

	FSigilQuestPreviewModel Preview;
	FText Error;
	TestTrue(TEXT("合法 Quest 可开始安全预览"), Preview.Start(Asset, Error));
	TestEqual(TEXT("预览必须从初始状态开始"), Preview.GetCurrentStateId(), FName(TEXT("Work")));
	TestEqual(TEXT("初始任务进度必须为零"), Preview.GetTaskProgress(TEXT("Collect")), 0);
	TestEqual(TEXT("进入状态事件只能被记录"), Preview.GetEventLog().Num(), 1);

	TestFalse(TEXT("任务未完成时不可转移"), Preview.TakeTransition(TEXT("Finish"), Error));
	TestTrue(TEXT("可以手动设置预览任务进度"), Preview.SetTaskProgress(TEXT("Collect"), 2));
	TestFalse(TEXT("Condition 未指定时不可转移"), Preview.TakeTransition(TEXT("Finish"), Error));

	const FSigilQuestPreviewConditionKey ConditionKey{TEXT("Work"), TEXT("Finish"), 0};
	Preview.SetConditionResult(ConditionKey, ESigilQuestPreviewConditionResult::False);
	TestFalse(TEXT("Condition 手动为 False 时不可转移"), Preview.TakeTransition(TEXT("Finish"), Error));
	Preview.SetConditionResult(ConditionKey, ESigilQuestPreviewConditionResult::True);
	TestTrue(TEXT("任务完成且 Condition 手动为 True 时可以转移"), Preview.TakeTransition(TEXT("Finish"), Error));

	TestEqual(TEXT("转移后必须进入目标状态"), Preview.GetCurrentStateId(), FName(TEXT("Done")));
	TestEqual(TEXT("成功状态必须结束预览"), Preview.GetStatus(), ESigilQuestStatus::Succeeded);
	TestFalse(TEXT("成功状态后预览不再活动"), Preview.IsActive());
	TestEqual(TEXT("必须记录入口、转移和目标入口三个事件"), Preview.GetEventLog().Num(), 3);
	TestEqual(TEXT("Condition 绝不能在预览中执行"), USigilDialogueEditorTestCondition::CallCount, 0);
	TestEqual(TEXT("Event 绝不能在预览中执行"), USigilDialogueEditorTestEvent::CallCount, 0);

	USigilQuestAsset* InvalidAsset = NewObject<USigilQuestAsset>(GetTransientPackage());
	InvalidAsset->QuestId = TEXT("Quest.InvalidPreview");
	TestFalse(TEXT("无效 Quest 不可开始预览"), Preview.Start(InvalidAsset, Error));

	TestTrue(TEXT("可以重新开始合法 Quest"), Preview.Start(Asset, Error));
	Preview.Invalidate();
	TestFalse(TEXT("失效后不可设置任务进度"), Preview.SetTaskProgress(TEXT("Collect"), 2));
	TestFalse(TEXT("失效后不可执行转移"), Preview.TakeTransition(TEXT("Finish"), Error));
	TestEqual(TEXT("失效后必须清空当前状态"), Preview.GetCurrentStateId(), NAME_None);

	return !HasAnyErrors();
}

#endif
