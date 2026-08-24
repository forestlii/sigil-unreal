// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "SigilNarrativeFlagCondition.h"
#include "SigilNarrativeSetFlagEvent.h"
#include "SigilNarrativeSubsystem.h"
#include "SigilQuestAsset.h"
#include "Tests/SigilQuestTestTypes.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeQuestRequiresCompletedTasksTest,
	"SigilNarrative.Quest.RequiresCompletedTasks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeQuestTerminalStateIsStableTest,
	"SigilNarrative.Quest.TerminalStateIsStable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeQuestRejectsInvalidDefinitionTest,
	"SigilNarrative.Quest.RejectsInvalidDefinition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeQuestEventOrderAndContextTest,
	"SigilNarrative.Quest.EventOrderAndContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeQuestCallbackReentrancyIsSafeTest,
	"SigilNarrative.Quest.CallbackReentrancyIsSafe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilNarrativeQuestRequiresCompletedTasksTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilQuestAsset* QuestAsset = NewObject<USigilQuestAsset>(GetTransientPackage());
	USigilNarrativeFlagCondition* ReadyCondition = NewObject<USigilNarrativeFlagCondition>(QuestAsset);
	USigilNarrativeSetFlagEvent* TransitionEvent = NewObject<USigilNarrativeSetFlagEvent>(QuestAsset);
	USigilNarrativeSetFlagEvent* DoneEntryEvent = NewObject<USigilNarrativeSetFlagEvent>(QuestAsset);
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Quest asset should be constructible"), QuestAsset);
	TestNotNull(TEXT("Ready condition should be constructible"), ReadyCondition);
	TestNotNull(TEXT("Transition event should be constructible"), TransitionEvent);
	TestNotNull(TEXT("Done entry event should be constructible"), DoneEntryEvent);
	if (!NarrativeSubsystem || !QuestAsset || !ReadyCondition || !TransitionEvent || !DoneEntryEvent)
	{
		return false;
	}

	ReadyCondition->Flag = TEXT("Flag.Ready");
	TransitionEvent->Flag = TEXT("Flag.TransitionTaken");
	DoneEntryEvent->Flag = TEXT("Flag.EnteredDone");

	FSigilQuestTaskDefinition CollectTask;
	CollectTask.TaskId = TEXT("Collect");
	CollectTask.RequiredCount = 2;

	FSigilQuestTransition FinishTransition;
	FinishTransition.TransitionId = TEXT("Finish");
	FinishTransition.TargetStateId = TEXT("Done");
	FinishTransition.RequiredTaskIds.Add(CollectTask.TaskId);
	FinishTransition.Conditions.Add(ReadyCondition);
	FinishTransition.Events.Add(TransitionEvent);

	FSigilQuestState WorkState;
	WorkState.StateId = TEXT("Work");
	WorkState.StateType = ESigilQuestStateType::Regular;
	WorkState.Tasks.Add(CollectTask);
	WorkState.Transitions.Add(FinishTransition);

	FSigilQuestState DoneState;
	DoneState.StateId = TEXT("Done");
	DoneState.StateType = ESigilQuestStateType::Success;
	DoneState.EntryEvents.Add(DoneEntryEvent);

	QuestAsset->QuestId = TEXT("Quest.RequiresCompletedTasks");
	QuestAsset->InitialStateId = WorkState.StateId;
	QuestAsset->States = { WorkState, DoneState };

	TestTrue(TEXT("Starting a valid quest should succeed"), NarrativeSubsystem->StartQuest(QuestAsset, GameInstance));
	TestEqual(TEXT("Start should enter Work"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Work")));
	TestEqual(TEXT("A started quest should be active"), NarrativeSubsystem->GetQuestStatus(QuestAsset->QuestId), ESigilQuestStatus::Active);
	TestEqual(TEXT("Collect should start at zero"), NarrativeSubsystem->GetQuestTaskProgress(QuestAsset->QuestId, CollectTask.TaskId), 0);

	TestTrue(TEXT("Positive progress should be accepted"), NarrativeSubsystem->AddQuestTaskProgress(QuestAsset->QuestId, CollectTask.TaskId, 1));
	TestEqual(TEXT("Collect should have one unit"), NarrativeSubsystem->GetQuestTaskProgress(QuestAsset->QuestId, CollectTask.TaskId), 1);
	TestFalse(TEXT("Finish should reject incomplete Collect"), NarrativeSubsystem->TryTakeQuestTransition(QuestAsset->QuestId, FinishTransition.TransitionId, GameInstance));
	TestEqual(TEXT("Rejected progress gate should leave Work active"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Work")));
	TestFalse(TEXT("Rejected progress gate should not run the transition event"), NarrativeSubsystem->HasFlag(TEXT("Flag.TransitionTaken")));
	TestFalse(TEXT("Rejected progress gate should not run the target entry event"), NarrativeSubsystem->HasFlag(TEXT("Flag.EnteredDone")));

	TestTrue(TEXT("A large positive delta should be accepted"), NarrativeSubsystem->AddQuestTaskProgress(QuestAsset->QuestId, CollectTask.TaskId, 100));
	TestEqual(TEXT("A large delta should clamp Collect to its required count"), NarrativeSubsystem->GetQuestTaskProgress(QuestAsset->QuestId, CollectTask.TaskId), 2);
	TestFalse(TEXT("Finish should reject a missing Ready flag"), NarrativeSubsystem->TryTakeQuestTransition(QuestAsset->QuestId, FinishTransition.TransitionId, GameInstance));
	TestEqual(TEXT("Rejected condition should leave Work active"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Work")));
	TestFalse(TEXT("Rejected condition should not run the transition event"), NarrativeSubsystem->HasFlag(TEXT("Flag.TransitionTaken")));
	TestFalse(TEXT("Rejected condition should not run the target entry event"), NarrativeSubsystem->HasFlag(TEXT("Flag.EnteredDone")));

	NarrativeSubsystem->SetFlag(TEXT("Flag.Ready"));
	TestTrue(TEXT("Finish should succeed after all requirements are met"), NarrativeSubsystem->TryTakeQuestTransition(QuestAsset->QuestId, FinishTransition.TransitionId, GameInstance));
	TestEqual(TEXT("Finish should enter Done"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Done")));
	TestEqual(TEXT("Done should mark the quest succeeded"), NarrativeSubsystem->GetQuestStatus(QuestAsset->QuestId), ESigilQuestStatus::Succeeded);
	TestTrue(TEXT("Finish should run the transition event"), NarrativeSubsystem->HasFlag(TEXT("Flag.TransitionTaken")));
	TestTrue(TEXT("Entering Done should run its entry event"), NarrativeSubsystem->HasFlag(TEXT("Flag.EnteredDone")));

	return true;
}

bool FSigilNarrativeQuestTerminalStateIsStableTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilQuestAsset* QuestAsset = NewObject<USigilQuestAsset>(GetTransientPackage());
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Quest asset should be constructible"), QuestAsset);
	if (!NarrativeSubsystem || !QuestAsset)
	{
		return false;
	}

	FSigilQuestTaskDefinition OptionalTask;
	OptionalTask.TaskId = TEXT("Optional");
	OptionalTask.RequiredCount = 1;

	FSigilQuestTransition FailTransition;
	FailTransition.TransitionId = TEXT("Fail");
	FailTransition.TargetStateId = TEXT("Failed");

	FSigilQuestState StartState;
	StartState.StateId = TEXT("Start");
	StartState.StateType = ESigilQuestStateType::Regular;
	StartState.Tasks.Add(OptionalTask);
	StartState.Transitions.Add(FailTransition);

	FSigilQuestState FailedState;
	FailedState.StateId = TEXT("Failed");
	FailedState.StateType = ESigilQuestStateType::Failure;

	QuestAsset->QuestId = TEXT("Quest.TerminalStateIsStable");
	QuestAsset->InitialStateId = StartState.StateId;
	QuestAsset->States = { StartState, FailedState };

	TestTrue(TEXT("The first Start should succeed"), NarrativeSubsystem->StartQuest(QuestAsset, GameInstance));
	TestEqual(TEXT("The first Start should enter Start"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Start")));
	TestFalse(TEXT("A duplicate Start should be rejected"), NarrativeSubsystem->StartQuest(QuestAsset, GameInstance));
	TestEqual(TEXT("A duplicate Start should not change the state"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Start")));

	TestTrue(TEXT("The explicit Fail transition should succeed"), NarrativeSubsystem->TryTakeQuestTransition(QuestAsset->QuestId, FailTransition.TransitionId, GameInstance));
	TestEqual(TEXT("Fail should enter Failed"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Failed")));
	TestEqual(TEXT("Failed should mark the quest failed"), NarrativeSubsystem->GetQuestStatus(QuestAsset->QuestId), ESigilQuestStatus::Failed);
	TestFalse(TEXT("A terminal quest should reject another transition"), NarrativeSubsystem->TryTakeQuestTransition(QuestAsset->QuestId, FailTransition.TransitionId, GameInstance));
	TestFalse(TEXT("A terminal quest should reject task progress"), NarrativeSubsystem->AddQuestTaskProgress(QuestAsset->QuestId, OptionalTask.TaskId, 1));
	TestEqual(TEXT("Rejected terminal operations should leave Failed selected"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), FName(TEXT("Failed")));
	TestEqual(TEXT("Rejected terminal operations should preserve Failed status"), NarrativeSubsystem->GetQuestStatus(QuestAsset->QuestId), ESigilQuestStatus::Failed);

	return true;
}

bool FSigilNarrativeQuestRejectsInvalidDefinitionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilQuestAsset* QuestAsset = NewObject<USigilQuestAsset>(GetTransientPackage());
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Quest asset should be constructible"), QuestAsset);
	if (!NarrativeSubsystem || !QuestAsset)
	{
		return false;
	}

	FSigilQuestState FirstState;
	FirstState.StateId = TEXT("Duplicate");
	FirstState.StateType = ESigilQuestStateType::Regular;

	FSigilQuestState SecondState;
	SecondState.StateId = TEXT("Duplicate");
	SecondState.StateType = ESigilQuestStateType::Success;

	QuestAsset->QuestId = TEXT("Quest.RejectsInvalidDefinition");
	QuestAsset->InitialStateId = FirstState.StateId;
	QuestAsset->States = { FirstState, SecondState };

	FText ValidationError;
	TestFalse(TEXT("A duplicate StateId definition should fail validation"), QuestAsset->ValidateDefinition(ValidationError));
	TestFalse(TEXT("A rejected definition should explain the validation failure"), ValidationError.IsEmpty());
	TestFalse(TEXT("StartQuest should reject a duplicate StateId definition"), NarrativeSubsystem->StartQuest(QuestAsset, GameInstance));
	TestEqual(TEXT("A rejected definition should not create runtime status"), NarrativeSubsystem->GetQuestStatus(QuestAsset->QuestId), ESigilQuestStatus::NotStarted);
	TestEqual(TEXT("A rejected definition should not create runtime state"), NarrativeSubsystem->GetQuestState(QuestAsset->QuestId), NAME_None);

	return true;
}

bool FSigilNarrativeQuestEventOrderAndContextTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	UObject* ContextObject = GameInstance;
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilQuestAsset* QuestAsset = NewObject<USigilQuestAsset>(GetTransientPackage());
	USigilNarrativeQuestTestProbe* Probe = NewObject<USigilNarrativeQuestTestProbe>(QuestAsset);
	USigilNarrativeQuestTestCondition* Condition = NewObject<USigilNarrativeQuestTestCondition>(QuestAsset);
	USigilNarrativeQuestTestEvent* TransitionEvent = NewObject<USigilNarrativeQuestTestEvent>(QuestAsset);
	USigilNarrativeQuestTestEvent* EntryEvent = NewObject<USigilNarrativeQuestTestEvent>(QuestAsset);
	TestNotNull(TEXT("Context object should be constructible"), ContextObject);
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Quest asset should be constructible"), QuestAsset);
	TestNotNull(TEXT("Test probe should be constructible"), Probe);
	TestNotNull(TEXT("Test condition should be constructible"), Condition);
	TestNotNull(TEXT("Transition event should be constructible"), TransitionEvent);
	TestNotNull(TEXT("Entry event should be constructible"), EntryEvent);
	if (!ContextObject || !NarrativeSubsystem || !QuestAsset || !Probe || !Condition || !TransitionEvent || !EntryEvent)
	{
		return false;
	}

	Condition->Probe = Probe;
	Condition->Label = TEXT("Condition");
	TransitionEvent->Probe = Probe;
	TransitionEvent->Label = TEXT("TransitionEvent");
	EntryEvent->Probe = Probe;
	EntryEvent->Label = TEXT("EntryEvent");

	FSigilQuestTransition FinishTransition;
	FinishTransition.TransitionId = TEXT("Finish");
	FinishTransition.TargetStateId = TEXT("Done");
	FinishTransition.Conditions.Add(Condition);
	FinishTransition.Events.Add(TransitionEvent);

	FSigilQuestState WorkState;
	WorkState.StateId = TEXT("Work");
	WorkState.StateType = ESigilQuestStateType::Regular;
	WorkState.Transitions.Add(FinishTransition);

	FSigilQuestState DoneState;
	DoneState.StateId = TEXT("Done");
	DoneState.StateType = ESigilQuestStateType::Success;
	DoneState.EntryEvents.Add(EntryEvent);

	QuestAsset->QuestId = TEXT("Quest.EventOrderAndContext");
	QuestAsset->InitialStateId = WorkState.StateId;
	QuestAsset->States = { WorkState, DoneState };

	TestTrue(TEXT("The context quest should start"), NarrativeSubsystem->StartQuest(QuestAsset, ContextObject));
	TestTrue(TEXT("The context quest should take Finish"), NarrativeSubsystem->TryTakeQuestTransition(QuestAsset->QuestId, FinishTransition.TransitionId, ContextObject));
	TestEqual(TEXT("Exactly three callbacks should run"), Probe->CallOrder.Num(), 3);
	if (Probe->CallOrder.Num() == 3)
	{
		TestEqual(TEXT("Condition should run first"), Probe->CallOrder[0], FName(TEXT("Condition")));
		TestEqual(TEXT("Transition event should run second"), Probe->CallOrder[1], FName(TEXT("TransitionEvent")));
		TestEqual(TEXT("Target entry event should run third"), Probe->CallOrder[2], FName(TEXT("EntryEvent")));
	}

	const FSigilNarrativeContext* ConditionContext = Probe->Contexts.Find(TEXT("Condition"));
	const FSigilNarrativeContext* TransitionContext = Probe->Contexts.Find(TEXT("TransitionEvent"));
	const FSigilNarrativeContext* EntryContext = Probe->Contexts.Find(TEXT("EntryEvent"));
	TestNotNull(TEXT("Condition context should be recorded"), ConditionContext);
	TestNotNull(TEXT("Transition context should be recorded"), TransitionContext);
	TestNotNull(TEXT("Entry context should be recorded"), EntryContext);
	if (ConditionContext && TransitionContext && EntryContext)
	{
		TestEqual(TEXT("Condition should receive the current subsystem"), ConditionContext->NarrativeSubsystem.Get(), NarrativeSubsystem);
		TestEqual(TEXT("Condition should receive the caller context object"), ConditionContext->ContextObject.Get(), ContextObject);
		TestEqual(TEXT("Condition should receive the QuestId"), ConditionContext->NarrativeId, QuestAsset->QuestId);
		TestEqual(TEXT("Condition should observe the source state"), ConditionContext->NodeId, WorkState.StateId);
		TestEqual(TEXT("Transition event should receive the current subsystem"), TransitionContext->NarrativeSubsystem.Get(), NarrativeSubsystem);
		TestEqual(TEXT("Transition event should receive the caller context object"), TransitionContext->ContextObject.Get(), ContextObject);
		TestEqual(TEXT("Transition event should receive the QuestId"), TransitionContext->NarrativeId, QuestAsset->QuestId);
		TestEqual(TEXT("Transition event should observe the source state"), TransitionContext->NodeId, WorkState.StateId);
		TestEqual(TEXT("Entry event should receive the current subsystem"), EntryContext->NarrativeSubsystem.Get(), NarrativeSubsystem);
		TestEqual(TEXT("Entry event should receive the caller context object"), EntryContext->ContextObject.Get(), ContextObject);
		TestEqual(TEXT("Entry event should receive the QuestId"), EntryContext->NarrativeId, QuestAsset->QuestId);
		TestEqual(TEXT("Entry event should observe the target state"), EntryContext->NodeId, DoneState.StateId);
	}

	return true;
}

bool FSigilNarrativeQuestCallbackReentrancyIsSafeTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilQuestAsset* MainQuest = NewObject<USigilQuestAsset>(GetTransientPackage());
	USigilQuestAsset* OtherQuest = NewObject<USigilQuestAsset>(GetTransientPackage());
	USigilNarrativeQuestTestProbe* Probe = NewObject<USigilNarrativeQuestTestProbe>(MainQuest);
	USigilNarrativeQuestTestEvent* InitialEntryEvent = NewObject<USigilNarrativeQuestTestEvent>(MainQuest);
	USigilNarrativeQuestTestCondition* ReentrantCondition = NewObject<USigilNarrativeQuestTestCondition>(MainQuest);
	USigilNarrativeQuestTestEvent* ReentrantTransitionEvent = NewObject<USigilNarrativeQuestTestEvent>(MainQuest);
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Main quest should be constructible"), MainQuest);
	TestNotNull(TEXT("Other quest should be constructible"), OtherQuest);
	TestNotNull(TEXT("Test probe should be constructible"), Probe);
	TestNotNull(TEXT("Initial entry event should be constructible"), InitialEntryEvent);
	TestNotNull(TEXT("Reentrant condition should be constructible"), ReentrantCondition);
	TestNotNull(TEXT("Reentrant transition event should be constructible"), ReentrantTransitionEvent);
	if (!NarrativeSubsystem || !MainQuest || !OtherQuest || !Probe || !InitialEntryEvent || !ReentrantCondition || !ReentrantTransitionEvent)
	{
		return false;
	}

	FSigilQuestState OtherDoneState;
	OtherDoneState.StateId = TEXT("OtherDone");
	OtherDoneState.StateType = ESigilQuestStateType::Success;
	OtherQuest->QuestId = TEXT("Quest.CallbackReentrancy.Other");
	OtherQuest->InitialStateId = OtherDoneState.StateId;
	OtherQuest->States = { OtherDoneState };

	InitialEntryEvent->Probe = Probe;
	InitialEntryEvent->Label = TEXT("InitialEntry");
	InitialEntryEvent->ReentrantTaskId = TEXT("Counter");
	InitialEntryEvent->ReentrantProgressDelta = 1;

	ReentrantCondition->Probe = Probe;
	ReentrantCondition->Label = TEXT("Condition");
	ReentrantCondition->ReentrantTaskId = TEXT("Counter");
	ReentrantCondition->ReentrantProgressDelta = 1;

	ReentrantTransitionEvent->Probe = Probe;
	ReentrantTransitionEvent->Label = TEXT("TransitionEvent");
	ReentrantTransitionEvent->ReentrantTaskId = TEXT("Counter");
	ReentrantTransitionEvent->ReentrantProgressDelta = 1;
	ReentrantTransitionEvent->ReentrantTransitionId = TEXT("Alternate");
	ReentrantTransitionEvent->QuestsToStart.Add(OtherQuest);

	FSigilQuestTaskDefinition CounterTask;
	CounterTask.TaskId = TEXT("Counter");
	CounterTask.RequiredCount = 3;

	FSigilQuestTransition FinishTransition;
	FinishTransition.TransitionId = TEXT("Finish");
	FinishTransition.TargetStateId = TEXT("Done");
	FinishTransition.Conditions.Add(ReentrantCondition);
	FinishTransition.Events.Add(ReentrantTransitionEvent);

	FSigilQuestTransition AlternateTransition;
	AlternateTransition.TransitionId = TEXT("Alternate");
	AlternateTransition.TargetStateId = TEXT("Failed");

	FSigilQuestState WorkState;
	WorkState.StateId = TEXT("Work");
	WorkState.StateType = ESigilQuestStateType::Regular;
	WorkState.Tasks.Add(CounterTask);
	WorkState.Transitions = { FinishTransition, AlternateTransition };
	WorkState.EntryEvents.Add(InitialEntryEvent);

	FSigilQuestState DoneState;
	DoneState.StateId = TEXT("Done");
	DoneState.StateType = ESigilQuestStateType::Success;

	FSigilQuestState FailedState;
	FailedState.StateId = TEXT("Failed");
	FailedState.StateType = ESigilQuestStateType::Failure;

	MainQuest->QuestId = TEXT("Quest.CallbackReentrancy.Main");
	MainQuest->InitialStateId = WorkState.StateId;
	MainQuest->States = { WorkState, DoneState, FailedState };

	TestTrue(TEXT("The main quest should start"), NarrativeSubsystem->StartQuest(MainQuest, GameInstance));
	TestEqual(TEXT("The main quest should remain in Work after initial entry callbacks"), NarrativeSubsystem->GetQuestState(MainQuest->QuestId), FName(TEXT("Work")));
	TestEqual(TEXT("Initial entry should not mutate progress reentrantly"), NarrativeSubsystem->GetQuestTaskProgress(MainQuest->QuestId, CounterTask.TaskId), 0);
	TestTrue(TEXT("The outer Finish transition should succeed"), NarrativeSubsystem->TryTakeQuestTransition(MainQuest->QuestId, FinishTransition.TransitionId, GameInstance));
	TestEqual(TEXT("The outer transition should deterministically enter Done"), NarrativeSubsystem->GetQuestState(MainQuest->QuestId), FName(TEXT("Done")));
	TestEqual(TEXT("The outer transition should succeed the main quest"), NarrativeSubsystem->GetQuestStatus(MainQuest->QuestId), ESigilQuestStatus::Succeeded);
	TestEqual(TEXT("Initial entry, condition, and transition event should each attempt progress"), Probe->ReentrantProgressResults.Num(), 3);
	for (const bool bProgressResult : Probe->ReentrantProgressResults)
	{
		TestFalse(TEXT("Same-quest progress should be rejected during callbacks"), bProgressResult);
	}
	TestEqual(TEXT("The transition event should attempt one nested transition"), Probe->ReentrantTransitionResults.Num(), 1);
	if (Probe->ReentrantTransitionResults.Num() == 1)
	{
		TestFalse(TEXT("A same-quest nested transition should be rejected"), Probe->ReentrantTransitionResults[0]);
	}
	TestEqual(TEXT("A transition event should still start one different quest"), Probe->StartedQuestCount, 1);
	TestEqual(TEXT("The different quest should reach its own terminal state"), NarrativeSubsystem->GetQuestState(OtherQuest->QuestId), FName(TEXT("OtherDone")));
	TestEqual(TEXT("The different quest should be succeeded"), NarrativeSubsystem->GetQuestStatus(OtherQuest->QuestId), ESigilQuestStatus::Succeeded);

	return true;
}

#endif
