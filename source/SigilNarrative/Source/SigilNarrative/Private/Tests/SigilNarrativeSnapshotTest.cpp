// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "SigilNarrativeCatalog.h"
#include "SigilNarrativeSubsystem.h"
#include "SigilQuestAsset.h"
#include "SigilStoryAsset.h"
#include "Tests/SigilNarrativeSnapshotTestTypes.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeSnapshotRoundTripTest,
	"SigilNarrative.Snapshot.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeSnapshotRejectsInvalidAtomicallyTest,
	"SigilNarrative.Snapshot.RejectsInvalidAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
	USigilQuestAsset* MakeActiveQuestAsset(UObject* Outer, USigilNarrativeEvent* EntryEvent, const FName QuestId)
	{
		USigilQuestAsset* QuestAsset = NewObject<USigilQuestAsset>(Outer);
		QuestAsset->QuestId = QuestId;
		QuestAsset->InitialStateId = TEXT("Work");

		FSigilQuestTaskDefinition CollectTask;
		CollectTask.TaskId = TEXT("Collect");
		CollectTask.RequiredCount = 3;

		FSigilQuestState WorkState;
		WorkState.StateId = QuestAsset->InitialStateId;
		WorkState.StateType = ESigilQuestStateType::Regular;
		WorkState.Tasks.Add(CollectTask);
		if (EntryEvent)
		{
			WorkState.EntryEvents.Add(EntryEvent);
		}

		QuestAsset->States.Add(WorkState);
		return QuestAsset;
	}

	USigilStoryAsset* MakeStoryAsset(UObject* Outer, USigilNarrativeEvent* CompleteEvent, USigilNarrativeEvent* EnterEvent, const FName StoryId)
	{
		USigilStoryAsset* StoryAsset = NewObject<USigilStoryAsset>(Outer);
		StoryAsset->StoryId = StoryId;

		FSigilStoryBeatDefinition IntroBeat;
		IntroBeat.BeatId = TEXT("Intro");
		if (CompleteEvent)
		{
			IntroBeat.CompleteEvents.Add(CompleteEvent);
		}

		FSigilStoryBeatDefinition AfterIntroBeat;
		AfterIntroBeat.BeatId = TEXT("AfterIntro");
		if (EnterEvent)
		{
			AfterIntroBeat.EnterEvents.Add(EnterEvent);
		}

		FSigilStoryBeatDefinition NumericBeat;
		NumericBeat.BeatId = TEXT("7");

		StoryAsset->Beats = { IntroBeat, AfterIntroBeat, NumericBeat };
		return StoryAsset;
	}
}

bool FSigilNarrativeSnapshotRoundTripTest::RunTest(const FString& Parameters)
{
	UGameInstance* SourceGameInstance = NewObject<UGameInstance>(GetTransientPackage());
	UGameInstance* ImportedGameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* SourceSubsystem = NewObject<USigilNarrativeSubsystem>(SourceGameInstance);
	USigilNarrativeSubsystem* ImportedSubsystem = NewObject<USigilNarrativeSubsystem>(ImportedGameInstance);
	USigilNarrativeSnapshotTestEvent* QuestEntryEvent = NewObject<USigilNarrativeSnapshotTestEvent>(GetTransientPackage());
	USigilNarrativeSnapshotTestEvent* StoryCompleteEvent = NewObject<USigilNarrativeSnapshotTestEvent>(GetTransientPackage());
	USigilNarrativeSnapshotTestEvent* StoryEnterEvent = NewObject<USigilNarrativeSnapshotTestEvent>(GetTransientPackage());
	TestNotNull(TEXT("Source subsystem should be constructible"), SourceSubsystem);
	TestNotNull(TEXT("Imported subsystem should be constructible"), ImportedSubsystem);
	if (!SourceSubsystem || !ImportedSubsystem || !QuestEntryEvent || !StoryCompleteEvent || !StoryEnterEvent)
	{
		return false;
	}

	USigilQuestAsset* QuestAsset = MakeActiveQuestAsset(GetTransientPackage(), QuestEntryEvent, TEXT("Quest.RoundTrip"));
	USigilStoryAsset* StoryAsset = MakeStoryAsset(
		GetTransientPackage(),
		StoryCompleteEvent,
		StoryEnterEvent,
		TEXT("Story.RoundTrip"));
	USigilNarrativeCatalog* Catalog = NewObject<USigilNarrativeCatalog>(GetTransientPackage());
	TestNotNull(TEXT("Quest asset should be constructible"), QuestAsset);
	TestNotNull(TEXT("Story asset should be constructible"), StoryAsset);
	TestNotNull(TEXT("Catalog should be constructible"), Catalog);
	if (!QuestAsset || !StoryAsset || !Catalog)
	{
		return false;
	}
	Catalog->QuestAssets.Add(QuestAsset);
	Catalog->StoryAssets.Add(StoryAsset);
	QuestEntryEvent->bExportDuringExecute = true;
	QuestEntryEvent->bImportDuringExecute = true;
	QuestEntryEvent->ImportJson = TEXT("{\"SchemaVersion\":1,\"Flags\":[],\"Quests\":[],\"Stories\":[]}");
	QuestEntryEvent->ImportCatalog = Catalog;
	QuestEntryEvent->LastExportJson = TEXT("must be cleared");

	SourceSubsystem->SetFlag(TEXT("Flag.Saved"));
	TestTrue(TEXT("Active quest should start"), SourceSubsystem->StartQuest(QuestAsset, SourceGameInstance));
	TestEqual(TEXT("Quest entry event should execute once"), QuestEntryEvent->ExecuteCount, 1);
	TestFalse(TEXT("Export should reject a quest callback in progress"), QuestEntryEvent->bLastExportResult);
	TestTrue(TEXT("A rejected callback export should clear its output"), QuestEntryEvent->LastExportJson.IsEmpty());
	TestFalse(TEXT("Import should reject a quest callback in progress"), QuestEntryEvent->bLastImportResult);
	TestTrue(TEXT("Quest progress should advance"), SourceSubsystem->AddQuestTaskProgress(QuestAsset->QuestId, TEXT("Collect"), 2));
	TestTrue(TEXT("Intro should enter"), SourceSubsystem->EnterStoryBeat(StoryAsset, TEXT("Intro"), SourceGameInstance));
	TestTrue(TEXT("Intro should complete"), SourceSubsystem->CompleteStoryBeat(StoryAsset->StoryId, TEXT("Intro"), SourceGameInstance));
	TestTrue(TEXT("AfterIntro should enter explicitly"), SourceSubsystem->EnterStoryBeat(StoryAsset, TEXT("AfterIntro"), SourceGameInstance));

	FString FirstJson;
	FString SecondJson;
	TestTrue(TEXT("The first snapshot should export"), SourceSubsystem->ExportSnapshotJson(FirstJson));
	TestTrue(TEXT("The second snapshot should export"), SourceSubsystem->ExportSnapshotJson(SecondJson));
	TestEqual(TEXT("Repeated exports of the same state should be byte-identical"), SecondJson, FirstJson);

	QuestEntryEvent->ExecuteCount = 0;
	StoryCompleteEvent->ExecuteCount = 0;
	StoryEnterEvent->ExecuteCount = 0;
	TestTrue(TEXT("A valid snapshot should import"), ImportedSubsystem->ImportSnapshotJson(FirstJson, Catalog));
	TestTrue(TEXT("Imported flags should match"), ImportedSubsystem->HasFlag(TEXT("Flag.Saved")));
	TestEqual(TEXT("Imported quest status should match"), ImportedSubsystem->GetQuestStatus(QuestAsset->QuestId), ESigilQuestStatus::Active);
	TestEqual(TEXT("Imported quest state should match"), ImportedSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Work")));
	TestEqual(TEXT("Imported quest progress should match"), ImportedSubsystem->GetQuestTaskProgress(QuestAsset->QuestId, TEXT("Collect")), 2);
	TestTrue(TEXT("Imported completed beat should match"), ImportedSubsystem->IsStoryBeatCompleted(StoryAsset->StoryId, TEXT("Intro")));
	TestEqual(TEXT("Imported active beat should match"), ImportedSubsystem->GetActiveStoryBeat(StoryAsset->StoryId), FName(TEXT("AfterIntro")));
	TestEqual(TEXT("Import must not replay the quest entry event"), QuestEntryEvent->ExecuteCount, 0);
	TestEqual(TEXT("Import must not replay the story complete event"), StoryCompleteEvent->ExecuteCount, 0);
	TestEqual(TEXT("Import must not replay the story enter event"), StoryEnterEvent->ExecuteCount, 0);

	FString ImportedJson;
	TestTrue(TEXT("The imported state should export"), ImportedSubsystem->ExportSnapshotJson(ImportedJson));
	TestEqual(TEXT("Round-tripped JSON should remain byte-identical"), ImportedJson, FirstJson);

	return true;
}

bool FSigilNarrativeSnapshotRejectsInvalidAtomicallyTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilQuestAsset* QuestAsset = MakeActiveQuestAsset(GetTransientPackage(), nullptr, TEXT("Quest.Atomic"));
	USigilStoryAsset* StoryAsset = MakeStoryAsset(GetTransientPackage(), nullptr, nullptr, TEXT("Story.Atomic"));
	USigilNarrativeCatalog* Catalog = NewObject<USigilNarrativeCatalog>(GetTransientPackage());
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Catalog should be constructible"), Catalog);
	if (!NarrativeSubsystem || !QuestAsset || !StoryAsset || !Catalog)
	{
		return false;
	}
	Catalog->QuestAssets.Add(QuestAsset);
	Catalog->StoryAssets.Add(StoryAsset);

	NarrativeSubsystem->SetFlag(TEXT("Flag.Preserve"));
	TestTrue(TEXT("Baseline quest should start"), NarrativeSubsystem->StartQuest(QuestAsset, GameInstance));
	TestTrue(TEXT("Baseline quest should advance"), NarrativeSubsystem->AddQuestTaskProgress(QuestAsset->QuestId, TEXT("Collect"), 1));
	TestTrue(TEXT("Baseline story beat should enter"), NarrativeSubsystem->EnterStoryBeat(StoryAsset, TEXT("Intro"), GameInstance));

	FString BaselineJson;
	TestTrue(TEXT("Baseline state should export"), NarrativeSubsystem->ExportSnapshotJson(BaselineJson));

	auto TestRejectedWithoutMutation = [this, NarrativeSubsystem, Catalog, &BaselineJson](const TCHAR* What, const FString& InvalidJson)
	{
		TestFalse(What, NarrativeSubsystem->ImportSnapshotJson(InvalidJson, Catalog));
		FString JsonAfterRejectedImport;
		TestTrue(TEXT("State should still export after a rejected import"), NarrativeSubsystem->ExportSnapshotJson(JsonAfterRejectedImport));
		TestEqual(TEXT("A rejected import must preserve all previous narrative state"), JsonAfterRejectedImport, BaselineJson);
		if (JsonAfterRejectedImport != BaselineJson)
		{
			TestTrue(TEXT("The test fixture should restore its baseline after exposing a mutation"), NarrativeSubsystem->ImportSnapshotJson(BaselineJson, Catalog));
		}
	};

	const FString UnknownQuestJson = TEXT("{\"SchemaVersion\":1,\"Flags\":[],\"Quests\":[{\"QuestId\":\"Quest.Unknown\",\"Status\":\"Active\",\"CurrentStateId\":\"Work\",\"Tasks\":[{\"TaskId\":\"Collect\",\"Count\":0}]}],\"Stories\":[]}");
	TestRejectedWithoutMutation(TEXT("An unknown quest should be rejected"), UnknownQuestJson);

	const FString IllegalCountJson = TEXT("{\"SchemaVersion\":1,\"Flags\":[\"Flag.Replaced\"],\"Quests\":[{\"QuestId\":\"Quest.Atomic\",\"Status\":\"Active\",\"CurrentStateId\":\"Work\",\"Tasks\":[{\"TaskId\":\"Collect\",\"Count\":999}]}],\"Stories\":[]}");
	TestRejectedWithoutMutation(TEXT("An out-of-range task count should be rejected"), IllegalCountJson);

	TestRejectedWithoutMutation(TEXT("Malformed JSON should be rejected"), TEXT("{not-json"));

	const FString NumericBeatJson = TEXT("{\"SchemaVersion\":1,\"Flags\":[],\"Quests\":[],\"Stories\":[{\"StoryId\":\"Story.Atomic\",\"ActiveBeatId\":7,\"CompletedBeatIds\":[]}]}");
	TestRejectedWithoutMutation(TEXT("A numeric BeatId should be rejected instead of converted to a string"), NumericBeatJson);

	const FString NumericFlagJson = TEXT("{\"SchemaVersion\":1,\"Flags\":[7],\"Quests\":[],\"Stories\":[]}");
	TestRejectedWithoutMutation(TEXT("A numeric flag should be rejected instead of converted to a string"), NumericFlagJson);

	const FString LongFlag = FString::ChrN(NAME_SIZE, TEXT('A'));
	const FString LongFlagJson = FString::Printf(
		TEXT("{\"SchemaVersion\":1,\"Flags\":[\"%s\"],\"Quests\":[],\"Stories\":[]}"),
		*LongFlag);
	TestRejectedWithoutMutation(TEXT("An overlong flag should be rejected before constructing an FName"), LongFlagJson);

	return true;
}

#endif
