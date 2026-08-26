// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SSigilStoryEditor.h"

#include "IDetailsView.h"
#include "IStructureDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"
#include "SigilStoryEditorModel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
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

void SSigilStoryEditor::Construct(const FArguments& InArgs, USigilStoryAsset* InAsset)
{
	check(InAsset);
	Model = MakeShared<FSigilStoryEditorModel>(InAsset);
	ModelChangedHandle = Model->OnModelChanged().AddSP(this, &SSigilStoryEditor::HandleModelChanged);

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
		LOCTEXT("StoryBeatDetails", "Story Beat"));
	StructureDetailsView->GetDetailsView()->SetIsPropertyVisibleDelegate(
		FIsPropertyVisible::CreateSP(this, &SSigilStoryEditor::IsBeatPropertyVisible));

	FDetailsViewArgs ObjectDetailsViewArgs;
	ObjectDetailsViewArgs.bAllowSearch = true;
	ObjectDetailsViewArgs.bHideSelectionTip = true;
	ObjectDetailsViewArgs.bLockable = false;
	ObjectDetailsViewArgs.bShowObjectLabel = true;
	ObjectDetailsViewArgs.bUpdatesFromSelection = false;
	ObjectDetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	BeatObjectDetailsView = PropertyEditorModule.CreateDetailView(ObjectDetailsViewArgs);
	BeatObjectPropertyChangedHandle = BeatObjectDetailsView->OnFinishedChangingProperties().AddSP(
		this,
		&SSigilStoryEditor::HandleBeatObjectPropertyChanged);

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
					.Text(LOCTEXT("StoryBeatsHeading", "Story Beats"))
					.Font(FAppStyle::GetFontStyle(TEXT("HeadingExtraSmall")))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f)
				[
					SNew(SSearchBox)
					.OnTextChanged(this, &SSigilStoryEditor::HandleFilterChanged)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FMargin(2.0f))
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddStoryBeat", "+ Beat"))
						.OnClicked(this, &SSigilStoryEditor::HandleAddBeat)
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("DuplicateStoryBeat", "Duplicate"))
						.IsEnabled_Lambda([this]() { return !SelectedBeatId.IsNone(); })
						.OnClicked(this, &SSigilStoryEditor::HandleDuplicateBeat)
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(0.0f, 6.0f)
				[
					SAssignNew(BeatListView, SListView<TSharedPtr<FName>>)
					.ListItemsSource(&BeatListItems)
					.OnGenerateRow(this, &SSigilStoryEditor::GenerateBeatRow)
					.OnSelectionChanged(this, &SSigilStoryEditor::HandleBeatSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("DeleteStoryBeat", "Delete"))
					.IsEnabled_Lambda([this]() { return !SelectedBeatId.IsNone(); })
					.OnClicked(this, &SSigilStoryEditor::HandleDeleteBeat)
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
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("StoryIdLabel", "Story ID"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SEditableTextBox)
						.Text(this, &SSigilStoryEditor::GetStoryIdText)
						.OnTextCommitted(this, &SSigilStoryEditor::HandleStoryIdCommitted)
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(0.28f)
				[
					StructureDetailsView->GetWidget().ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.FillHeight(0.34f)
				.Padding(0.0f, 6.0f)
				[
					SNew(SSplitter)
					.Orientation(Orient_Vertical)
					+ SSplitter::Slot()
					.Value(0.58f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								BuildBeatObjectSection(
									LOCTEXT("EnterConditionsHeading", "Enter Conditions"),
									ESigilStoryBeatObjectList::EnterConditions,
									USigilNarrativeCondition::StaticClass(),
									EnterConditionsBox)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 4.0f)
							[
								BuildBeatObjectSection(
									LOCTEXT("EnterEventsHeading", "Enter Events"),
									ESigilStoryBeatObjectList::EnterEvents,
									USigilNarrativeEvent::StaticClass(),
									EnterEventsBox)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								BuildBeatObjectSection(
									LOCTEXT("CompleteEventsHeading", "Complete Events"),
									ESigilStoryBeatObjectList::CompleteEvents,
									USigilNarrativeEvent::StaticClass(),
									CompleteEventsBox)
							]
						]
					]
					+ SSplitter::Slot()
					.Value(0.42f)
					[
						SNew(SBorder)
						.Padding(4.0f)
						[
							BeatObjectDetailsView.ToSharedRef()
						]
					]
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
				.FillHeight(0.38f)
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

SSigilStoryEditor::~SSigilStoryEditor()
{
	if (BeatObjectDetailsView.IsValid())
	{
		BeatObjectDetailsView->OnFinishedChangingProperties().Remove(BeatObjectPropertyChangedHandle);
	}
	if (Model.IsValid())
	{
		Model->OnModelChanged().Remove(ModelChangedHandle);
	}
}

void SSigilStoryEditor::Refresh()
{
	Preview.Invalidate();
	RefreshBeatList();
	if (!SelectedBeatId.IsNone() && Model->FindBeatIndex(SelectedBeatId) != INDEX_NONE)
	{
		ShowBeatDetails(SelectedBeatId);
	}
	else
	{
		ClearBeatDetails();
	}
	RefreshBeatObjectEditors();
	RefreshValidation();
	RefreshPreviewPanel();
}

bool SSigilStoryEditor::IsBeatPropertyVisible(const FPropertyAndParent& PropertyAndParent) const
{
	const FName PropertyName = PropertyAndParent.Property.GetFName();
	return PropertyName != GET_MEMBER_NAME_CHECKED(FSigilStoryBeatDefinition, EnterConditions)
		&& PropertyName != GET_MEMBER_NAME_CHECKED(FSigilStoryBeatDefinition, EnterEvents)
		&& PropertyName != GET_MEMBER_NAME_CHECKED(FSigilStoryBeatDefinition, CompleteEvents);
}

FText SSigilStoryEditor::GetStoryIdText() const
{
	const USigilStoryAsset* StoryAsset = Model.IsValid() ? Model->GetAsset() : nullptr;
	return StoryAsset && !StoryAsset->StoryId.IsNone()
		? FText::FromName(StoryAsset->StoryId)
		: FText::GetEmpty();
}

void SSigilStoryEditor::HandleStoryIdCommitted(
	const FText& NewText,
	const ETextCommit::Type CommitType)
{
	(void)CommitType;
	FString StoryIdText = NewText.ToString();
	StoryIdText.TrimStartAndEndInline();
	FText Error;
	bSuppressModelRefresh = true;
	const bool bChanged = Model->SetStoryId(FName(*StoryIdText), Error);
	bSuppressModelRefresh = false;
	ActionMessage = bChanged ? LOCTEXT("StoryIdChanged", "Story ID updated.") : Error;
	Refresh();
}

TSharedRef<SWidget> SSigilStoryEditor::BuildBeatObjectSection(
	const FText& Heading,
	const ESigilStoryBeatObjectList List,
	const UClass* BaseClass,
	TSharedPtr<SVerticalBox>& OutListBox)
{
	return SNew(SBorder)
	.Padding(4.0f)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(Heading)
			.Font(FAppStyle::GetFontStyle(TEXT("HeadingExtraSmall")))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 3.0f)
		[
			SNew(SClassPropertyEntryBox)
			.MetaClass(BaseClass)
			.AllowAbstract(false)
			.AllowNone(false)
			.ShowDisplayNames(true)
			.SelectedClass(nullptr)
			.OnSetClass(FOnSetClass::CreateSP(
				this,
				&SSigilStoryEditor::HandleAddBeatObjectClass,
				List))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(OutListBox, SVerticalBox)
		]
	];
}

void SSigilStoryEditor::RefreshBeatObjectEditors()
{
	if (!EnterConditionsBox.IsValid() || !EnterEventsBox.IsValid() || !CompleteEventsBox.IsValid())
	{
		return;
	}

	EnterConditionsBox->ClearChildren();
	EnterEventsBox->ClearChildren();
	CompleteEventsBox->ClearChildren();
	USigilStoryAsset* StoryAsset = Model->GetAsset();
	const int32 BeatIndex = Model->FindBeatIndex(SelectedBeatId);
	if (!StoryAsset || !StoryAsset->Beats.IsValidIndex(BeatIndex))
	{
		SelectedBeatObject.Reset();
		BeatObjectDetailsView->SetObject(nullptr);
		return;
	}

	auto AddObjectRow = [this](
		SVerticalBox& Box,
		UObject* Object,
		const ESigilStoryBeatObjectList List,
		const int32 ObjectIndex)
	{
		Box.AddSlot()
		.AutoHeight()
		.Padding(0.0f, 1.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.Text(Object ? Object->GetClass()->GetDisplayNameText() : LOCTEXT("MissingBeatObject", "Missing object"))
				.IsEnabled(Object != nullptr)
				.OnClicked(this, &SSigilStoryEditor::HandleSelectBeatObject, Object)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RemoveBeatObject", "Remove"))
				.OnClicked(this, &SSigilStoryEditor::HandleRemoveBeatObject, List, ObjectIndex)
			]
		];
	};

	const FSigilStoryBeatDefinition& Beat = StoryAsset->Beats[BeatIndex];
	bool bSelectedObjectStillPresent = false;
	for (int32 Index = 0; Index < Beat.EnterConditions.Num(); ++Index)
	{
		UObject* Object = Beat.EnterConditions[Index];
		bSelectedObjectStillPresent |= Object && Object == SelectedBeatObject.Get();
		AddObjectRow(*EnterConditionsBox, Object, ESigilStoryBeatObjectList::EnterConditions, Index);
	}
	for (int32 Index = 0; Index < Beat.EnterEvents.Num(); ++Index)
	{
		UObject* Object = Beat.EnterEvents[Index];
		bSelectedObjectStillPresent |= Object && Object == SelectedBeatObject.Get();
		AddObjectRow(*EnterEventsBox, Object, ESigilStoryBeatObjectList::EnterEvents, Index);
	}
	for (int32 Index = 0; Index < Beat.CompleteEvents.Num(); ++Index)
	{
		UObject* Object = Beat.CompleteEvents[Index];
		bSelectedObjectStillPresent |= Object && Object == SelectedBeatObject.Get();
		AddObjectRow(*CompleteEventsBox, Object, ESigilStoryBeatObjectList::CompleteEvents, Index);
	}

	if (!bSelectedObjectStillPresent)
	{
		SelectedBeatObject.Reset();
		BeatObjectDetailsView->SetObject(nullptr);
	}
}

void SSigilStoryEditor::HandleAddBeatObjectClass(
	const UClass* ObjectClass,
	const ESigilStoryBeatObjectList List)
{
	if (!ObjectClass || SelectedBeatId.IsNone())
	{
		return;
	}

	FText Error;
	bSuppressModelRefresh = true;
	const int32 AddedIndex = Model->AddBeatObject(
		SelectedBeatId,
		List,
		const_cast<UClass*>(ObjectClass),
		Error);
	bSuppressModelRefresh = false;
	if (AddedIndex == INDEX_NONE)
	{
		ActionMessage = Error;
		Refresh();
		return;
	}

	ActionMessage = LOCTEXT("StoryBeatObjectAdded", "Condition or Event added.");
	Refresh();
	USigilStoryAsset* StoryAsset = Model->GetAsset();
	const int32 BeatIndex = Model->FindBeatIndex(SelectedBeatId);
	if (!StoryAsset || !StoryAsset->Beats.IsValidIndex(BeatIndex))
	{
		return;
	}
	FSigilStoryBeatDefinition& Beat = StoryAsset->Beats[BeatIndex];
	UObject* AddedObject = List == ESigilStoryBeatObjectList::EnterConditions
		? static_cast<UObject*>(Beat.EnterConditions[AddedIndex].Get())
		: static_cast<UObject*>(
			List == ESigilStoryBeatObjectList::EnterEvents
				? Beat.EnterEvents[AddedIndex].Get()
				: Beat.CompleteEvents[AddedIndex].Get());
	HandleSelectBeatObject(AddedObject);
}

FReply SSigilStoryEditor::HandleSelectBeatObject(UObject* Object)
{
	SelectedBeatObject = Object;
	BeatObjectDetailsView->SetObject(Object);
	return FReply::Handled();
}

FReply SSigilStoryEditor::HandleRemoveBeatObject(
	const ESigilStoryBeatObjectList List,
	const int32 ObjectIndex)
{
	bSuppressModelRefresh = true;
	const bool bRemoved = Model->RemoveBeatObject(SelectedBeatId, List, ObjectIndex);
	bSuppressModelRefresh = false;
	ActionMessage = bRemoved
		? LOCTEXT("StoryBeatObjectRemoved", "Condition or Event removed.")
		: LOCTEXT("StoryBeatObjectRemoveFailed", "The Condition or Event could not be removed.");
	Refresh();
	return FReply::Handled();
}

void SSigilStoryEditor::HandleBeatObjectPropertyChanged(
	const FPropertyChangedEvent& PropertyChangedEvent)
{
	(void)PropertyChangedEvent;
	if (USigilStoryAsset* StoryAsset = Model->GetAsset())
	{
		StoryAsset->MarkPackageDirty();
	}
	Preview.Invalidate();
	ActionMessage = LOCTEXT("StoryBeatObjectChanged", "Condition or Event updated. Restart the safe preview.");
	RefreshValidation();
	RefreshPreviewPanel();
}

void SSigilStoryEditor::NotifyPreChange(FProperty* PropertyAboutToChange)
{
	(void)PropertyAboutToChange;
	USigilStoryAsset* StoryAsset = Model->GetAsset();
	const int32 BeatIndex = Model->FindBeatIndex(SelectedBeatId);
	if (!StoryAsset || !StoryAsset->Beats.IsValidIndex(BeatIndex))
	{
		return;
	}
	StoryAsset->Modify();
	PreChangeBeatIndex = BeatIndex;
	PreChangeBeatId = StoryAsset->Beats[BeatIndex].BeatId;
}

void SSigilStoryEditor::NotifyPostChange(
	const FPropertyChangedEvent& PropertyChangedEvent,
	FProperty* PropertyThatChanged)
{
	(void)PropertyThatChanged;
	const int32 BeatIndex = PreChangeBeatIndex;
	if (BeatIndex == INDEX_NONE)
	{
		if ((PropertyChangedEvent.ChangeType & EPropertyChangeType::Interactive) == 0)
		{
			Refresh();
		}
		return;
	}

	FText Error;
	bSuppressModelRefresh = true;
	Model->ReconcileBeatEdit(BeatIndex, PreChangeBeatId, Error);
	bSuppressModelRefresh = false;
	USigilStoryAsset* StoryAsset = Model->GetAsset();
	if (StoryAsset && StoryAsset->Beats.IsValidIndex(BeatIndex))
	{
		SelectedBeatId = StoryAsset->Beats[BeatIndex].BeatId;
	}
	PreChangeBeatIndex = INDEX_NONE;
	ActionMessage = Error;
	if ((PropertyChangedEvent.ChangeType & EPropertyChangeType::Interactive) != 0)
	{
		Preview.Invalidate();
		RefreshValidation();
		RefreshPreviewPanel();
	}
	else
	{
		Refresh();
	}
}

void SSigilStoryEditor::RefreshBeatList()
{
	BeatListItems.Reset();
	for (const FName BeatId : Model->GetFilteredBeatIds(BeatFilter))
	{
		BeatListItems.Add(MakeShared<FName>(BeatId));
	}
	if (BeatListView.IsValid())
	{
		BeatListView->RequestListRefresh();
		for (const TSharedPtr<FName>& Item : BeatListItems)
		{
			if (Item.IsValid() && *Item == SelectedBeatId)
			{
				BeatListView->SetSelection(Item);
				break;
			}
		}
	}
}

void SSigilStoryEditor::ShowBeatDetails(const FName BeatId)
{
	const int32 BeatIndex = Model->FindBeatIndex(BeatId);
	if (BeatIndex == INDEX_NONE)
	{
		ClearBeatDetails();
		return;
	}
	SelectedBeatId = BeatId;
	SelectedBeatIndex = BeatIndex;
	SelectedBeatScope = MakeShared<FStoryBeatStructOnScope>(Model->GetAsset(), BeatIndex);
	StructureDetailsView->SetStructureData(SelectedBeatScope);
}

void SSigilStoryEditor::ClearBeatDetails()
{
	SelectedBeatId = NAME_None;
	SelectedBeatIndex = INDEX_NONE;
	SelectedBeatScope.Reset();
	StructureDetailsView->SetStructureData(nullptr);
}

void SSigilStoryEditor::RefreshValidation()
{
	FText Error;
	const USigilStoryAsset* StoryAsset = Model->GetAsset();
	if (StoryAsset && StoryAsset->ValidateDefinition(Error))
	{
		ValidationText = LOCTEXT("StoryDefinitionValid", "Definition: Valid");
		ValidationColor = FLinearColor(0.2f, 0.8f, 0.3f);
	}
	else
	{
		ValidationText = FText::Format(
			LOCTEXT("StoryDefinitionInvalid", "Definition: Invalid - {0}"),
			Error);
		ValidationColor = FLinearColor(0.9f, 0.2f, 0.2f);
	}
}

void SSigilStoryEditor::RefreshPreviewPanel()
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
			.Text(LOCTEXT("StorySafePreviewHeading", "Safe Preview"))
			.Font(FAppStyle::GetFontStyle(TEXT("HeadingExtraSmall")))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(10.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("StartStoryPreview", "Start / Restart"))
			.OnClicked(this, &SSigilStoryEditor::HandleStartPreview)
		]
	];
	PreviewBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT(
			"StoryPreviewSafety",
			"Conditions are set manually. Events are recorded but never executed. Beats never advance automatically."))
		.AutoWrapText(true)
	];

	if (!Preview.IsActive())
	{
		return;
	}

	PreviewBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 3.0f)
	[
		SNew(STextBlock)
		.Text(FText::Format(
			LOCTEXT("StoryPreviewActiveBeat", "Active Beat: {0}"),
			Preview.GetActiveBeatId().IsNone()
				? LOCTEXT("StoryPreviewNoActiveBeat", "None")
				: FText::FromName(Preview.GetActiveBeatId())))
	];

	const USigilStoryAsset* StoryAsset = Model->GetAsset();
	if (StoryAsset)
	{
		for (const FSigilStoryBeatDefinition& Beat : StoryAsset->Beats)
		{
			const FName BeatId = Beat.BeatId;
			const bool bCompleted = Preview.IsBeatCompleted(BeatId);
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
						LOCTEXT("StoryPreviewBeatRow", "{0}  [{1}]"),
						FText::FromName(BeatId),
						bCompleted
							? LOCTEXT("StoryPreviewCompleted", "Completed")
							: (Preview.GetActiveBeatId() == BeatId
								? LOCTEXT("StoryPreviewActive", "Active")
								: LOCTEXT("StoryPreviewAvailable", "Available"))))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("EnterStoryPreviewBeat", "Enter"))
					.IsEnabled_Lambda([this, BeatId]()
					{
						FText Reason;
						return Preview.CanEnterBeat(BeatId, Reason);
					})
					.ToolTipText_Lambda([this, BeatId]()
					{
						FText Reason;
						return Preview.CanEnterBeat(BeatId, Reason)
							? LOCTEXT("StoryPreviewBeatReady", "Beat is ready to enter.")
							: Reason;
					})
					.OnClicked(this, &SSigilStoryEditor::HandleEnterBeat, BeatId)
				]
			];

			for (int32 ConditionIndex = 0; ConditionIndex < Beat.EnterConditions.Num(); ++ConditionIndex)
			{
				const FSigilStoryPreviewConditionKey Key{BeatId, ConditionIndex};
				PreviewBox->AddSlot()
				.AutoHeight()
				.Padding(16.0f, 1.0f)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this, Key]()
					{
						switch (Preview.GetConditionResult(Key))
						{
						case ESigilStoryPreviewConditionResult::True:
							return ECheckBoxState::Checked;
						case ESigilStoryPreviewConditionResult::False:
							return ECheckBoxState::Unchecked;
						case ESigilStoryPreviewConditionResult::Unspecified:
						default:
							return ECheckBoxState::Undetermined;
						}
					})
					.OnCheckStateChanged(this, &SSigilStoryEditor::HandleConditionCheckChanged, Key)
					[
						SNew(STextBlock)
						.Text(FText::Format(
							LOCTEXT("StoryPreviewCondition", "Enter Condition {0} (manual)"),
							FText::AsNumber(ConditionIndex + 1)))
					]
				];
			}
		}
	}

	PreviewBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 6.0f, 0.0f, 2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("CompleteStoryPreviewBeat", "Complete Active Beat"))
		.IsEnabled_Lambda([this]() { return !Preview.GetActiveBeatId().IsNone(); })
		.OnClicked(this, &SSigilStoryEditor::HandleCompleteBeat)
	];
	PreviewBox->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(FText::Format(
			LOCTEXT("StoryPreviewEventLog", "Recorded events: {0} (executed: 0)"),
			FText::AsNumber(Preview.GetEventLog().Num())))
	];
}

void SSigilStoryEditor::HandleModelChanged()
{
	if (bSuppressModelRefresh)
	{
		return;
	}
	ActionMessage = LOCTEXT("StoryPreviewInvalidated", "Asset changed. Restart the safe preview.");
	Refresh();
}

TSharedRef<ITableRow> SSigilStoryEditor::GenerateBeatRow(
	TSharedPtr<FName> Item,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
	[
		SNew(STextBlock)
		.Text(Item.IsValid() ? FText::FromName(*Item) : LOCTEXT("StoryBeatMissing", "Missing beat"))
	];
}

void SSigilStoryEditor::HandleBeatSelectionChanged(
	TSharedPtr<FName> Item,
	const ESelectInfo::Type SelectInfo)
{
	(void)SelectInfo;
	if (Item.IsValid())
	{
		ShowBeatDetails(*Item);
	}
	else
	{
		ClearBeatDetails();
	}
}

void SSigilStoryEditor::HandleFilterChanged(const FText& NewText)
{
	BeatFilter = NewText.ToString();
	RefreshBeatList();
}

FReply SSigilStoryEditor::HandleAddBeat()
{
	bSuppressModelRefresh = true;
	SelectedBeatId = Model->AddBeat();
	bSuppressModelRefresh = false;
	ActionMessage = LOCTEXT("StoryBeatAdded", "Beat added.");
	Refresh();
	return FReply::Handled();
}

FReply SSigilStoryEditor::HandleDuplicateBeat()
{
	bSuppressModelRefresh = true;
	const FName NewBeatId = Model->DuplicateBeat(SelectedBeatId);
	bSuppressModelRefresh = false;
	if (!NewBeatId.IsNone())
	{
		SelectedBeatId = NewBeatId;
		ActionMessage = LOCTEXT("StoryBeatDuplicated", "Beat duplicated.");
	}
	Refresh();
	return FReply::Handled();
}

FReply SSigilStoryEditor::HandleDeleteBeat()
{
	bSuppressModelRefresh = true;
	if (Model->DeleteBeat(SelectedBeatId))
	{
		SelectedBeatId = NAME_None;
		ActionMessage = LOCTEXT("StoryBeatDeleted", "Beat deleted.");
	}
	bSuppressModelRefresh = false;
	Refresh();
	return FReply::Handled();
}

FReply SSigilStoryEditor::HandleStartPreview()
{
	FText Error;
	if (Preview.Start(Model->GetAsset(), Error))
	{
		ActionMessage = LOCTEXT("StoryPreviewStarted", "Safe preview started.");
	}
	else
	{
		ActionMessage = Error;
	}
	RefreshPreviewPanel();
	return FReply::Handled();
}

FReply SSigilStoryEditor::HandleEnterBeat(const FName BeatId)
{
	FText Error;
	if (Preview.EnterBeat(BeatId, Error))
	{
		ActionMessage = FText::Format(
			LOCTEXT("StoryPreviewBeatEntered", "Entered beat {0}."),
			FText::FromName(BeatId));
	}
	else
	{
		ActionMessage = Error;
	}
	RefreshPreviewPanel();
	return FReply::Handled();
}

FReply SSigilStoryEditor::HandleCompleteBeat()
{
	FText Error;
	if (Preview.CompleteActiveBeat(Error))
	{
		ActionMessage = LOCTEXT("StoryPreviewBeatCompleted", "Active beat completed.");
	}
	else
	{
		ActionMessage = Error;
	}
	RefreshPreviewPanel();
	return FReply::Handled();
}

void SSigilStoryEditor::HandleConditionCheckChanged(
	const ECheckBoxState NewState,
	const FSigilStoryPreviewConditionKey Key)
{
	ESigilStoryPreviewConditionResult Result = ESigilStoryPreviewConditionResult::Unspecified;
	if (NewState == ECheckBoxState::Checked)
	{
		Result = ESigilStoryPreviewConditionResult::True;
	}
	else if (NewState == ECheckBoxState::Unchecked)
	{
		Result = ESigilStoryPreviewConditionResult::False;
	}
	Preview.SetConditionResult(Key, Result);
	RefreshPreviewPanel();
}

#undef LOCTEXT_NAMESPACE
