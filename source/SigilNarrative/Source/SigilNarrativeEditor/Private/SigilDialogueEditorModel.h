// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilDialogueAsset.h"

class FSigilDialogueEditorModel
{
public:
	explicit FSigilDialogueEditorModel(USigilDialogueAsset* InAsset);

	TArray<FName> GetFilteredNodeIds(const FString& Filter) const;
	int32 FindNodeIndex(FName NodeId) const;
	FName AddNode(ESigilDialogueNodeType NodeType);
	FName DuplicateNode(FName SourceNodeId);
	bool CanDeleteNode(FName NodeId, FText& OutReason) const;
	bool DeleteNode(FName NodeId, FText& OutReason);
	bool SetEntryNode(FName NodeId);
	bool ReconcileNodeEdit(
		int32 NodeIndex,
		FName PreviousNodeId,
		ESigilDialogueNodeType PreviousNodeType,
		FText& OutError);
	FSimpleMulticastDelegate& OnModelChanged();

	USigilDialogueAsset* GetAsset() const;

private:
	FName MakeUniqueNodeId(const FString& BaseName, bool bUsePaddedNumber) const;
	void FinishStructuralChange();

	TWeakObjectPtr<USigilDialogueAsset> Asset;
	FSimpleMulticastDelegate ModelChanged;
};
