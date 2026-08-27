// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "SigilDialogueAsset.h"
#include "SigilDialogueSession.h"
#include "SigilNarrativeFlagCondition.h"
#include "SigilNarrativeSetFlagEvent.h"
#include "SigilNarrativeSubsystem.h"
#include "SigilNarrativeCatalog.h"
#include "Tests/SigilNarrativeSnapshotTestTypes.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeDialogueBranchingSessionTest,
	"SigilNarrative.Dialogue.BranchingSession",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeDialogueConditionAndEventTest,
	"SigilNarrative.Dialogue.ConditionAndEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeDialogueCancelStopsSessionTest,
	"SigilNarrative.Dialogue.CancelStopsSession",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeDialogueRejectsInvalidDefinitionTest,
	"SigilNarrative.Dialogue.RejectsInvalidDefinition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeDialogueCallbackReentrancyTest,
	"SigilNarrative.Dialogue.CallbackReentrancyAndSnapshotAreBlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilNarrativeDialogueBranchingSessionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilDialogueAsset* DialogueAsset = NewObject<USigilDialogueAsset>(GetTransientPackage());
	USigilDialogueSession* Session = NewObject<USigilDialogueSession>(GameInstance);
	UObject* ContextObject = GameInstance;
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Dialogue asset should be constructible"), DialogueAsset);
	TestNotNull(TEXT("Dialogue session should be constructible"), Session);
	if (!NarrativeSubsystem || !DialogueAsset || !Session)
	{
		return false;
	}

	DialogueAsset->DialogueId = TEXT("Dialogue.BranchingSession");
	DialogueAsset->EntryNodeId = TEXT("Start");

	FSigilDialogueNode StartNode;
	StartNode.NodeId = TEXT("Start");
	StartNode.NodeType = ESigilDialogueNodeType::Line;
	StartNode.SpeakerId = TEXT("Speaker.Guide");
	StartNode.Text = FText::FromString(TEXT("Choose your path."));
	StartNode.NextNodeId = TEXT("Pick");

	FSigilDialogueOption ContinueOption;
	ContinueOption.OptionId = TEXT("Continue");
	ContinueOption.Text = FText::FromString(TEXT("Continue."));
	ContinueOption.TargetNodeId = TEXT("Result");

	FSigilDialogueNode PickNode;
	PickNode.NodeId = TEXT("Pick");
	PickNode.NodeType = ESigilDialogueNodeType::Choice;
	PickNode.SpeakerId = TEXT("Speaker.Guide");
	PickNode.Text = FText::FromString(TEXT("Make a choice."));
	PickNode.Options.Add(ContinueOption);

	FSigilDialogueNode ResultNode;
	ResultNode.NodeId = TEXT("Result");
	ResultNode.NodeType = ESigilDialogueNodeType::Line;
	ResultNode.SpeakerId = TEXT("Speaker.Guide");
	ResultNode.Text = FText::FromString(TEXT("You continued."));
	ResultNode.NextNodeId = TEXT("End");

	FSigilDialogueNode EndNode;
	EndNode.NodeId = TEXT("End");
	EndNode.NodeType = ESigilDialogueNodeType::End;
	EndNode.SpeakerId = TEXT("Speaker.Guide");
	EndNode.Text = FText::GetEmpty();

	DialogueAsset->Nodes = { StartNode, PickNode, ResultNode, EndNode };

	TestTrue(TEXT("Starting the dialogue should succeed"), Session->Start(DialogueAsset, NarrativeSubsystem, ContextObject));
	TestEqual(TEXT("Start should select the entry node"), Session->GetCurrentNodeId(), FName(TEXT("Start")));
	TestTrue(TEXT("The session should be active after Start"), Session->IsActive());
	TestFalse(TEXT("The session should not be completed after Start"), Session->IsCompleted());
	TestTrue(TEXT("Advancing a line should enter Pick"), Session->Advance());
	TestEqual(TEXT("Line Advance should enter Pick"), Session->GetCurrentNodeId(), FName(TEXT("Pick")));
	TestFalse(TEXT("An unknown option should be rejected"), Session->Choose(TEXT("Missing")));
	TestEqual(TEXT("A rejected option should leave Pick selected"), Session->GetCurrentNodeId(), FName(TEXT("Pick")));
	TestTrue(TEXT("The valid option should enter Result"), Session->Choose(TEXT("Continue")));
	TestEqual(TEXT("A valid option should enter Result"), Session->GetCurrentNodeId(), FName(TEXT("Result")));
	TestTrue(TEXT("Advancing Result should enter End"), Session->Advance());
	TestEqual(TEXT("The session should finish at End"), Session->GetCurrentNodeId(), FName(TEXT("End")));
	TestFalse(TEXT("The session should no longer be active at End"), Session->IsActive());
	TestTrue(TEXT("The session should be completed at End"), Session->IsCompleted());
	TestFalse(TEXT("Advance should fail after completion"), Session->Advance());
	TestFalse(TEXT("Choose should fail after completion"), Session->Choose(TEXT("Continue")));

	return true;
}

bool FSigilNarrativeDialogueConditionAndEventTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilDialogueAsset* DialogueAsset = NewObject<USigilDialogueAsset>(GetTransientPackage());
	USigilDialogueSession* Session = NewObject<USigilDialogueSession>(GameInstance);
	UObject* ContextObject = GameInstance;
	USigilNarrativeFlagCondition* TrustCondition = NewObject<USigilNarrativeFlagCondition>(GetTransientPackage());
	USigilNarrativeSetFlagEvent* ChosenEvent = NewObject<USigilNarrativeSetFlagEvent>(GetTransientPackage());
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Dialogue asset should be constructible"), DialogueAsset);
	TestNotNull(TEXT("Dialogue session should be constructible"), Session);
	TestNotNull(TEXT("Trust condition should be constructible"), TrustCondition);
	TestNotNull(TEXT("Chosen event should be constructible"), ChosenEvent);
	if (!NarrativeSubsystem || !DialogueAsset || !Session || !TrustCondition || !ChosenEvent)
	{
		return false;
	}

	TrustCondition->Flag = TEXT("Flag.Trust");
	TrustCondition->bNegate = false;
	ChosenEvent->Flag = TEXT("Flag.Chosen");
	ChosenEvent->bEnabled = true;

	DialogueAsset->DialogueId = TEXT("Dialogue.ConditionAndEvent");
	DialogueAsset->EntryNodeId = TEXT("Pick");

	FSigilDialogueOption TrustOption;
	TrustOption.OptionId = TEXT("Trust");
	TrustOption.Text = FText::FromString(TEXT("Trust the guide."));
	TrustOption.TargetNodeId = TEXT("End");
	TrustOption.Conditions.Add(TrustCondition);
	TrustOption.Events.Add(ChosenEvent);

	FSigilDialogueNode PickNode;
	PickNode.NodeId = TEXT("Pick");
	PickNode.NodeType = ESigilDialogueNodeType::Choice;
	PickNode.SpeakerId = TEXT("Speaker.Guide");
	PickNode.Text = FText::FromString(TEXT("Do you trust the guide?"));
	PickNode.Options.Add(TrustOption);

	FSigilDialogueNode EndNode;
	EndNode.NodeId = TEXT("End");
	EndNode.NodeType = ESigilDialogueNodeType::End;
	EndNode.SpeakerId = TEXT("Speaker.Guide");
	EndNode.Text = FText::GetEmpty();

	DialogueAsset->Nodes = { PickNode, EndNode };

	TestTrue(TEXT("Starting the dialogue should succeed"), Session->Start(DialogueAsset, NarrativeSubsystem, ContextObject));
	TestFalse(TEXT("Trust should be absent by default"), NarrativeSubsystem->HasFlag(TEXT("Flag.Trust")));
	TestFalse(TEXT("Chosen should be absent by default"), NarrativeSubsystem->HasFlag(TEXT("Flag.Chosen")));
	TestFalse(TEXT("A missing Trust flag should reject the option"), Session->Choose(TEXT("Trust")));
	TestEqual(TEXT("A rejected option should leave Pick selected"), Session->GetCurrentNodeId(), FName(TEXT("Pick")));
	TestFalse(TEXT("A rejected option should not set Chosen"), NarrativeSubsystem->HasFlag(TEXT("Flag.Chosen")));
	NarrativeSubsystem->SetFlag(TEXT("Flag.Trust"), true);
	TestTrue(TEXT("A present Trust flag should allow the option"), Session->Choose(TEXT("Trust")));
	TestTrue(TEXT("Choosing the option should set Chosen"), NarrativeSubsystem->HasFlag(TEXT("Flag.Chosen")));
	TestEqual(TEXT("Choosing the option should enter End"), Session->GetCurrentNodeId(), FName(TEXT("End")));
	TestFalse(TEXT("Choose should fail after leaving the Choice node"), Session->Choose(TEXT("Trust")));
	TestEqual(TEXT("A repeated Choose should leave End selected"), Session->GetCurrentNodeId(), FName(TEXT("End")));
	TestTrue(TEXT("A repeated Choose should not change Chosen"), NarrativeSubsystem->HasFlag(TEXT("Flag.Chosen")));

	return true;
}

bool FSigilNarrativeDialogueCancelStopsSessionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilDialogueAsset* DialogueAsset = NewObject<USigilDialogueAsset>(GetTransientPackage());
	USigilDialogueSession* Session = NewObject<USigilDialogueSession>(GameInstance);
	UObject* ContextObject = GameInstance;
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Dialogue asset should be constructible"), DialogueAsset);
	TestNotNull(TEXT("Dialogue session should be constructible"), Session);
	if (!NarrativeSubsystem || !DialogueAsset || !Session)
	{
		return false;
	}

	DialogueAsset->DialogueId = TEXT("Dialogue.CancelStopsSession");
	DialogueAsset->EntryNodeId = TEXT("Start");

	FSigilDialogueNode StartNode;
	StartNode.NodeId = TEXT("Start");
	StartNode.NodeType = ESigilDialogueNodeType::Line;
	StartNode.Text = FText::FromString(TEXT("Wait here."));
	StartNode.NextNodeId = TEXT("End");

	FSigilDialogueNode EndNode;
	EndNode.NodeId = TEXT("End");
	EndNode.NodeType = ESigilDialogueNodeType::End;

	DialogueAsset->Nodes = { StartNode, EndNode };

	TestTrue(TEXT("Starting the dialogue should succeed"), Session->Start(DialogueAsset, NarrativeSubsystem, ContextObject));
	Session->Cancel();
	TestFalse(TEXT("Cancel should stop the session"), Session->IsActive());
	TestFalse(TEXT("Cancel should not complete the session"), Session->IsCompleted());
	TestFalse(TEXT("Advance should fail after Cancel"), Session->Advance());
	TestFalse(TEXT("Choose should fail after Cancel"), Session->Choose(TEXT("Missing")));

	return true;
}

bool FSigilNarrativeDialogueRejectsInvalidDefinitionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilDialogueAsset* DialogueAsset = NewObject<USigilDialogueAsset>(GetTransientPackage());
	USigilDialogueSession* Session = NewObject<USigilDialogueSession>(GameInstance);
	UObject* ContextObject = GameInstance;
	TestNotNull(TEXT("Narrative subsystem should be constructible"), NarrativeSubsystem);
	TestNotNull(TEXT("Dialogue asset should be constructible"), DialogueAsset);
	TestNotNull(TEXT("Dialogue session should be constructible"), Session);
	if (!NarrativeSubsystem || !DialogueAsset || !Session)
	{
		return false;
	}

	DialogueAsset->DialogueId = TEXT("Dialogue.RejectsInvalidDefinition");
	DialogueAsset->EntryNodeId = TEXT("Duplicate");

	FSigilDialogueNode FirstNode;
	FirstNode.NodeId = TEXT("Duplicate");
	FirstNode.NodeType = ESigilDialogueNodeType::Line;
	FirstNode.NextNodeId = TEXT("Duplicate");

	FSigilDialogueNode SecondNode;
	SecondNode.NodeId = TEXT("Duplicate");
	SecondNode.NodeType = ESigilDialogueNodeType::End;

	DialogueAsset->Nodes = { FirstNode, SecondNode };

	TestFalse(TEXT("A duplicate NodeId definition should be rejected"), Session->Start(DialogueAsset, NarrativeSubsystem, ContextObject));
	TestFalse(TEXT("A rejected definition should leave the session inactive"), Session->IsActive());
	TestFalse(TEXT("A rejected definition should leave the session incomplete"), Session->IsCompleted());

	FSigilDialogueNode InvalidTypeNode;
	InvalidTypeNode.NodeId = TEXT("InvalidType");
	InvalidTypeNode.NodeType = static_cast<ESigilDialogueNodeType>(255);
	DialogueAsset->EntryNodeId = InvalidTypeNode.NodeId;
	DialogueAsset->Nodes = { InvalidTypeNode };

	TestFalse(TEXT("An unknown node type should be rejected"), Session->Start(DialogueAsset, NarrativeSubsystem, ContextObject));
	TestFalse(TEXT("An unknown node type should leave the session inactive"), Session->IsActive());
	TestFalse(TEXT("An unknown node type should leave the session incomplete"), Session->IsCompleted());

	return true;
}

bool FSigilNarrativeDialogueCallbackReentrancyTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilNarrativeSubsystem* NarrativeSubsystem = NewObject<USigilNarrativeSubsystem>(GameInstance);
	USigilDialogueAsset* DialogueAsset = NewObject<USigilDialogueAsset>(GetTransientPackage());
	USigilDialogueSession* Session = NewObject<USigilDialogueSession>(GameInstance);
	USigilNarrativeCatalog* Catalog = NewObject<USigilNarrativeCatalog>(GetTransientPackage());
	USigilNarrativeSnapshotTestEvent* CallbackEvent = NewObject<USigilNarrativeSnapshotTestEvent>(GetTransientPackage());
	if (!NarrativeSubsystem || !DialogueAsset || !Session || !Catalog || !CallbackEvent)
	{
		AddError(TEXT("Dialogue callback test fixtures should be constructible."));
		return false;
	}

	DialogueAsset->DialogueId = TEXT("Dialogue.CallbackReentrancy");
	DialogueAsset->EntryNodeId = TEXT("Pick");

	FSigilDialogueOption ContinueOption;
	ContinueOption.OptionId = TEXT("Continue");
	ContinueOption.Text = FText::FromString(TEXT("Continue."));
	ContinueOption.TargetNodeId = TEXT("End");
	ContinueOption.Events.Add(CallbackEvent);

	FSigilDialogueNode PickNode;
	PickNode.NodeId = TEXT("Pick");
	PickNode.NodeType = ESigilDialogueNodeType::Choice;
	PickNode.Options.Add(ContinueOption);

	FSigilDialogueNode EndNode;
	EndNode.NodeId = TEXT("End");
	EndNode.NodeType = ESigilDialogueNodeType::End;
	DialogueAsset->Nodes = { PickNode, EndNode };

	CallbackEvent->bExportDuringExecute = true;
	CallbackEvent->bImportDuringExecute = true;
	CallbackEvent->ImportJson = TEXT("{\"SchemaVersion\":1,\"Flags\":[],\"Quests\":[],\"Stories\":[]}");
	CallbackEvent->ImportCatalog = Catalog;
	CallbackEvent->DialogueSession = Session;
	CallbackEvent->ReentrantOptionId = ContinueOption.OptionId;

	TestTrue(TEXT("Starting the callback dialogue should succeed"), Session->Start(DialogueAsset, NarrativeSubsystem, GameInstance));
	TestTrue(TEXT("The outer choice should succeed"), Session->Choose(ContinueOption.OptionId));
	TestEqual(TEXT("The callback event should execute once"), CallbackEvent->ExecuteCount, 1);
	TestFalse(TEXT("The same session should reject a reentrant choice"), CallbackEvent->bLastReentrantChooseResult);
	TestFalse(TEXT("Snapshot export should be rejected during a dialogue callback"), CallbackEvent->bLastExportResult);
	TestTrue(TEXT("Rejected callback export should leave no JSON"), CallbackEvent->LastExportJson.IsEmpty());
	TestFalse(TEXT("Snapshot import should be rejected during a dialogue callback"), CallbackEvent->bLastImportResult);
	TestTrue(TEXT("The outer choice should still finish at End"), Session->IsCompleted());

	USigilDialogueSession* CancelSession = NewObject<USigilDialogueSession>(GameInstance);
	USigilNarrativeSnapshotTestEvent* CancelEvent = NewObject<USigilNarrativeSnapshotTestEvent>(GetTransientPackage());
	if (!CancelSession || !CancelEvent)
	{
		AddError(TEXT("Dialogue cancel callback fixtures should be constructible."));
		return false;
	}

	DialogueAsset->Nodes[0].Options[0].Events = { CancelEvent };
	CancelEvent->DialogueSession = CancelSession;
	CancelEvent->bCancelDialogueDuringExecute = true;
	TestTrue(TEXT("Starting the cancel callback dialogue should succeed"), CancelSession->Start(DialogueAsset, NarrativeSubsystem, GameInstance));
	TestFalse(TEXT("A callback Cancel should interrupt the outer choice"), CancelSession->Choose(ContinueOption.OptionId));
	TestEqual(TEXT("The cancel callback should execute once"), CancelEvent->ExecuteCount, 1);
	TestFalse(TEXT("A callback Cancel should leave the session inactive"), CancelSession->IsActive());
	TestFalse(TEXT("A callback Cancel should not mark the session completed"), CancelSession->IsCompleted());

	return true;
}

#endif
