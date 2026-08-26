// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SSigilQuestEditor.h"

#include "IStructureDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "SigilQuestEditorModel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

namespace
{
	FText QuestStatusText(const ESigilQuestStatus Status)
	{
		switch (Status)
		{
		case ESigilQuestStatus::Active:
			return LOCTEXT("QuestPreviewActive", "Active");
		case ESigilQuestStatus::Succeeded:
			return LOCTEXT("QuestPreviewSucceeded", "Succeeded");
		case ESigilQuestStatus::Failed:
			return LOCTEXT("QuestPreviewFailed", "Failed");
		case ESigilQuestStatus::NotStarted:
		default:
			return LOCTEXT("QuestPreviewNotStarted", "Not Started");
		}
	}

	FText StateTypeText(const ESigilQuestStateType Type)
	{
		switch (Type)
		{
		case ESigilQuestStateType::Success:
			return LOCTEXT("QuestStateSuccess", "Success");
		case ESigilQuestStateType::Failure:
			return LOCTEXT("QuestStateFailure", "Failure");
		case ESigilQuestStateType::Regular:
		default:
			return LOCTEXT("QuestStateRegular", "Regular");
		}
	}
}

void SSigilQuestEditor::Construct(const FArguments& InArgs, USigilQuestAsset* InAsset)
{
	check(InAsset);
	Model = MakeShared<FSigilQuestEditorModel>(InAsset);
	ModelChangedHandle = Model->OnModelChanged().AddSP(this, &SSigilQuestEditor::HandleModelChanged);

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowObjectLabel = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;

	FStructureDetailsViewArgs StructureViewArgs;
	StructureViewArgs.bShowObjects = true;
	StructureViewArgs.bShowAssets = true;
	StructureViewArgs.bShowClasses = true;
	StructureViewArgs.bShowInterfaces = false;

	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	StructureDetailsView = PropertyEditorModule.CreateStructureDetailView(
		DetailsViewArgs,
		StructureViewArgs,
		nullptr,
		LOCTEXT("QuestStateDetails", "Quest State"));

	ChildSlot
	[
		SNew(SSplitter)
		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("QuestStatesHeading", "Quest States"))
					.Font(FAppStyle::GetFontStyle(TEXT("HeadingExtraSmall")))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f)
				[
					SNew(SSearchBox)
					.OnTextChanged(this, &SSigilQuestEditor::HandleFilterChanged)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FMargin(2.0f))
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddRegularState", "+ Regular"))
						.OnClicked_Lambda([this]() { return HandleAddState(ESigilQuestStateType::Regular); })
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddSuccessState", "+ Success"))
						.OnClicked_Lambda([this]() { return HandleAddState(ESigilQuestStateType::Success); })
					]
					+ SUniformGridPanel::Slot(0, 1)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddFailureState", "+ Failure"))
						.OnClicked_Lambda([this]() { return HandleAddState(ESigilQuestStateType::Failure); })
					]
					+ SUniformGridPanel::Slot(1, 1)
					[
						SNew(SButton)
						.Text(LOCTEXT("DuplicateQuestState", "Duplicate"))
						.IsEnabled_Lambda([this]() { return !SelectedStateId.IsNone(); })
						.OnClicked(this, &SSigilQuestEditor::HandleDuplicateState)
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(0.0f, 6.0f)
				[
					SAssignNew(StateListView, SListView<TSharedPtr<FName>>)
					.ListItemsSource(&StateListItems)
					.OnGenerateRow(this, &SSigilQuestEditor::GenerateStateRow)
					.OnSelectionChanged(this, &SSigilQuestEditor::HandleStateSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FMargin(2.0f))
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("SetInitialQuestState", "Set Initial"))
						.IsEnabled_Lambda([this]() { return !SelectedStateId.IsNone(); })
						.OnClicked(this, &SSigilQuestEditor::HandleSetInitialState)
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("DeleteQuestState", "Delete"))
						.IsEnabled_Lambda([this]() { return !SelectedStateId.IsNone(); })
						.OnClicked(this, &SSigilQuestEditor::HandleDeleteState)
					]
				]
			]
		]
		+ SSplitter::Slot()
		.Value(0.75f)
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(0.58f)
				[
					StructureDetailsView->GetWidget().ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return ValidationText; })
					.ColorAndOpacity_Lambda([this]() { return ValidationColor; })
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 3.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return ActionMessage; })
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot()
				.FillHeight(0.42f)
				[
					SNew(SBorder)
					.Padding(8.0f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(PreviewBox, SVerticalBox)
						]
					]
				]
			]
		]
	];

	Refresh();
}

SSigilQuestEditor::~SSigilQuestEditor()
{
	if (Model.IsValid())
	{
		Model->OnModelChanged().Remove(ModelChangedHandle);
	}
}

void SSigilQuestEditor::Refresh()
{
	Preview.Invalidate();
	RefreshStateList();
	if (!SelectedStateId.IsNone() && Model->FindStateIndex(SelectedStateId) != INDEX_NONE)
	{
		ShowStateDetails(SelectedStateId);
	}
	else
	{
		ClearStateDetails();
	}
	RefreshValidation();
	RefreshPreviewPanel();
}

void SSigilQuestEditor::NotifyPreChange(FProperty* PropertyAboutToChange)
{
	(void)PropertyAboutToChange;
	USigilQuestAsset* QuestAsset = Model->GetAsset();
	const int32 StateIndex = Model->FindStateIndex(SelectedStateId);
	if (!QuestAsset || !QuestAsset->States.IsValidIndex(StateIndex))
	{
		return;
	}
	QuestAsset->Modify();
	PreChangeStateIndex = StateIndex;
	PreChangeStateId = QuestAsset->States[StateIndex].StateId;
	PreChangeStateType = QuestAsset->States[StateIndex].StateType;
}

void SSigilQuestEditor::NotifyPostChange(
	const FPropertyChangedEvent& PropertyChangedEvent,
	FProperty* PropertyThatChanged)
{
	(void)PropertyChangedEvent;
	(void)PropertyThatChanged;
	const int32 StateIndex = PreChangeStateIndex;
	if (StateIndex == INDEX_NONE)
	{
		Refresh();
		return;
	}

	FText Error;
	Model->ReconcileStateEdit(StateIndex, PreChangeStateId, PreChangeStateType, Error);
	USigilQuestAsset* QuestAsset = Model->GetAsset();
	if (QuestAsset && QuestAsset->States.IsValidIndex(StateIndex))
	{
		SelectedStateId = QuestAsset->States[StateIndex].StateId;
	}
	ActionMessage = Error;
	Refresh();
}

void SSigilQuestEditor::RefreshStateList()
{
	StateListItems.Reset();
	for (const FName StateId : Model->GetFilteredStateIds(StateFilter))
	{
		StateListItems.Add(MakeShared<FName>(StateId));
	}
	if (StateListView.IsValid())
	{
		StateListView->RequestListRefresh();
		for (const TSharedPtr<FName>& Item : StateListItems)
		{
			if (Item.IsValid() && *Item == SelectedStateId)
			{
				StateListView->SetSelection(Item);
				break;
			}
		}
	}
}

void SSigilQuestEditor::ShowStateDetails(const FName StateId)
{
	const int32 StateIndex = Model->FindStateIndex(StateId);
	if (StateIndex == INDEX_NONE)
	{
		ClearStateDetails();
		return;
	}
	SelectedStateId = StateId;
	SelectedStateIndex = StateIndex;
	SelectedStateScope = MakeShared<FQuestStateStructOnScope>(Model->GetAsset(), StateIndex);
	StructureDetailsView->SetStructureData(SelectedStateScope);
}

void SSigilQuestEditor::ClearStateDetails()
{
	SelectedStateId = NAME_None;
	SelectedStateIndex = INDEX_NONE;
	SelectedStateScope.Reset();
	StructureDetailsView->SetStructureData(nullptr);
}

void SSigilQuestEditor::RefreshValidation()
{
	FText Error;
	const USigilQuestAsset* QuestAsset = Model->GetAsset();
	if (QuestAsset && QuestAsset->ValidateDefinition(Error))
	{
		ValidationText = LOCTEXT("QuestDefinitionValid", "Definition: Valid");
		ValidationColor = FLinearColor(0.2f, 0.8f, 0.3f);
	}
	else
	{
		ValidationText = FText::Format(
			LOCTEXT("QuestDefinitionInvalid", "Definition: Invalid - {0}"),
			Error);
		ValidationColor = FLinearColor(0.9f, 0.2f, 0.2f);
	}
}

void SSigilQuestEditor::RefreshPreviewPanel()
{
	if (!PreviewBox.IsValid())
	{
		return;
	}
	PreviewBox->ClearChildren();
	PreviewBox->AddSlot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("QuestSafePreviewHeading", "Safe Preview"))
			.Font(FAppStyle::GetFontStyle(TEXT("HeadingExtraSmall")))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(10.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("StartQuestPreview", "Start / Restart"))
			.OnClicked(this, &SSigilQuestEditor::HandleStartPreview)
		]
	];
	PreviewBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT(
			"QuestPreviewSafety",
			"Tasks and conditions are set manually. Events are recorded but never executed."))
		.AutoWrapText(true)
	];

	if (Preview.GetCurrentStateId().IsNone())
	{
		return;
	}

	PreviewBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::Format(
			LOCTEXT("QuestPreviewCurrent", "Current: {0} ({1})"),
			FText::FromName(Preview.GetCurrentStateId()),
			QuestStatusText(Preview.GetStatus())))
	];

	const USigilQuestAsset* QuestAsset = Model->GetAsset();
	const FSigilQuestState* State = QuestAsset
		? QuestAsset->FindState(Preview.GetCurrentStateId())
		: nullptr;
	if (State && Preview.IsActive())
	{
		for (const FSigilQuestTaskDefinition& Task : State->Tasks)
		{
			const FName TaskId = Task.TaskId;
			const int32 RequiredCount = Task.RequiredCount;
			PreviewBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this, TaskId, RequiredCount]()
				{
					return Preview.GetTaskProgress(TaskId) >= RequiredCount
						? ECheckBoxState::Checked
						: ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged(this, &SSigilQuestEditor::HandleTaskCheckChanged, TaskId, RequiredCount)
				[
					SNew(STextBlock)
					.Text(FText::Format(
						LOCTEXT("QuestPreviewTask", "Task {0}: {1}"),
						FText::FromName(TaskId),
						Task.ObjectiveText))
				]
			];
		}

		for (const FSigilQuestTransition& Transition : State->Transitions)
		{
			const FName TransitionId = Transition.TransitionId;
			PreviewBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 5.0f, 0.0f, 1.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::Format(
						LOCTEXT("QuestPreviewTransition", "{0} -> {1} ({2} events)"),
						FText::FromName(TransitionId),
						FText::FromName(Transition.TargetStateId),
						FText::AsNumber(Transition.Events.Num())))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("TakeQuestTransition", "Take"))
					.IsEnabled_Lambda([this, TransitionId]()
					{
						FText Reason;
						return Preview.CanTakeTransition(TransitionId, Reason);
					})
					.ToolTipText_Lambda([this, TransitionId]()
					{
						FText Reason;
						return Preview.CanTakeTransition(TransitionId, Reason)
							? LOCTEXT("QuestTransitionReady", "Transition is ready.")
							: Reason;
					})
					.OnClicked(this, &SSigilQuestEditor::HandleTakeTransition, TransitionId)
				]
			];

			for (int32 ConditionIndex = 0; ConditionIndex < Transition.Conditions.Num(); ++ConditionIndex)
			{
				const FSigilQuestPreviewConditionKey Key{State->StateId, TransitionId, ConditionIndex};
				PreviewBox->AddSlot()
				.AutoHeight()
				.Padding(16.0f, 1.0f)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this, Key]()
					{
						switch (Preview.GetConditionResult(Key))
						{
						case ESigilQuestPreviewConditionResult::True:
							return ECheckBoxState::Checked;
						case ESigilQuestPreviewConditionResult::False:
							return ECheckBoxState::Unchecked;
						case ESigilQuestPreviewConditionResult::Unspecified:
						default:
							return ECheckBoxState::Undetermined;
						}
					})
					.OnCheckStateChanged(this, &SSigilQuestEditor::HandleConditionCheckChanged, Key)
					[
						SNew(STextBlock)
						.Text(FText::Format(
							LOCTEXT("QuestPreviewCondition", "Condition {0} (manual)"),
							FText::AsNumber(ConditionIndex + 1)))
					]
				];
			}
		}
	}

	PreviewBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 5.0f)
	[
		SNew(STextBlock)
		.Text(FText::Format(
			LOCTEXT("QuestPreviewEventLog", "Recorded events: {0} (executed: 0)"),
			FText::AsNumber(Preview.GetEventLog().Num())))
	];
}

void SSigilQuestEditor::HandleModelChanged()
{
	ActionMessage = LOCTEXT("QuestPreviewInvalidated", "Asset changed. Restart the safe preview.");
	Refresh();
}

TSharedRef<ITableRow> SSigilQuestEditor::GenerateStateRow(
	TSharedPtr<FName> Item,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	const USigilQuestAsset* QuestAsset = Model->GetAsset();
	const FSigilQuestState* State = Item.IsValid() && QuestAsset ? QuestAsset->FindState(*Item) : nullptr;
	const bool bInitial = State && QuestAsset->InitialStateId == State->StateId;
	const FText Label = State
		? FText::Format(
			LOCTEXT("QuestStateRow", "{0}{1}  [{2}]"),
			bInitial ? LOCTEXT("QuestInitialMarker", "* ") : FText::GetEmpty(),
			FText::FromName(State->StateId),
			StateTypeText(State->StateType))
		: LOCTEXT("QuestStateMissing", "Missing state");
	return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
	[
		SNew(STextBlock).Text(Label)
	];
}

void SSigilQuestEditor::HandleStateSelectionChanged(
	TSharedPtr<FName> Item,
	const ESelectInfo::Type SelectInfo)
{
	(void)SelectInfo;
	if (Item.IsValid())
	{
		ShowStateDetails(*Item);
	}
	else
	{
		ClearStateDetails();
	}
}

void SSigilQuestEditor::HandleFilterChanged(const FText& NewText)
{
	StateFilter = NewText.ToString();
	RefreshStateList();
}

FReply SSigilQuestEditor::HandleAddState(const ESigilQuestStateType StateType)
{
	SelectedStateId = Model->AddState(StateType);
	ActionMessage = LOCTEXT("QuestStateAdded", "State added.");
	Refresh();
	return FReply::Handled();
}

FReply SSigilQuestEditor::HandleDuplicateState()
{
	const FName NewStateId = Model->DuplicateState(SelectedStateId);
	if (!NewStateId.IsNone())
	{
		SelectedStateId = NewStateId;
		ActionMessage = LOCTEXT("QuestStateDuplicated", "State duplicated.");
	}
	Refresh();
	return FReply::Handled();
}

FReply SSigilQuestEditor::HandleDeleteState()
{
	FText Reason;
	if (Model->DeleteState(SelectedStateId, Reason))
	{
		SelectedStateId = NAME_None;
		ActionMessage = LOCTEXT("QuestStateDeleted", "State deleted.");
	}
	else
	{
		ActionMessage = Reason;
	}
	Refresh();
	return FReply::Handled();
}

FReply SSigilQuestEditor::HandleSetInitialState()
{
	if (Model->SetInitialState(SelectedStateId))
	{
		ActionMessage = LOCTEXT("QuestInitialStateSet", "Initial state updated.");
	}
	Refresh();
	return FReply::Handled();
}

FReply SSigilQuestEditor::HandleStartPreview()
{
	FText Error;
	if (Preview.Start(Model->GetAsset(), Error))
	{
		ActionMessage = LOCTEXT("QuestPreviewStarted", "Safe preview started.");
	}
	else
	{
		ActionMessage = Error;
	}
	RefreshPreviewPanel();
	return FReply::Handled();
}

FReply SSigilQuestEditor::HandleTakeTransition(const FName TransitionId)
{
	FText Error;
	if (Preview.TakeTransition(TransitionId, Error))
	{
		ActionMessage = LOCTEXT("QuestPreviewTransitionTaken", "Preview transition taken.");
	}
	else
	{
		ActionMessage = Error;
	}
	RefreshPreviewPanel();
	return FReply::Handled();
}

void SSigilQuestEditor::HandleTaskCheckChanged(
	const ECheckBoxState NewState,
	const FName TaskId,
	const int32 RequiredCount)
{
	Preview.SetTaskProgress(TaskId, NewState == ECheckBoxState::Checked ? RequiredCount : 0);
	RefreshPreviewPanel();
}

void SSigilQuestEditor::HandleConditionCheckChanged(
	const ECheckBoxState NewState,
	const FSigilQuestPreviewConditionKey Key)
{
	ESigilQuestPreviewConditionResult Result = ESigilQuestPreviewConditionResult::Unspecified;
	if (NewState == ECheckBoxState::Checked)
	{
		Result = ESigilQuestPreviewConditionResult::True;
	}
	else if (NewState == ECheckBoxState::Unchecked)
	{
		Result = ESigilQuestPreviewConditionResult::False;
	}
	Preview.SetConditionResult(Key, Result);
	RefreshPreviewPanel();
}

#undef LOCTEXT_NAMESPACE
