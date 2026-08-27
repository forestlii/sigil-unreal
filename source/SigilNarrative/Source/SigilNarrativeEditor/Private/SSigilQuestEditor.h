// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Misc/NotifyHook.h"
#include "SigilQuestAsset.h"
#include "SigilQuestPreviewModel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class FProperty;
class FQuestStateStructOnScope;
class FSigilQuestEditorModel;
class IStructureDetailsView;
class SVerticalBox;

class SSigilQuestEditor final : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SSigilQuestEditor)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, USigilQuestAsset* InAsset);
	virtual ~SSigilQuestEditor() override;

	void Refresh();
	virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
	virtual void NotifyPostChange(
		const FPropertyChangedEvent& PropertyChangedEvent,
		FProperty* PropertyThatChanged) override;

private:
	void RefreshStateList();
	void ShowStateDetails(FName StateId);
	void ClearStateDetails();
	void RefreshValidation();
	void RefreshPreviewPanel();
	void HandleModelChanged();

	TSharedRef<ITableRow> GenerateStateRow(
		TSharedPtr<FName> Item,
		const TSharedRef<STableViewBase>& OwnerTable) const;
	void HandleStateSelectionChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo);
	void HandleFilterChanged(const FText& NewText);
	FReply HandleAddState(ESigilQuestStateType StateType);
	FReply HandleDuplicateState();
	FReply HandleDeleteState();
	FReply HandleSetInitialState();
	FReply HandleStartPreview();
	FReply HandleTakeTransition(FName TransitionId);
	void HandleTaskCheckChanged(ECheckBoxState NewState, FName TaskId, int32 RequiredCount);
	void HandleConditionCheckChanged(
		ECheckBoxState NewState,
		FSigilQuestPreviewConditionKey Key);

	TSharedPtr<FSigilQuestEditorModel> Model;
	FDelegateHandle ModelChangedHandle;
	TArray<TSharedPtr<FName>> StateListItems;
	TSharedPtr<SListView<TSharedPtr<FName>>> StateListView;
	TSharedPtr<IStructureDetailsView> StructureDetailsView;
	TSharedPtr<FQuestStateStructOnScope> SelectedStateScope;
	TSharedPtr<SVerticalBox> PreviewBox;
	FName SelectedStateId;
	int32 SelectedStateIndex = INDEX_NONE;
	FString StateFilter;
	FName PreChangeStateId;
	int32 PreChangeStateIndex = INDEX_NONE;
	ESigilQuestStateType PreChangeStateType = ESigilQuestStateType::Regular;
	FText ValidationText;
	FLinearColor ValidationColor = FLinearColor::White;
	FText ActionMessage;
	FSigilQuestPreviewModel Preview;
};
