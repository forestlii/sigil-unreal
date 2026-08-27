// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "SigilDialogueAsset.h"
#include "SigilDialoguePreviewModel.h"
#include "Tests/SigilDialogueEditorTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilDialogueSafePreviewTest,
	"SigilNarrative.DialogueEditor.SafePreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilDialogueSafePreviewTest::RunTest(const FString& Parameters)
{
	USigilDialogueEditorTestCondition::CallCount = 0;
	USigilDialogueEditorTestEvent::CallCount = 0;

	USigilDialogueAsset* Asset = NewObject<USigilDialogueAsset>(GetTransientPackage());
	Asset->DialogueId = TEXT("SafePreviewTest");
	Asset->EntryNodeId = TEXT("Intro");

	FSigilDialogueNode IntroNode;
	IntroNode.NodeId = TEXT("Intro");
	IntroNode.NodeType = ESigilDialogueNodeType::Line;
	IntroNode.SpeakerId = TEXT("Guide");
	IntroNode.Text = FText::FromString(TEXT("Welcome"));
	IntroNode.NextNodeId = TEXT("Choice");

	FSigilDialogueNode ChoiceNode;
	ChoiceNode.NodeId = TEXT("Choice");
	ChoiceNode.NodeType = ESigilDialogueNodeType::Choice;
	FSigilDialogueOption& AcceptOption = ChoiceNode.Options.AddDefaulted_GetRef();
	AcceptOption.OptionId = TEXT("Accept");
	AcceptOption.Text = FText::FromString(TEXT("Continue"));
	AcceptOption.TargetNodeId = TEXT("End");
	AcceptOption.Conditions.Add(NewObject<USigilDialogueEditorTestCondition>(Asset));
	AcceptOption.Events.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));

	FSigilDialogueNode EndNode;
	EndNode.NodeId = TEXT("End");
	EndNode.NodeType = ESigilDialogueNodeType::End;

	Asset->Nodes = {IntroNode, ChoiceNode, EndNode};

	FSigilDialoguePreviewModel Preview;
	FText Error;
	TestTrue(TEXT("合法资产可开始"), Preview.Start(Asset, Error));
	TestEqual(TEXT("开始后位于入口节点"), Preview.GetCurrentNodeId(), FName(TEXT("Intro")));
	TestTrue(TEXT("Line 可推进"), Preview.Advance(Error));
	TestEqual(TEXT("推进后位于 Choice"), Preview.GetCurrentNodeId(), FName(TEXT("Choice")));

	const FSigilDialoguePreviewConditionKey ConditionKey{
		TEXT("Choice"),
		TEXT("Accept"),
		0};
	TestFalse(TEXT("Condition 未指定不可选择"), Preview.Choose(TEXT("Accept"), Error));
	Preview.SetConditionResult(ConditionKey, ESigilDialoguePreviewConditionResult::False);
	TestFalse(TEXT("Condition 手动为 False 不可选择"), Preview.Choose(TEXT("Accept"), Error));
	Preview.SetConditionResult(ConditionKey, ESigilDialoguePreviewConditionResult::True);
	TestTrue(TEXT("Condition 手动为 True 后可选择"), Preview.Choose(TEXT("Accept"), Error));
	TestEqual(TEXT("选择后保留 End 节点供 UI 显示"), Preview.GetCurrentNodeId(), FName(TEXT("End")));
	TestTrue(TEXT("进入 End 后预览完成"), Preview.IsComplete());
	TestFalse(TEXT("完成后不再活动"), Preview.IsActive());
	TestEqual(TEXT("Condition 从未执行"), USigilDialogueEditorTestCondition::CallCount, 0);
	TestEqual(TEXT("Event 从未执行"), USigilDialogueEditorTestEvent::CallCount, 0);
	TestEqual(TEXT("只记录一个 Event"), Preview.GetEventLog().Num(), 1);
	if (Preview.GetEventLog().Num() == 1)
	{
		const FSigilDialoguePreviewEventRecord& EventRecord = Preview.GetEventLog()[0];
		TestEqual(TEXT("Event 日志记录源节点"), EventRecord.NodeId, FName(TEXT("Choice")));
		TestEqual(TEXT("Event 日志记录 Option"), EventRecord.OptionId, FName(TEXT("Accept")));
		TestTrue(
			TEXT("Event 日志记录类型"),
			EventRecord.EventClass == USigilDialogueEditorTestEvent::StaticClass());
	}

	USigilDialogueAsset* InvalidAsset = NewObject<USigilDialogueAsset>(GetTransientPackage());
	InvalidAsset->DialogueId = TEXT("InvalidPreviewTest");
	TestFalse(TEXT("无效定义不可开始"), Preview.Start(InvalidAsset, Error));

	TestTrue(TEXT("可重新开始合法资产"), Preview.Start(Asset, Error));
	Preview.Invalidate();
	TestFalse(TEXT("失效后 Advance 必须失败"), Preview.Advance(Error));
	TestFalse(TEXT("失效后 Choose 必须失败"), Preview.Choose(TEXT("Accept"), Error));
	TestEqual(TEXT("失效后清空当前节点"), Preview.GetCurrentNodeId(), NAME_None);

	return !HasAnyErrors();
}

#endif
