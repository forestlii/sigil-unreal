// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "SigilNarrativeSubsystem.h"
#include "SigilStoryAsset.h"
#include "Tests/SigilNarrativePresentationTestTypes.h"
#include "UObject/Package.h"

namespace
{
	USigilStoryAsset* MakePresentationStory(
		const FName StoryId,
		const FName BeatId,
		USigilNarrativePresentationAsset* Presentation)
	{
		USigilStoryAsset* Story = NewObject<USigilStoryAsset>(GetTransientPackage());
		Story->StoryId = StoryId;

		FSigilStoryBeatDefinition Beat;
		Beat.BeatId = BeatId;
		Beat.Presentation = Presentation;
		Story->Beats.Add(Beat);
		return Story;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativePresentationStoryLifecycleTest,
	"SigilNarrative.Presentation.StoryLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilNarrativePresentationStoryLifecycleTest::RunTest(const FString& Parameters)
{
	const FName BeatId = TEXT("Beat");
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* Narrative = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilNarrativePresentationTestHost* Host = NewObject<USigilNarrativePresentationTestHost>(GameInstance);
	USigilNarrativePresentationTestAsset* Presentation =
		NewObject<USigilNarrativePresentationTestAsset>(GetTransientPackage());
	if (!TestNotNull(TEXT("Narrative subsystem should exist"), Narrative)
		|| !TestNotNull(TEXT("Presentation host should exist"), Host)
		|| !TestNotNull(TEXT("Presentation asset should exist"), Presentation))
	{
		return false;
	}

	USigilStoryAsset* MissingHostStory = MakePresentationStory(
		TEXT("Story.Presentation.MissingHost"), BeatId, Presentation);
	TestTrue(TEXT("Missing-host story should enter its beat"),
		Narrative->EnterStoryBeat(MissingHostStory, BeatId, GameInstance));
	TestFalse(TEXT("A presentation cannot begin without a registered host"),
		Narrative->BeginStoryPresentation(MissingHostStory->StoryId, BeatId, GameInstance).IsValid());
	TestTrue(TEXT("A valid host should register"), Narrative->RegisterPresentationHost(Host));

	Host->bCanBegin = false;
	TestFalse(TEXT("A host preflight rejection should return no handle"),
		Narrative->BeginStoryPresentation(MissingHostStory->StoryId, BeatId, GameInstance).IsValid());
	TestFalse(TEXT("A rejected presentation should not become active"), Narrative->HasActivePresentation());
	Host->bCanBegin = true;

	USigilStoryAsset* CompletedStory = MakePresentationStory(
		TEXT("Story.Presentation.Completed"), BeatId, Presentation);
	TestTrue(TEXT("Completed story should enter its beat"),
		Narrative->EnterStoryBeat(CompletedStory, BeatId, GameInstance));
	const FSigilNarrativePresentationHandle CompletedHandle =
		Narrative->BeginStoryPresentation(CompletedStory->StoryId, BeatId, GameInstance);
	TestTrue(TEXT("Completed presentation should receive a valid handle"), CompletedHandle.IsValid());
	TestTrue(TEXT("A presentation should be active after begin"), Narrative->HasActivePresentation());

	FSigilNarrativePresentationHandle WrongGeneration = CompletedHandle;
	++WrongGeneration.Generation;
	TestFalse(TEXT("A mismatched generation should be rejected"), Narrative->ResolveStoryPresentation(
		WrongGeneration, ESigilNarrativePresentationResult::Completed, GameInstance));
	TestTrue(TEXT("A rejected generation should preserve the active presentation"), Narrative->HasActivePresentation());
	TestFalse(TEXT("A rejected generation should not complete the beat"),
		Narrative->IsStoryBeatCompleted(CompletedStory->StoryId, BeatId));

	TestTrue(TEXT("Completed should resolve the active presentation"), Narrative->ResolveStoryPresentation(
		CompletedHandle, ESigilNarrativePresentationResult::Completed, GameInstance));
	TestTrue(TEXT("Completed should complete the beat"),
		Narrative->IsStoryBeatCompleted(CompletedStory->StoryId, BeatId));
	TestFalse(TEXT("A resolved presentation should no longer be active"), Narrative->HasActivePresentation());
	TestFalse(TEXT("A late duplicate result should be rejected"), Narrative->ResolveStoryPresentation(
		CompletedHandle, ESigilNarrativePresentationResult::Completed, GameInstance));

	USigilStoryAsset* SkippedStory = MakePresentationStory(
		TEXT("Story.Presentation.Skipped"), BeatId, Presentation);
	TestTrue(TEXT("Skipped story should enter its beat"),
		Narrative->EnterStoryBeat(SkippedStory, BeatId, GameInstance));
	const FSigilNarrativePresentationHandle SkippedHandle =
		Narrative->BeginStoryPresentation(SkippedStory->StoryId, BeatId, GameInstance);
	TestTrue(TEXT("Skipped should resolve the active presentation"), Narrative->ResolveStoryPresentation(
		SkippedHandle, ESigilNarrativePresentationResult::Skipped, GameInstance));
	TestTrue(TEXT("Skipped should complete the beat"),
		Narrative->IsStoryBeatCompleted(SkippedStory->StoryId, BeatId));

	for (const ESigilNarrativePresentationResult Result : {
		ESigilNarrativePresentationResult::Cancelled,
		ESigilNarrativePresentationResult::Failed })
	{
		const FName StoryId = Result == ESigilNarrativePresentationResult::Cancelled
			? TEXT("Story.Presentation.Cancelled")
			: TEXT("Story.Presentation.Failed");
		USigilStoryAsset* Story = MakePresentationStory(StoryId, BeatId, Presentation);
		TestTrue(TEXT("Non-completing story should enter its beat"),
			Narrative->EnterStoryBeat(Story, BeatId, GameInstance));
		const FSigilNarrativePresentationHandle Handle =
			Narrative->BeginStoryPresentation(StoryId, BeatId, GameInstance);
		TestTrue(TEXT("Cancelled or failed should resolve the active presentation"),
			Narrative->ResolveStoryPresentation(Handle, Result, GameInstance));
		TestFalse(TEXT("Cancelled or failed should not complete the beat"),
			Narrative->IsStoryBeatCompleted(StoryId, BeatId));
		TestEqual(TEXT("Cancelled or failed should leave the beat active"),
			Narrative->GetActiveStoryBeat(StoryId), BeatId);
	}

	USigilStoryAsset* CancelRequestStory = MakePresentationStory(
		TEXT("Story.Presentation.CancelRequest"), BeatId, Presentation);
	TestTrue(TEXT("Cancel-request story should enter its beat"),
		Narrative->EnterStoryBeat(CancelRequestStory, BeatId, GameInstance));
	const FSigilNarrativePresentationHandle CancelHandle =
		Narrative->BeginStoryPresentation(CancelRequestStory->StoryId, BeatId, GameInstance);
	TestTrue(TEXT("Explicit cancel should accept the current handle"),
		Narrative->CancelStoryPresentation(CancelHandle));
	TestEqual(TEXT("Explicit cancel should notify the host once"), Host->CancelCount, 1);
	TestFalse(TEXT("Explicit cancel should not complete the beat"),
		Narrative->IsStoryBeatCompleted(CancelRequestStory->StoryId, BeatId));
	TestFalse(TEXT("Explicit cancel should clear the active presentation"), Narrative->HasActivePresentation());

	TestTrue(TEXT("The registered host should unregister"), Narrative->UnregisterPresentationHost(Host));
	return true;
}

#endif
