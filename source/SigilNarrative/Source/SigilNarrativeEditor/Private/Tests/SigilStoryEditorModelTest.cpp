// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "SigilStoryAsset.h"
#include "SigilStoryEditorModel.h"
#include "Tests/SigilDialogueEditorTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilStoryEditorModelTest,
	"SigilNarrative.StoryEditor.BeatModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilStoryEditorModelTest::RunTest(const FString& Parameters)
{
	USigilStoryAsset* Asset = NewObject<USigilStoryAsset>(
		GetTransientPackage(), NAME_None, RF_Transactional);

	FSigilStoryBeatDefinition IntroBeat;
	IntroBeat.BeatId = TEXT("Intro");
	IntroBeat.EnterConditions.Add(NewObject<USigilDialogueEditorTestCondition>(Asset));
	IntroBeat.EnterEvents.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));
	IntroBeat.CompleteEvents.Add(NewObject<USigilDialogueEditorTestEvent>(Asset));
	Asset->Beats = {IntroBeat};

	FSigilStoryEditorModel Model(Asset);
	FText Error;
	TestFalse(TEXT("空 StoryId 的定义必须无效"), Asset->ValidateDefinition(Error));
	TestFalse(TEXT("编辑器模型必须拒绝空 StoryId"), Model.SetStoryId(NAME_None, Error));
	TestTrue(
		TEXT("编辑器模型必须能够设置 StoryId"),
		Model.SetStoryId(TEXT("Story.EditorModel"), Error));
	TestEqual(TEXT("StoryId 必须写入资产"), Asset->StoryId, FName(TEXT("Story.EditorModel")));
	TestTrue(TEXT("StoryId 修改必须可以撤销"), GEditor->UndoTransaction());
	TestTrue(TEXT("撤销后 StoryId 必须恢复为空"), Asset->StoryId.IsNone());
	TestTrue(TEXT("StoryId 修改必须可以重做"), GEditor->RedoTransaction());
	TestEqual(TEXT("重做后 StoryId 必须恢复"), Asset->StoryId, FName(TEXT("Story.EditorModel")));
	TestTrue(TEXT("设置 StoryId 后定义必须有效"), Asset->ValidateDefinition(Error));

	const int32 IntroIndex = Model.FindBeatIndex(TEXT("Intro"));
	const TSharedRef<FStoryBeatStructOnScope> IntroScope =
		MakeShared<FStoryBeatStructOnScope>(Asset, IntroIndex);
	TestTrue(TEXT("Beat 结构提供器必须有效"), IntroScope->IsValid());
	TestTrue(
		TEXT("Beat 结构类型必须正确"),
		IntroScope->GetStruct() == FSigilStoryBeatDefinition::StaticStruct());
	FSigilStoryBeatDefinition* ScopedIntro =
		reinterpret_cast<FSigilStoryBeatDefinition*>(IntroScope->GetStructMemory());
	ScopedIntro->BeatId = TEXT("IntroEdited");
	TestEqual(
		TEXT("结构提供器必须直接编辑真实 Story Beat"),
		Asset->Beats[IntroIndex].BeatId,
		FName(TEXT("IntroEdited")));
	ScopedIntro->BeatId = TEXT("Intro");

	int32 ChangeCount = 0;
	const FDelegateHandle ChangeHandle = Model.OnModelChanged().AddLambda([&ChangeCount]()
	{
		++ChangeCount;
	});

	const FName AddedBeatId = Model.AddBeat();
	TestEqual(TEXT("新增 Beat ID 必须稳定"), AddedBeatId, FName(TEXT("Beat_001")));
	TestTrue(TEXT("新增 Beat 必须存在"), Model.FindBeatIndex(AddedBeatId) != INDEX_NONE);
	TestTrue(TEXT("新增 Beat 后定义仍有效"), Asset->ValidateDefinition(Error));
	TestTrue(TEXT("新增 Beat 必须可以撤销"), GEditor->UndoTransaction());
	TestEqual(TEXT("撤销新增必须移除 Beat"), Model.FindBeatIndex(AddedBeatId), INDEX_NONE);
	TestTrue(TEXT("新增 Beat 必须可以重做"), GEditor->RedoTransaction());
	TestTrue(TEXT("重做新增必须恢复 Beat"), Model.FindBeatIndex(AddedBeatId) != INDEX_NONE);

	const int32 AddedConditionIndex = Model.AddBeatObject(
		TEXT("Intro"),
		ESigilStoryBeatObjectList::EnterConditions,
		USigilDialogueEditorTestCondition::StaticClass(),
		Error);
	TestEqual(TEXT("新增进入条件必须返回稳定索引"), AddedConditionIndex, 1);
	TestTrue(
		TEXT("新增进入条件必须归属 Story 资产"),
		Asset->Beats[IntroIndex].EnterConditions[AddedConditionIndex]->GetOuter() == Asset);
	TestEqual(
		TEXT("错误类型不能加入完成事件"),
		Model.AddBeatObject(
			TEXT("Intro"),
			ESigilStoryBeatObjectList::CompleteEvents,
			USigilDialogueEditorTestCondition::StaticClass(),
			Error),
		INDEX_NONE);
	TestTrue(
		TEXT("编辑器模型必须可以删除进入条件"),
		Model.RemoveBeatObject(
			TEXT("Intro"),
			ESigilStoryBeatObjectList::EnterConditions,
			AddedConditionIndex));
	TestEqual(TEXT("删除后进入条件数量必须恢复"), Asset->Beats[IntroIndex].EnterConditions.Num(), 1);
	TestTrue(TEXT("删除进入条件必须可以撤销"), GEditor->UndoTransaction());
	TestEqual(TEXT("撤销删除必须恢复进入条件"), Asset->Beats[IntroIndex].EnterConditions.Num(), 2);
	TestTrue(TEXT("删除进入条件必须可以重做"), GEditor->RedoTransaction());
	TestEqual(TEXT("重做删除必须再次移除进入条件"), Asset->Beats[IntroIndex].EnterConditions.Num(), 1);

	const FName FirstCopyId = Model.DuplicateBeat(TEXT("Intro"));
	const FName SecondCopyId = Model.DuplicateBeat(TEXT("Intro"));
	TestEqual(TEXT("第一次复制 ID 必须稳定"), FirstCopyId, FName(TEXT("Intro_Copy")));
	TestEqual(TEXT("第二次复制 ID 必须稳定"), SecondCopyId, FName(TEXT("Intro_Copy_2")));
	const FSigilStoryBeatDefinition* FirstCopy = Asset->FindBeat(FirstCopyId);
	if (TestNotNull(TEXT("第一次复制 Beat 必须存在"), FirstCopy))
	{
		TestEqual(TEXT("复制必须保留进入条件"), FirstCopy->EnterConditions.Num(), 1);
		TestEqual(TEXT("复制必须保留进入事件"), FirstCopy->EnterEvents.Num(), 1);
		TestEqual(TEXT("复制必须保留完成事件"), FirstCopy->CompleteEvents.Num(), 1);
		if (FirstCopy->EnterConditions.Num() == 1)
		{
			TestTrue(
				TEXT("复制的 Instanced Condition 必须是独立对象"),
				FirstCopy->EnterConditions[0] != IntroBeat.EnterConditions[0]);
			TestTrue(
				TEXT("复制的 Instanced Condition 必须归属 Story 资产"),
				FirstCopy->EnterConditions[0]->GetOuter() == Asset);
		}
	}

	const int32 FirstCopyIndex = Model.FindBeatIndex(FirstCopyId);
	Asset->Beats[FirstCopyIndex].BeatId = TEXT("ChapterOne");
	TestTrue(
		TEXT("合法 Beat 改名必须成功"),
		Model.ReconcileBeatEdit(FirstCopyIndex, FirstCopyId, Error));
	TestNotNull(TEXT("改名后的 Beat 必须存在"), Asset->FindBeat(TEXT("ChapterOne")));

	const int32 ChapterIndex = Model.FindBeatIndex(TEXT("ChapterOne"));
	Asset->Beats[ChapterIndex].BeatId = NAME_None;
	TestFalse(
		TEXT("空 BeatId 必须被拒绝"),
		Model.ReconcileBeatEdit(ChapterIndex, TEXT("ChapterOne"), Error));
	TestEqual(
		TEXT("空 BeatId 被拒后必须恢复"),
		Asset->Beats[ChapterIndex].BeatId,
		FName(TEXT("ChapterOne")));

	TestTrue(TEXT("Beat 可以删除"), Model.DeleteBeat(AddedBeatId));
	TestEqual(TEXT("删除后 Beat 必须不存在"), Model.FindBeatIndex(AddedBeatId), INDEX_NONE);
	TestTrue(TEXT("删除 Beat 必须可以撤销"), GEditor->UndoTransaction());
	TestTrue(TEXT("撤销删除必须恢复 Beat"), Model.FindBeatIndex(AddedBeatId) != INDEX_NONE);

	const TArray<FName> FilteredBeatIds = Model.GetFilteredBeatIds(TEXT("copy"));
	TestEqual(TEXT("过滤必须忽略大小写并返回一个未改名复制 Beat"), FilteredBeatIds.Num(), 1);
	TestTrue(TEXT("过滤结果必须包含第二次复制"), FilteredBeatIds.Contains(SecondCopyId));
	TestTrue(TEXT("模型变化必须广播"), ChangeCount >= 5);

	Model.OnModelChanged().Remove(ChangeHandle);
	return !HasAnyErrors();
}

#endif
