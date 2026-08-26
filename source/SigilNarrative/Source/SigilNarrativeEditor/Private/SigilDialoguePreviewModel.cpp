// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilDialoguePreviewModel.h"

#include "SigilDialogueAsset.h"
#include "SigilNarrativeEvent.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

bool FSigilDialoguePreviewModel::Start(
	const USigilDialogueAsset* InAsset,
	FText& OutError)
{
	Invalidate();
	OutError = FText::GetEmpty();
	if (!InAsset)
	{
		OutError = LOCTEXT("PreviewMissingAsset", "Dialogue asset is unavailable.");
		return false;
	}

	if (!InAsset->ValidateDefinition(OutError))
	{
		return false;
	}

	Asset = const_cast<USigilDialogueAsset*>(InAsset);
	return EnterNode(InAsset->EntryNodeId, OutError);
}

void FSigilDialoguePreviewModel::Invalidate()
{
	Asset.Reset();
	CurrentNodeId = NAME_None;
	bActive = false;
	bComplete = false;
	ConditionResults.Reset();
	VisitHistory.Reset();
	EventLog.Reset();
}

bool FSigilDialoguePreviewModel::Advance(FText& OutError)
{
	OutError = FText::GetEmpty();
	const USigilDialogueAsset* DialogueAsset = Asset.Get();
	if (!bActive || !DialogueAsset)
	{
		OutError = LOCTEXT("PreviewNotActive", "Start or restart the preview first.");
		return false;
	}

	const FSigilDialogueNode* Node = DialogueAsset->FindNode(CurrentNodeId);
	if (!Node || Node->NodeType != ESigilDialogueNodeType::Line)
	{
		OutError = LOCTEXT("PreviewAdvanceNeedsLine", "Only a Line node can advance directly.");
		return false;
	}

	return EnterNode(Node->NextNodeId, OutError);
}

bool FSigilDialoguePreviewModel::Choose(const FName OptionId, FText& OutError)
{
	if (!CanChoose(OptionId, OutError))
	{
		return false;
	}

	const USigilDialogueAsset* DialogueAsset = Asset.Get();
	const FSigilDialogueNode* Node = DialogueAsset ? DialogueAsset->FindNode(CurrentNodeId) : nullptr;
	const FSigilDialogueOption* Option = Node
		? Node->Options.FindByPredicate([OptionId](const FSigilDialogueOption& Candidate)
		{
			return Candidate.OptionId == OptionId;
		})
		: nullptr;
	if (!Node || !Option)
	{
		OutError = LOCTEXT("PreviewOptionChanged", "The selected option is no longer available.");
		return false;
	}

	for (int32 EventIndex = 0; EventIndex < Option->Events.Num(); ++EventIndex)
	{
		const USigilNarrativeEvent* Event = Option->Events[EventIndex];
		EventLog.Add({Node->NodeId, Option->OptionId, EventIndex, Event ? Event->GetClass() : nullptr});
	}

	return EnterNode(Option->TargetNodeId, OutError);
}

void FSigilDialoguePreviewModel::SetConditionResult(
	const FSigilDialoguePreviewConditionKey& Key,
	const ESigilDialoguePreviewConditionResult Result)
{
	if (Result == ESigilDialoguePreviewConditionResult::Unspecified)
	{
		ConditionResults.Remove(Key);
		return;
	}

	ConditionResults.Add(Key, Result);
}

ESigilDialoguePreviewConditionResult FSigilDialoguePreviewModel::GetConditionResult(
	const FSigilDialoguePreviewConditionKey& Key) const
{
	const ESigilDialoguePreviewConditionResult* Result = ConditionResults.Find(Key);
	return Result ? *Result : ESigilDialoguePreviewConditionResult::Unspecified;
}

bool FSigilDialoguePreviewModel::CanChoose(
	const FName OptionId,
	FText& OutReason) const
{
	OutReason = FText::GetEmpty();
	const USigilDialogueAsset* DialogueAsset = Asset.Get();
	if (!bActive || !DialogueAsset)
	{
		OutReason = LOCTEXT("PreviewChooseNotActive", "Start or restart the preview first.");
		return false;
	}

	const FSigilDialogueNode* Node = DialogueAsset->FindNode(CurrentNodeId);
	if (!Node || Node->NodeType != ESigilDialogueNodeType::Choice)
	{
		OutReason = LOCTEXT("PreviewChooseNeedsChoice", "The current node is not a Choice node.");
		return false;
	}

	const FSigilDialogueOption* Option = Node->Options.FindByPredicate(
		[OptionId](const FSigilDialogueOption& Candidate)
		{
			return Candidate.OptionId == OptionId;
		});
	if (!Option)
	{
		OutReason = LOCTEXT("PreviewOptionMissing", "The selected option does not exist.");
		return false;
	}

	for (int32 ConditionIndex = 0; ConditionIndex < Option->Conditions.Num(); ++ConditionIndex)
	{
		const FSigilDialoguePreviewConditionKey Key{Node->NodeId, Option->OptionId, ConditionIndex};
		const ESigilDialoguePreviewConditionResult* Result = ConditionResults.Find(Key);
		if (!Result || *Result == ESigilDialoguePreviewConditionResult::Unspecified)
		{
			OutReason = FText::Format(
				LOCTEXT("PreviewConditionUnspecified", "Condition {0} needs a manual preview result."),
				FText::AsNumber(ConditionIndex + 1));
			return false;
		}
		if (*Result == ESigilDialoguePreviewConditionResult::False)
		{
			OutReason = FText::Format(
				LOCTEXT("PreviewConditionFalse", "Condition {0} is manually set to False."),
				FText::AsNumber(ConditionIndex + 1));
			return false;
		}
	}

	return true;
}

FName FSigilDialoguePreviewModel::GetCurrentNodeId() const
{
	return CurrentNodeId;
}

bool FSigilDialoguePreviewModel::IsActive() const
{
	return bActive;
}

bool FSigilDialoguePreviewModel::IsComplete() const
{
	return bComplete;
}

const TArray<FName>& FSigilDialoguePreviewModel::GetVisitHistory() const
{
	return VisitHistory;
}

const TArray<FSigilDialoguePreviewEventRecord>& FSigilDialoguePreviewModel::GetEventLog() const
{
	return EventLog;
}

bool FSigilDialoguePreviewModel::EnterNode(const FName NodeId, FText& OutError)
{
	const USigilDialogueAsset* DialogueAsset = Asset.Get();
	const FSigilDialogueNode* Node = DialogueAsset ? DialogueAsset->FindNode(NodeId) : nullptr;
	if (!Node)
	{
		OutError = LOCTEXT("PreviewTargetMissing", "The target node is no longer available.");
		return false;
	}

	CurrentNodeId = Node->NodeId;
	VisitHistory.Add(CurrentNodeId);
	bComplete = Node->NodeType == ESigilDialogueNodeType::End;
	bActive = !bComplete;
	OutError = FText::GetEmpty();
	return true;
}

#undef LOCTEXT_NAMESPACE
