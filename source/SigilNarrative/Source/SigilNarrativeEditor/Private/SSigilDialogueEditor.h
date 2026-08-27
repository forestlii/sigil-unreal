// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Misc/NotifyHook.h"
#include "SigilDialogueAsset.h"
#include "SigilDialoguePreviewModel.h"
#include "UObject/StructOnScope.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class FSigilDialogueEditorModel;
class IPropertyHandle;
class IStructureDetailsView;
class SVerticalBox;

class FDialogueNodeStructOnScope final : public FStructOnScope
{
public:
	FDialogueNodeStructOnScope(USigilDialogueAsset* InAsset, int32 InNodeIndex);

	virtual uint8* GetStructMemory() override;
	virtual const uint8* GetStructMemory() const override;
	virtual const UScriptStruct* GetStruct() const override;
	virtual UPackage* GetPackage() const override;
	virtual void SetPackage(UPackage* InPackage) override;
	virtual bool IsValid() const override;

private:
	TWeakObjectPtr<USigilDialogueAsset> Asset;
	int32 NodeIndex = INDEX_NONE;
};

class SSigilDialogueEditor final : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SSigilDialogueEditor)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, USigilDialogueAsset* InAsset);
	virtual ~SSigilDialogueEditor() override;

	void Refresh();

	virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
	virtual void NotifyPostChange(
		const FPropertyChangedEvent& PropertyChangedEvent,
		FProperty* PropertyThatChanged) override;

private:
	void RefreshNodeList();
	void ShowNodeDetails(FName NodeId);
	void ClearNodeDetails();
	void HandleModelChanged();
	void RefreshPreviewPanel();
	void OnSearchTextChanged(const FText& SearchText);
	void OnNodeSelectionChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo);
	TSharedRef<ITableRow> GenerateNodeRow(
		TSharedPtr<FName> Item,
		const TSharedRef<STableViewBase>& OwnerTable) const;

	FReply AddNode(ESigilDialogueNodeType NodeType);
	FReply DuplicateSelectedNode();
	FReply DeleteSelectedNode();
	FReply SetSelectedNodeAsEntry();
	FReply StartPreview();
	FReply AdvancePreview();
	FReply ChoosePreviewOption(FName OptionId);
	FReply SetPreviewConditionResult(
		FSigilDialoguePreviewConditionKey Key,
		ESigilDialoguePreviewConditionResult Result);

	bool HasSelectedNode() const;
	bool ShouldHideProperty(const TSharedRef<IPropertyHandle>& PropertyHandle) const;
	FText GetValidationText() const;
	FText GetErrorText() const;
	EVisibility GetErrorVisibility() const;
	FText GetNodeRowText(FName NodeId) const;

	TSharedPtr<FSigilDialogueEditorModel> Model;
	TSharedPtr<IStructureDetailsView> StructureDetailsView;
	TSharedPtr<FDialogueNodeStructOnScope> SelectedNodeStruct;
	TSharedPtr<SListView<TSharedPtr<FName>>> NodeListView;
	TSharedPtr<SVerticalBox> PreviewContent;
	TArray<TSharedPtr<FName>> NodeItems;
	FDelegateHandle ModelChangedHandle;

	FString FilterText;
	FName SelectedNodeId;
	int32 SelectedNodeIndex = INDEX_NONE;
	FName PreviousNodeId;
	ESigilDialogueNodeType PreviousNodeType = ESigilDialogueNodeType::Line;
	FText ErrorText;
	FText PreviewMessage;
	FSigilDialoguePreviewModel Preview;
	bool bSuppressModelRefresh = false;
};
