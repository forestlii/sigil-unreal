// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Misc/NotifyHook.h"
#include "SigilStoryAsset.h"
#include "SigilStoryPreviewModel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class FProperty;
struct FPropertyAndParent;
class FSigilStoryEditorModel;
class FStoryBeatStructOnScope;
class IDetailsView;
class IStructureDetailsView;
class SVerticalBox;
class UClass;
class UObject;
enum class ESigilStoryBeatObjectList : uint8;

class SSigilStoryEditor final : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SSigilStoryEditor)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, USigilStoryAsset* InAsset);
	virtual ~SSigilStoryEditor() override;

	void Refresh();
	virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
	virtual void NotifyPostChange(
		const FPropertyChangedEvent& PropertyChangedEvent,
		FProperty* PropertyThatChanged) override;

private:
	void RefreshBeatList();
	void RefreshBeatObjectEditors();
	void ShowBeatDetails(FName BeatId);
	void ClearBeatDetails();
	void RefreshValidation();
	void RefreshPreviewPanel();
	void HandleModelChanged();
	bool IsBeatPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;
	FText GetStoryIdText() const;
	void HandleStoryIdCommitted(const FText& NewText, ETextCommit::Type CommitType);
	TSharedRef<SWidget> BuildBeatObjectSection(
		const FText& Heading,
		ESigilStoryBeatObjectList List,
		const UClass* BaseClass,
		TSharedPtr<SVerticalBox>& OutListBox);
	void HandleAddBeatObjectClass(const UClass* ObjectClass, ESigilStoryBeatObjectList List);
	FReply HandleSelectBeatObject(UObject* Object);
	FReply HandleRemoveBeatObject(ESigilStoryBeatObjectList List, int32 ObjectIndex);
	void HandleBeatObjectPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

	TSharedRef<ITableRow> GenerateBeatRow(
		TSharedPtr<FName> Item,
		const TSharedRef<STableViewBase>& OwnerTable) const;
	void HandleBeatSelectionChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo);
	void HandleFilterChanged(const FText& NewText);
	FReply HandleAddBeat();
	FReply HandleDuplicateBeat();
	FReply HandleDeleteBeat();
	FReply HandleStartPreview();
	FReply HandleEnterBeat(FName BeatId);
	FReply HandleCompleteBeat();
	void HandleConditionCheckChanged(
		ECheckBoxState NewState,
		FSigilStoryPreviewConditionKey Key);

	TSharedPtr<FSigilStoryEditorModel> Model;
	FDelegateHandle ModelChangedHandle;
	TArray<TSharedPtr<FName>> BeatListItems;
	TSharedPtr<SListView<TSharedPtr<FName>>> BeatListView;
	TSharedPtr<IStructureDetailsView> StructureDetailsView;
	TSharedPtr<IDetailsView> BeatObjectDetailsView;
	TSharedPtr<FStoryBeatStructOnScope> SelectedBeatScope;
	TSharedPtr<SVerticalBox> EnterConditionsBox;
	TSharedPtr<SVerticalBox> EnterEventsBox;
	TSharedPtr<SVerticalBox> CompleteEventsBox;
	TSharedPtr<SVerticalBox> PreviewBox;
	TWeakObjectPtr<UObject> SelectedBeatObject;
	FName SelectedBeatId;
	int32 SelectedBeatIndex = INDEX_NONE;
	FString BeatFilter;
	FName PreChangeBeatId;
	int32 PreChangeBeatIndex = INDEX_NONE;
	FText ValidationText;
	FLinearColor ValidationColor = FLinearColor::White;
	FText ActionMessage;
	FSigilStoryPreviewModel Preview;
	FDelegateHandle BeatObjectPropertyChangedHandle;
	bool bSuppressModelRefresh = false;
};
