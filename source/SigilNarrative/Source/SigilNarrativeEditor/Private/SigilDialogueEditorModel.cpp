// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilDialogueEditorModel.h"

#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

FSigilDialogueEditorModel::FSigilDialogueEditorModel(USigilDialogueAsset* InAsset)
	: Asset(InAsset)
{
}

TArray<FName> FSigilDialogueEditorModel::GetFilteredNodeIds(const FString& Filter) const
{
	TArray<FName> Result;
	const USigilDialogueAsset* DialogueAsset = Asset.Get();
	if (!DialogueAsset)
	{
		return Result;
	}

	for (const FSigilDialogueNode& Node : DialogueAsset->Nodes)
	{
		if (Filter.IsEmpty() || Node.NodeId.ToString().Contains(Filter, ESearchCase::IgnoreCase))
		{
			Result.Add(Node.NodeId);
		}
	}
	return Result;
}

int32 FSigilDialogueEditorModel::FindNodeIndex(const FName NodeId) const
{
	const USigilDialogueAsset* DialogueAsset = Asset.Get();
	if (!DialogueAsset)
	{
		return INDEX_NONE;
	}

	return DialogueAsset->Nodes.IndexOfByPredicate([NodeId](const FSigilDialogueNode& Node)
	{
		return Node.NodeId == NodeId;
	});
}

FName FSigilDialogueEditorModel::AddNode(const ESigilDialogueNodeType NodeType)
{
	USigilDialogueAsset* DialogueAsset = Asset.Get();
	if (!DialogueAsset)
	{
		return NAME_None;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddDialogueNode", "Add Dialogue Node"));
	DialogueAsset->Modify();
	FSigilDialogueNode& AddedNode = DialogueAsset->Nodes.AddDefaulted_GetRef();
	AddedNode.NodeId = MakeUniqueNodeId(TEXT("Node"), true);
	AddedNode.NodeType = NodeType;
	const FName AddedNodeId = AddedNode.NodeId;
	FinishStructuralChange();
	return AddedNodeId;
}

FName FSigilDialogueEditorModel::DuplicateNode(const FName SourceNodeId)
{
	USigilDialogueAsset* DialogueAsset = Asset.Get();
	const int32 SourceIndex = FindNodeIndex(SourceNodeId);
	if (!DialogueAsset || SourceIndex == INDEX_NONE)
	{
		return NAME_None;
	}

	const FScopedTransaction Transaction(LOCTEXT("DuplicateDialogueNode", "Duplicate Dialogue Node"));
	DialogueAsset->Modify();
	FSigilDialogueNode DuplicatedNode = DialogueAsset->Nodes[SourceIndex];
	DuplicatedNode.NodeId = MakeUniqueNodeId(SourceNodeId.ToString() + TEXT("_Copy"), false);
	const FName DuplicatedNodeId = DuplicatedNode.NodeId;
	DialogueAsset->Nodes.Add(MoveTemp(DuplicatedNode));
	FinishStructuralChange();
	return DuplicatedNodeId;
}

bool FSigilDialogueEditorModel::CanDeleteNode(const FName NodeId, FText& OutReason) const
{
	OutReason = FText::GetEmpty();
	const USigilDialogueAsset* DialogueAsset = Asset.Get();
	if (!DialogueAsset || FindNodeIndex(NodeId) == INDEX_NONE)
	{
		OutReason = LOCTEXT("DeleteMissingNode", "The selected node no longer exists.");
		return false;
	}

	if (DialogueAsset->EntryNodeId == NodeId)
	{
		OutReason = FText::Format(
			LOCTEXT("DeleteEntryNode", "Node {0} is the dialogue entry node."),
			FText::FromName(NodeId));
		return false;
	}

	for (const FSigilDialogueNode& Node : DialogueAsset->Nodes)
	{
		if (Node.NextNodeId == NodeId)
		{
			OutReason = FText::Format(
				LOCTEXT("DeleteReferencedByNode", "Node {0} is referenced by node {1}."),
				FText::FromName(NodeId),
				FText::FromName(Node.NodeId));
			return false;
		}

		for (const FSigilDialogueOption& Option : Node.Options)
		{
			if (Option.TargetNodeId == NodeId)
			{
				OutReason = FText::Format(
					LOCTEXT("DeleteReferencedByOption", "Node {0} is referenced by option {1} on node {2}."),
					FText::FromName(NodeId),
					FText::FromName(Option.OptionId),
					FText::FromName(Node.NodeId));
				return false;
			}
		}
	}

	return true;
}

bool FSigilDialogueEditorModel::DeleteNode(const FName NodeId, FText& OutReason)
{
	if (!CanDeleteNode(NodeId, OutReason))
	{
		return false;
	}

	USigilDialogueAsset* DialogueAsset = Asset.Get();
	const int32 NodeIndex = FindNodeIndex(NodeId);
	if (!DialogueAsset || NodeIndex == INDEX_NONE)
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteDialogueNode", "Delete Dialogue Node"));
	DialogueAsset->Modify();
	DialogueAsset->Nodes.RemoveAt(NodeIndex);
	FinishStructuralChange();
	return true;
}

bool FSigilDialogueEditorModel::SetEntryNode(const FName NodeId)
{
	USigilDialogueAsset* DialogueAsset = Asset.Get();
	if (!DialogueAsset || FindNodeIndex(NodeId) == INDEX_NONE)
	{
		return false;
	}

	if (DialogueAsset->EntryNodeId == NodeId)
	{
		return true;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetDialogueEntryNode", "Set Dialogue Entry Node"));
	DialogueAsset->Modify();
	DialogueAsset->EntryNodeId = NodeId;
	FinishStructuralChange();
	return true;
}

bool FSigilDialogueEditorModel::ReconcileNodeEdit(
	const int32 NodeIndex,
	const FName PreviousNodeId,
	const ESigilDialogueNodeType PreviousNodeType,
	FText& OutError)
{
	OutError = FText::GetEmpty();
	USigilDialogueAsset* DialogueAsset = Asset.Get();
	if (!DialogueAsset || !DialogueAsset->Nodes.IsValidIndex(NodeIndex))
	{
		OutError = LOCTEXT("EditedNodeMissing", "The edited node no longer exists.");
		return false;
	}

	FSigilDialogueNode& EditedNode = DialogueAsset->Nodes[NodeIndex];
	const FName EditedNodeId = EditedNode.NodeId;
	const bool bDuplicateNodeId = DialogueAsset->Nodes.ContainsByPredicate(
		[NodeIndex, EditedNodeId, DialogueAsset](const FSigilDialogueNode& Node)
		{
			return &Node != &DialogueAsset->Nodes[NodeIndex] && Node.NodeId == EditedNodeId;
		});
	if (EditedNodeId.IsNone() || bDuplicateNodeId)
	{
		EditedNode.NodeId = PreviousNodeId;
		OutError = EditedNodeId.IsNone()
			? LOCTEXT("EmptyNodeId", "NodeId must not be empty.")
			: FText::Format(
				LOCTEXT("DuplicateNodeId", "NodeId {0} is already in use."),
				FText::FromName(EditedNodeId));
		FinishStructuralChange();
		return false;
	}

	if (PreviousNodeId != EditedNodeId)
	{
		if (DialogueAsset->EntryNodeId == PreviousNodeId)
		{
			DialogueAsset->EntryNodeId = EditedNodeId;
		}

		for (FSigilDialogueNode& Node : DialogueAsset->Nodes)
		{
			if (Node.NextNodeId == PreviousNodeId)
			{
				Node.NextNodeId = EditedNodeId;
			}
			for (FSigilDialogueOption& Option : Node.Options)
			{
				if (Option.TargetNodeId == PreviousNodeId)
				{
					Option.TargetNodeId = EditedNodeId;
				}
			}
		}
	}

	if (PreviousNodeType != EditedNode.NodeType)
	{
		switch (EditedNode.NodeType)
		{
		case ESigilDialogueNodeType::Choice:
			EditedNode.NextNodeId = NAME_None;
			break;
		case ESigilDialogueNodeType::End:
			EditedNode.NextNodeId = NAME_None;
			EditedNode.Options.Reset();
			break;
		case ESigilDialogueNodeType::Line:
		default:
			EditedNode.Options.Reset();
			break;
		}
	}

	FinishStructuralChange();
	return true;
}

FSimpleMulticastDelegate& FSigilDialogueEditorModel::OnModelChanged()
{
	return ModelChanged;
}

USigilDialogueAsset* FSigilDialogueEditorModel::GetAsset() const
{
	return Asset.Get();
}

FName FSigilDialogueEditorModel::MakeUniqueNodeId(const FString& BaseName, const bool bUsePaddedNumber) const
{
	if (bUsePaddedNumber)
	{
		for (int32 Number = 1; Number < MAX_int32; ++Number)
		{
			const FName Candidate(*FString::Printf(TEXT("%s_%03d"), *BaseName, Number));
			if (FindNodeIndex(Candidate) == INDEX_NONE)
			{
				return Candidate;
			}
		}
		return NAME_None;
	}

	const FName BaseCandidate(*BaseName);
	if (FindNodeIndex(BaseCandidate) == INDEX_NONE)
	{
		return BaseCandidate;
	}

	for (int32 Number = 2; Number < MAX_int32; ++Number)
	{
		const FName Candidate(*FString::Printf(TEXT("%s_%d"), *BaseName, Number));
		if (FindNodeIndex(Candidate) == INDEX_NONE)
		{
			return Candidate;
		}
	}
	return NAME_None;
}

void FSigilDialogueEditorModel::FinishStructuralChange()
{
	if (USigilDialogueAsset* DialogueAsset = Asset.Get())
	{
		DialogueAsset->MarkPackageDirty();
	}
	ModelChanged.Broadcast();
}

#undef LOCTEXT_NAMESPACE
