// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "SigilStoryAsset.h"
#include "SigilStoryPreviewModel.h"
#include "Tests/SigilDialogueEditorTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilStorySafePreviewTest,
	"SigilNarrative.StoryEditor.SafePreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilStorySafePreviewTest::RunTest(const FString& Parameters)
{
	USigilDialogueEditorTestCondition::CallCount = 0;
	USigilDialogueEditorTestEvent::CallCount = 0;

	USigilStoryAsset* Asset = NewObject<USigilStoryAsset>(GetTransientPackage());
	Asset->StoryId = TEXT("Story.SafePreview");

	FSigilStoryBeatDefinition IntroBeat;
	IntroBeat.BeatId = TEXT("Intro");
	IntroBeat.EnterConditions.Add(NewObject<USigilDialogueEditorTestCondition>(Asset));
	IntroBeat.EnterEvents.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));
	IntroBeat.CompleteEvents.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));

	FSigilStoryBeatDefinition AfterIntroBeat;
	AfterIntroBeat.BeatId = TEXT("AfterIntro");
	AfterIntroBeat.EnterEvents.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));
	Asset->Beats = {IntroBeat, AfterIntroBeat};

	FSigilStoryPreviewModel Preview;
	FText Error;
	TestTrue(TEXT("合法 Story 可开始安全预览"), Preview.Start(Asset, Error));
	TestEqual(TEXT("预览开始时不得自动进入 Beat"), Preview.GetActiveBeatId(), NAME_None);

	TestFalse(TEXT("Condition 未指定时不可进入 Intro"), Preview.EnterBeat(TEXT("Intro"), Error));
	const FSigilStoryPreviewConditionKey IntroCondition{TEXT("Intro"), 0};
	Preview.SetConditionResult(IntroCondition, ESigilStoryPreviewConditionResult::False);
	TestFalse(TEXT("Condition 手动为 False 时不可进入 Intro"), Preview.EnterBeat(TEXT("Intro"), Error));
	Preview.SetConditionResult(IntroCondition, ESigilStoryPreviewConditionResult::True);
	TestTrue(TEXT("Condition 手动为 True 时可以进入 Intro"), Preview.EnterBeat(TEXT("Intro"), Error));
	TestEqual(TEXT("Intro 必须成为活动 Beat"), Preview.GetActiveBeatId(), FName(TEXT("Intro")));
	TestEqual(TEXT("进入事件只能被记录"), Preview.GetEventLog().Num(), 1);

	TestFalse(TEXT("已有活动 Beat 时不可进入第二个 Beat"), Preview.EnterBeat(TEXT("AfterIntro"), Error));
	TestTrue(TEXT("可以手动完成活动 Beat"), Preview.CompleteActiveBeat(Error));
	TestTrue(TEXT("Intro 必须记录为已完成"), Preview.IsBeatCompleted(TEXT("Intro")));
	TestEqual(TEXT("完成后必须清空活动 Beat"), Preview.GetActiveBeatId(), NAME_None);
	TestEqual(TEXT("必须记录进入和完成两个事件"), Preview.GetEventLog().Num(), 2);
	TestEqual(TEXT("完成 Intro 后不得自动进入下一 Beat"), Preview.GetVisitHistory().Num(), 1);

	TestTrue(TEXT("下一 Beat 只能由设计者手动进入"), Preview.EnterBeat(TEXT("AfterIntro"), Error));
	TestEqual(TEXT("AfterIntro 必须成为活动 Beat"), Preview.GetActiveBeatId(), FName(TEXT("AfterIntro")));
	TestFalse(TEXT("已完成 Beat 不可再次进入"), Preview.EnterBeat(TEXT("Intro"), Error));
	TestEqual(TEXT("Condition 绝不能在预览中执行"), USigilDialogueEditorTestCondition::CallCount, 0);
	TestEqual(TEXT("Event 绝不能在预览中执行"), USigilDialogueEditorTestEvent::CallCount, 0);

	USigilStoryAsset* InvalidAsset = NewObject<USigilStoryAsset>(GetTransientPackage());
	TestFalse(TEXT("无效 Story 不可开始预览"), Preview.Start(InvalidAsset, Error));

	TestTrue(TEXT("可以重新开始合法 Story"), Preview.Start(Asset, Error));
	Preview.Invalidate();
	TestFalse(TEXT("失效后不可进入 Beat"), Preview.EnterBeat(TEXT("Intro"), Error));
	TestFalse(TEXT("失效后不可完成 Beat"), Preview.CompleteActiveBeat(Error));
	TestEqual(TEXT("失效后必须清空活动 Beat"), Preview.GetActiveBeatId(), NAME_None);

	return !HasAnyErrors();
}

#endif
