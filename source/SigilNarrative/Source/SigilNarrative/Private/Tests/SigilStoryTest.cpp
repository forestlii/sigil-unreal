// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "SigilNarrativeFlagCondition.h"
#include "SigilNarrativeSetFlagEvent.h"
#include "SigilNarrativeSubsystem.h"
#include "SigilStoryAsset.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeStoryExplicitBeatLifecycleTest,
	"SigilNarrative.Story.ExplicitBeatLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeStoryRejectsInvalidAndCompletedBeatsTest,
	"SigilNarrative.Story.RejectsInvalidAndCompletedBeats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilNarrativeStoryExplicitBeatLifecycleTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilStoryAsset* StoryAsset = NewObject<USigilStoryAsset>(GetTransientPackage());
	USigilNarrativeFlagCondition* ReadyCondition = NewObject<USigilNarrativeFlagCondition>(StoryAsset);
	USigilNarrativeSetFlagEvent* IntroEnterEvent = NewObject<USigilNarrativeSetFlagEvent>(StoryAsset);
	USigilNarrativeSetFlagEvent* IntroCompleteEvent = NewObject<USigilNarrativeSetFlagEvent>(StoryAsset);
	USigilNarrativeSetFlagEvent* AfterIntroEnterEvent = NewObject<USigilNarrativeSetFlagEvent>(StoryAsset);
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Story asset should be constructible"), StoryAsset);
	TestNotNull(TEXT("Ready condition should be constructible"), ReadyCondition);
	TestNotNull(TEXT("Intro enter event should be constructible"), IntroEnterEvent);
	TestNotNull(TEXT("Intro complete event should be constructible"), IntroCompleteEvent);
	TestNotNull(TEXT("AfterIntro enter event should be constructible"), AfterIntroEnterEvent);
	if (!NarrativeSubsystem || !StoryAsset || !ReadyCondition || !IntroEnterEvent || !IntroCompleteEvent || !AfterIntroEnterEvent)
	{
		return false;
	}

	ReadyCondition->Flag = TEXT("Flag.Ready");
	IntroEnterEvent->Flag = TEXT("Flag.IntroEntered");
	IntroCompleteEvent->Flag = TEXT("Flag.IntroCompleted");
	AfterIntroEnterEvent->Flag = TEXT("Flag.AfterIntroEntered");

	FSigilStoryBeatDefinition IntroBeat;
	IntroBeat.BeatId = TEXT("Intro");
	IntroBeat.EnterConditions.Add(ReadyCondition);
	IntroBeat.EnterEvents.Add(IntroEnterEvent);
	IntroBeat.CompleteEvents.Add(IntroCompleteEvent);

	FSigilStoryBeatDefinition AfterIntroBeat;
	AfterIntroBeat.BeatId = TEXT("AfterIntro");
	AfterIntroBeat.EnterEvents.Add(AfterIntroEnterEvent);

	StoryAsset->StoryId = TEXT("Story.ExplicitBeatLifecycle");
	StoryAsset->Beats = { IntroBeat, AfterIntroBeat };

	TestFalse(TEXT("Intro should reject a missing Ready flag"), NarrativeSubsystem->EnterStoryBeat(StoryAsset, IntroBeat.BeatId, GameInstance));
	TestEqual(TEXT("A rejected condition should leave no active beat"), NarrativeSubsystem->GetActiveStoryBeat(StoryAsset->StoryId), NAME_None);
	TestFalse(TEXT("A rejected condition should not run Intro enter events"), NarrativeSubsystem->HasFlag(TEXT("Flag.IntroEntered")));

	NarrativeSubsystem->SetFlag(TEXT("Flag.Ready"));
	TestTrue(TEXT("Intro should enter after Ready is set"), NarrativeSubsystem->EnterStoryBeat(StoryAsset, IntroBeat.BeatId, GameInstance));
	TestEqual(TEXT("Intro should become active"), NarrativeSubsystem->GetActiveStoryBeat(StoryAsset->StoryId), IntroBeat.BeatId);
	TestTrue(TEXT("Entering Intro should run its enter event"), NarrativeSubsystem->HasFlag(TEXT("Flag.IntroEntered")));

	TestFalse(TEXT("AfterIntro should be rejected while Intro is active"), NarrativeSubsystem->EnterStoryBeat(StoryAsset, AfterIntroBeat.BeatId, GameInstance));
	TestEqual(TEXT("A rejected second beat should leave Intro active"), NarrativeSubsystem->GetActiveStoryBeat(StoryAsset->StoryId), IntroBeat.BeatId);
	TestFalse(TEXT("A rejected second beat should not run its enter event"), NarrativeSubsystem->HasFlag(TEXT("Flag.AfterIntroEntered")));

	TestTrue(TEXT("The active Intro beat should complete"), NarrativeSubsystem->CompleteStoryBeat(StoryAsset->StoryId, IntroBeat.BeatId, GameInstance));
	TestEqual(TEXT("Completing Intro should clear the active beat"), NarrativeSubsystem->GetActiveStoryBeat(StoryAsset->StoryId), NAME_None);
	TestTrue(TEXT("Intro should be recorded as completed"), NarrativeSubsystem->IsStoryBeatCompleted(StoryAsset->StoryId, IntroBeat.BeatId));
	TestTrue(TEXT("Completing Intro should run its complete event"), NarrativeSubsystem->HasFlag(TEXT("Flag.IntroCompleted")));
	TestFalse(TEXT("Completing Intro should not auto-enter AfterIntro"), NarrativeSubsystem->HasFlag(TEXT("Flag.AfterIntroEntered")));

	TestTrue(TEXT("AfterIntro should enter only after an explicit call"), NarrativeSubsystem->EnterStoryBeat(StoryAsset, AfterIntroBeat.BeatId, GameInstance));
	TestEqual(TEXT("The explicit call should make AfterIntro active"), NarrativeSubsystem->GetActiveStoryBeat(StoryAsset->StoryId), AfterIntroBeat.BeatId);
	TestTrue(TEXT("Explicitly entering AfterIntro should run its enter event"), NarrativeSubsystem->HasFlag(TEXT("Flag.AfterIntroEntered")));

	return true;
}

bool FSigilNarrativeStoryRejectsInvalidAndCompletedBeatsTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilStoryAsset* InvalidStoryAsset = NewObject<USigilStoryAsset>(GetTransientPackage());
	USigilStoryAsset* ValidStoryAsset = NewObject<USigilStoryAsset>(GetTransientPackage());
	USigilStoryAsset* DifferentStoryAsset = NewObject<USigilStoryAsset>(GetTransientPackage());
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Invalid story asset should be constructible"), InvalidStoryAsset);
	TestNotNull(TEXT("Valid story asset should be constructible"), ValidStoryAsset);
	TestNotNull(TEXT("Different story asset should be constructible"), DifferentStoryAsset);
	if (!NarrativeSubsystem || !InvalidStoryAsset || !ValidStoryAsset || !DifferentStoryAsset)
	{
		return false;
	}

	FSigilStoryBeatDefinition FirstDuplicateBeat;
	FirstDuplicateBeat.BeatId = TEXT("Duplicate");
	FSigilStoryBeatDefinition SecondDuplicateBeat;
	SecondDuplicateBeat.BeatId = TEXT("Duplicate");
	InvalidStoryAsset->StoryId = TEXT("Story.InvalidDefinition");
	InvalidStoryAsset->Beats = { FirstDuplicateBeat, SecondDuplicateBeat };

	FText ValidationError;
	TestFalse(TEXT("A duplicate BeatId definition should fail validation"), InvalidStoryAsset->ValidateDefinition(ValidationError));
	TestFalse(TEXT("A rejected definition should explain the validation failure"), ValidationError.IsEmpty());
	TestFalse(TEXT("EnterStoryBeat should reject a duplicate BeatId definition"), NarrativeSubsystem->EnterStoryBeat(InvalidStoryAsset, FirstDuplicateBeat.BeatId, GameInstance));
	TestEqual(TEXT("A rejected definition should not create an active beat"), NarrativeSubsystem->GetActiveStoryBeat(InvalidStoryAsset->StoryId), NAME_None);
	TestFalse(TEXT("A rejected definition should not create completed state"), NarrativeSubsystem->IsStoryBeatCompleted(InvalidStoryAsset->StoryId, FirstDuplicateBeat.BeatId));

	FSigilStoryBeatDefinition FirstBeat;
	FirstBeat.BeatId = TEXT("First");
	FSigilStoryBeatDefinition SecondBeat;
	SecondBeat.BeatId = TEXT("Second");
	ValidStoryAsset->StoryId = TEXT("Story.CompletedBeatIsStable");
	ValidStoryAsset->Beats = { FirstBeat, SecondBeat };
	DifferentStoryAsset->StoryId = ValidStoryAsset->StoryId;
	DifferentStoryAsset->Beats = { FirstBeat, SecondBeat };

	TestTrue(TEXT("The first valid beat should enter"), NarrativeSubsystem->EnterStoryBeat(ValidStoryAsset, FirstBeat.BeatId, GameInstance));
	TestTrue(TEXT("The active first beat should complete"), NarrativeSubsystem->CompleteStoryBeat(ValidStoryAsset->StoryId, FirstBeat.BeatId, GameInstance));
	TestEqual(TEXT("Completing the first beat should clear the active beat"), NarrativeSubsystem->GetActiveStoryBeat(ValidStoryAsset->StoryId), NAME_None);
	TestTrue(TEXT("The first beat should remain completed"), NarrativeSubsystem->IsStoryBeatCompleted(ValidStoryAsset->StoryId, FirstBeat.BeatId));
	TestFalse(TEXT("The same StoryId should reject a different asset object after the active beat clears"), NarrativeSubsystem->EnterStoryBeat(DifferentStoryAsset, SecondBeat.BeatId, GameInstance));
	TestEqual(TEXT("A rejected different asset should leave no active beat"), NarrativeSubsystem->GetActiveStoryBeat(ValidStoryAsset->StoryId), NAME_None);
	TestFalse(TEXT("A completed beat should not enter again"), NarrativeSubsystem->EnterStoryBeat(ValidStoryAsset, FirstBeat.BeatId, GameInstance));
	TestFalse(TEXT("A completed beat should not complete again"), NarrativeSubsystem->CompleteStoryBeat(ValidStoryAsset->StoryId, FirstBeat.BeatId, GameInstance));
	TestTrue(TEXT("Rejected repeat operations should preserve completed state"), NarrativeSubsystem->IsStoryBeatCompleted(ValidStoryAsset->StoryId, FirstBeat.BeatId));
	TestTrue(TEXT("The original asset should still enter its second beat"), NarrativeSubsystem->EnterStoryBeat(ValidStoryAsset, SecondBeat.BeatId, GameInstance));
	TestEqual(TEXT("The original asset should make its second beat active"), NarrativeSubsystem->GetActiveStoryBeat(ValidStoryAsset->StoryId), SecondBeat.BeatId);

	return true;
}

#endif
