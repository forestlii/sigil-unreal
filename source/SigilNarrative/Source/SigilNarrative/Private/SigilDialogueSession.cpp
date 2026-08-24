// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilDialogueSession.h"

#include "SigilDialogueAsset.h"
#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"
#include "SigilNarrativeSubsystem.h"
#include "Misc/ScopeExit.h"

bool USigilDialogueSession::Start(
	USigilDialogueAsset* InDialogueAsset,
	USigilNarrativeSubsystem* InNarrativeSubsystem,
	UObject* InContextObject)
{
	if (bActive || bCallbackInProgress)
	{
		return false;
	}

	Cancel();
	FText ValidationError;
	if (!InDialogueAsset || !InNarrativeSubsystem || !InDialogueAsset->ValidateDefinition(ValidationError))
	{
		return false;
	}

	DialogueAsset = InDialogueAsset;
	Context.NarrativeSubsystem = InNarrativeSubsystem;
	Context.ContextObject = InContextObject;
	Context.NarrativeId = InDialogueAsset->DialogueId;
	return EnterNode(InDialogueAsset->EntryNodeId);
}

bool USigilDialogueSession::Advance()
{
	if (!bActive || !DialogueAsset || bCallbackInProgress)
	{
		return false;
	}

	const FSigilDialogueNode* CurrentNode = DialogueAsset->FindNode(Context.NodeId);
	return CurrentNode && CurrentNode->NodeType == ESigilDialogueNodeType::Line && EnterNode(CurrentNode->NextNodeId);
}

bool USigilDialogueSession::Choose(const FName OptionId)
{
	if (!bActive || !DialogueAsset || bCallbackInProgress)
	{
		return false;
	}

	const FSigilDialogueNode* CurrentNode = DialogueAsset->FindNode(Context.NodeId);
	if (!CurrentNode || CurrentNode->NodeType != ESigilDialogueNodeType::Choice)
	{
		return false;
	}

	const FSigilDialogueOption* Option = CurrentNode->Options.FindByPredicate([OptionId](const FSigilDialogueOption& Candidate)
	{
		return Candidate.OptionId == OptionId;
	});
	if (!Option)
	{
		return false;
	}

	USigilNarrativeSubsystem* NarrativeSubsystem = Context.NarrativeSubsystem;
	if (!NarrativeSubsystem)
	{
		return false;
	}

	const TObjectPtr<USigilDialogueAsset> ExpectedDialogueAsset = DialogueAsset;
	const FName SourceNodeId = Context.NodeId;
	const FName TargetNodeId = Option->TargetNodeId;
	const TArray<TObjectPtr<USigilNarrativeCondition>> Conditions = Option->Conditions;
	const TArray<TObjectPtr<USigilNarrativeEvent>> Events = Option->Events;
	bCallbackInProgress = true;
	NarrativeSubsystem->BeginDialogueCallbackDispatch();
	ON_SCOPE_EXIT
	{
		NarrativeSubsystem->EndDialogueCallbackDispatch();
		bCallbackInProgress = false;
	};

	auto IsDispatchStateExpected = [this, ExpectedDialogueAsset, SourceNodeId]()
	{
		return bActive && DialogueAsset == ExpectedDialogueAsset && Context.NodeId == SourceNodeId;
	};

	bool bConditionsMet = true;
	for (const USigilNarrativeCondition* Condition : Conditions)
	{
		const bool bConditionMet = Condition && Condition->IsMet(Context);
		if (!IsDispatchStateExpected() || !bConditionMet)
		{
			bConditionsMet = false;
			break;
		}
	}

	if (bConditionsMet)
	{
		for (USigilNarrativeEvent* Event : Events)
		{
			if (!Event)
			{
				return false;
			}

			Event->Run(Context);
			if (!IsDispatchStateExpected())
			{
				return false;
			}
		}
	}

	return bConditionsMet && EnterNode(TargetNodeId);
}

void USigilDialogueSession::Cancel()
{
	DialogueAsset = nullptr;
	Context = FSigilNarrativeContext();
	bActive = false;
	bCompleted = false;
}

FName USigilDialogueSession::GetCurrentNodeId() const
{
	return Context.NodeId;
}

bool USigilDialogueSession::IsActive() const
{
	return bActive;
}

bool USigilDialogueSession::IsCompleted() const
{
	return bCompleted;
}

bool USigilDialogueSession::EnterNode(const FName NodeId)
{
	if (!DialogueAsset)
	{
		return false;
	}

	const FSigilDialogueNode* Node = DialogueAsset->FindNode(NodeId);
	if (!Node)
	{
		return false;
	}

	Context.NarrativeId = DialogueAsset->DialogueId;
	Context.NodeId = Node->NodeId;
	bCompleted = Node->NodeType == ESigilDialogueNodeType::End;
	bActive = !bCompleted;
	return true;
}
