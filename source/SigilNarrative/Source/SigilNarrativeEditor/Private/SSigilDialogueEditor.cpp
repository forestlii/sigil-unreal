// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SSigilDialogueEditor.h"

#include "IStructureDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "SigilDialogueEditorModel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

FDialogueNodeStructOnScope::FDialogueNodeStructOnScope(
	USigilDialogueAsset* InAsset,
	const int32 InNodeIndex)
	: FStructOnScope()
	, Asset(InAsset)
	, NodeIndex(InNodeIndex)
{
}

uint8* FDialogueNodeStructOnScope::GetStructMemory()
{
	USigilDialogueAsset* DialogueAsset = Asset.Get();
	return DialogueAsset && DialogueAsset->Nodes.IsValidIndex(NodeIndex)
		? reinterpret_cast<uint8*>(&DialogueAsset->Nodes[NodeIndex])
		: nullptr;
}

const uint8* FDialogueNodeStructOnScope::GetStructMemory() const
{
	const USigilDialogueAsset* DialogueAsset = Asset.Get();
	return DialogueAsset && DialogueAsset->Nodes.IsValidIndex(NodeIndex)
		? reinterpret_cast<const uint8*>(&DialogueAsset->Nodes[NodeIndex])
		: nullptr;
}

const UScriptStruct* FDialogueNodeStructOnScope::GetStruct() const
{
	return FSigilDialogueNode::StaticStruct();
}

UPackage* FDialogueNodeStructOnScope::GetPackage() const
{
	return Asset.IsValid() ? Asset->GetOutermost() : nullptr;
}

void FDialogueNodeStructOnScope::SetPackage(UPackage* InPackage)
{
	(void)InPackage;
}

bool FDialogueNodeStructOnScope::IsValid() const
{
	return GetStructMemory() != nullptr;
}

void SSigilDialogueEditor::Construct(const FArguments& InArgs, USigilDialogueAsset* InAsset)
{
	check(InAsset);
	Model = MakeShared<FSigilDialogueEditorModel>(InAsset);
	ModelChangedHandle = Model->OnModelChanged().AddSP(this, &SSigilDialogueEditor::HandleModelChanged);

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowObjectLabel = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.ShouldForceHideProperty =
		FDetailsViewArgs::FShouldForceHideProperty::CreateSP(
			this,
			&SSigilDialogueEditor::ShouldHideProperty);

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
		LOCTEXT("DialogueNodeDetails", "Dialogue Node"));

	ChildSlot
	[
		SNew(SSplitter)
		+ SSplitter::Slot()
		.Value(0.32f)
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSearchBox)
					.HintText(LOCTEXT("SearchNodes", "Search node IDs"))
					.OnTextChanged(this, &SSigilDialogueEditor::OnSearchTextChanged)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(2.0f)
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddLine", "Add Line"))
						.OnClicked_Lambda([this]() { return AddNode(ESigilDialogueNodeType::Line); })
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddChoice", "Add Choice"))
						.OnClicked_Lambda([this]() { return AddNode(ESigilDialogueNodeType::Choice); })
					]
					+ SUniformGridPanel::Slot(2, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddEnd", "Add End"))
						.OnClicked_Lambda([this]() { return AddNode(ESigilDialogueNodeType::End); })
					]
					+ SUniformGridPanel::Slot(0, 1)
					[
						SNew(SButton)
						.Text(LOCTEXT("DuplicateNode", "Duplicate"))
						.IsEnabled(this, &SSigilDialogueEditor::HasSelectedNode)
						.OnClicked(this, &SSigilDialogueEditor::DuplicateSelectedNode)
					]
					+ SUniformGridPanel::Slot(1, 1)
					[
						SNew(SButton)
						.Text(LOCTEXT("DeleteNode", "Delete"))
						.IsEnabled(this, &SSigilDialogueEditor::HasSelectedNode)
						.OnClicked(this, &SSigilDialogueEditor::DeleteSelectedNode)
					]
					+ SUniformGridPanel::Slot(2, 1)
					[
						SNew(SButton)
						.Text(LOCTEXT("SetEntryNode", "Set Entry"))
						.IsEnabled(this, &SSigilDialogueEditor::HasSelectedNode)
						.OnClicked(this, &SSigilDialogueEditor::SetSelectedNodeAsEntry)
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SAssignNew(NodeListView, SListView<TSharedPtr<FName>>)
					.ListItemsSource(&NodeItems)
					.OnGenerateRow(this, &SSigilDialogueEditor::GenerateNodeRow)
					.OnSelectionChanged(this, &SSigilDialogueEditor::OnNodeSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
			]
		]
		+ SSplitter::Slot()
		.Value(0.68f)
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					StructureDetailsView->GetWidget().ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(this, &SSigilDialogueEditor::GetErrorText)
					.Visibility(this, &SSigilDialogueEditor::GetErrorVisibility)
					.ColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(this, &SSigilDialogueEditor::GetValidationText)
					.AutoWrapText(true)
				]
			]
		]
	];

	SelectedNodeId = InAsset->EntryNodeId;
	RefreshNodeList();
}

SSigilDialogueEditor::~SSigilDialogueEditor()
{
	if (Model.IsValid() && ModelChangedHandle.IsValid())
	{
		Model->OnModelChanged().Remove(ModelChangedHandle);
	}
}

void SSigilDialogueEditor::Refresh()
{
	RefreshNodeList();
}

void SSigilDialogueEditor::NotifyPreChange(FProperty* PropertyAboutToChange)
{
	USigilDialogueAsset* DialogueAsset = Model.IsValid() ? Model->GetAsset() : nullptr;
	if (!DialogueAsset || !DialogueAsset->Nodes.IsValidIndex(SelectedNodeIndex))
	{
		return;
	}

	DialogueAsset->Modify();
	PreviousNodeId = DialogueAsset->Nodes[SelectedNodeIndex].NodeId;
	PreviousNodeType = DialogueAsset->Nodes[SelectedNodeIndex].NodeType;
}

void SSigilDialogueEditor::NotifyPostChange(
	const FPropertyChangedEvent& PropertyChangedEvent,
	FProperty* PropertyThatChanged)
{
	USigilDialogueAsset* DialogueAsset = Model.IsValid() ? Model->GetAsset() : nullptr;
	if (!DialogueAsset || !DialogueAsset->Nodes.IsValidIndex(SelectedNodeIndex))
	{
		return;
	}

	FText ReconcileError;
	SelectedNodeId = DialogueAsset->Nodes[SelectedNodeIndex].NodeId;
	bSuppressModelRefresh = true;
	const bool bReconciled = Model->ReconcileNodeEdit(
		SelectedNodeIndex,
		PreviousNodeId,
		PreviousNodeType,
		ReconcileError);
	bSuppressModelRefresh = false;
	SelectedNodeId = DialogueAsset->Nodes.IsValidIndex(SelectedNodeIndex)
		? DialogueAsset->Nodes[SelectedNodeIndex].NodeId
		: NAME_None;
	ErrorText = bReconciled ? FText::GetEmpty() : ReconcileError;
	RefreshNodeList();
}

void SSigilDialogueEditor::RefreshNodeList()
{
	const FName DesiredSelection = SelectedNodeId;
	ClearNodeDetails();
	NodeItems.Reset();

	if (Model.IsValid())
	{
		for (const FName NodeId : Model->GetFilteredNodeIds(FilterText))
		{
			NodeItems.Add(MakeShared<FName>(NodeId));
		}
	}

	if (!NodeListView.IsValid())
	{
		return;
	}

	NodeListView->RequestListRefresh();
	TSharedPtr<FName> ItemToSelect;
	for (const TSharedPtr<FName>& Item : NodeItems)
	{
		if (Item.IsValid() && *Item == DesiredSelection)
		{
			ItemToSelect = Item;
			break;
		}
	}

	if (!ItemToSelect.IsValid())
	{
		const USigilDialogueAsset* DialogueAsset = Model.IsValid() ? Model->GetAsset() : nullptr;
		if (DialogueAsset)
		{
			for (const TSharedPtr<FName>& Item : NodeItems)
			{
				if (Item.IsValid() && *Item == DialogueAsset->EntryNodeId)
				{
					ItemToSelect = Item;
					break;
				}
			}
		}
	}

	if (!ItemToSelect.IsValid() && !NodeItems.IsEmpty())
	{
		ItemToSelect = NodeItems[0];
	}

	if (ItemToSelect.IsValid())
	{
		NodeListView->SetSelection(ItemToSelect, ESelectInfo::Direct);
	}
	else
	{
		SelectedNodeId = NAME_None;
		NodeListView->ClearSelection();
	}
}

void SSigilDialogueEditor::ShowNodeDetails(const FName NodeId)
{
	ClearNodeDetails();
	if (!Model.IsValid() || !StructureDetailsView.IsValid())
	{
		return;
	}

	SelectedNodeIndex = Model->FindNodeIndex(NodeId);
	if (SelectedNodeIndex == INDEX_NONE)
	{
		SelectedNodeId = NAME_None;
		return;
	}

	SelectedNodeId = NodeId;
	SelectedNodeStruct = MakeShared<FDialogueNodeStructOnScope>(
		Model->GetAsset(),
		SelectedNodeIndex);
	StructureDetailsView->SetCustomName(FText::FromName(NodeId));
	StructureDetailsView->SetStructureData(SelectedNodeStruct);
}

void SSigilDialogueEditor::ClearNodeDetails()
{
	if (StructureDetailsView.IsValid())
	{
		StructureDetailsView->SetStructureData(nullptr);
	}
	SelectedNodeStruct.Reset();
	SelectedNodeIndex = INDEX_NONE;
}

void SSigilDialogueEditor::HandleModelChanged()
{
	if (!bSuppressModelRefresh)
	{
		RefreshNodeList();
	}
}

void SSigilDialogueEditor::OnSearchTextChanged(const FText& SearchText)
{
	FilterText = SearchText.ToString();
	RefreshNodeList();
}

void SSigilDialogueEditor::OnNodeSelectionChanged(
	const TSharedPtr<FName> Item,
	const ESelectInfo::Type SelectInfo)
{
	if (Item.IsValid())
	{
		ShowNodeDetails(*Item);
	}
	else
	{
		SelectedNodeId = NAME_None;
		ClearNodeDetails();
	}
}

TSharedRef<ITableRow> SSigilDialogueEditor::GenerateNodeRow(
	const TSharedPtr<FName> Item,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
	[
		SNew(STextBlock)
		.Text(Item.IsValid() ? GetNodeRowText(*Item) : FText::GetEmpty())
	];
}

FReply SSigilDialogueEditor::AddNode(const ESigilDialogueNodeType NodeType)
{
	if (!Model.IsValid())
	{
		return FReply::Handled();
	}

	ClearNodeDetails();
	bSuppressModelRefresh = true;
	const FName NewNodeId = Model->AddNode(NodeType);
	bSuppressModelRefresh = false;
	SelectedNodeId = NewNodeId;
	ErrorText = FText::GetEmpty();
	RefreshNodeList();
	return FReply::Handled();
}

FReply SSigilDialogueEditor::DuplicateSelectedNode()
{
	if (!Model.IsValid() || SelectedNodeId.IsNone())
	{
		return FReply::Handled();
	}

	const FName SourceNodeId = SelectedNodeId;
	ClearNodeDetails();
	bSuppressModelRefresh = true;
	const FName NewNodeId = Model->DuplicateNode(SourceNodeId);
	bSuppressModelRefresh = false;
	SelectedNodeId = NewNodeId;
	ErrorText = FText::GetEmpty();
	RefreshNodeList();
	return FReply::Handled();
}

FReply SSigilDialogueEditor::DeleteSelectedNode()
{
	if (!Model.IsValid() || SelectedNodeId.IsNone())
	{
		return FReply::Handled();
	}

	const FName NodeIdToDelete = SelectedNodeId;
	ClearNodeDetails();
	FText DeleteReason;
	bSuppressModelRefresh = true;
	const bool bDeleted = Model->DeleteNode(NodeIdToDelete, DeleteReason);
	bSuppressModelRefresh = false;
	SelectedNodeId = bDeleted ? NAME_None : NodeIdToDelete;
	ErrorText = bDeleted ? FText::GetEmpty() : DeleteReason;
	RefreshNodeList();
	return FReply::Handled();
}

FReply SSigilDialogueEditor::SetSelectedNodeAsEntry()
{
	if (!Model.IsValid() || SelectedNodeId.IsNone())
	{
		return FReply::Handled();
	}

	bSuppressModelRefresh = true;
	const bool bSetEntry = Model->SetEntryNode(SelectedNodeId);
	bSuppressModelRefresh = false;
	ErrorText = bSetEntry
		? FText::GetEmpty()
		: LOCTEXT("SetEntryFailed", "The selected node no longer exists.");
	RefreshNodeList();
	return FReply::Handled();
}

bool SSigilDialogueEditor::HasSelectedNode() const
{
	return Model.IsValid()
		&& !SelectedNodeId.IsNone()
		&& Model->FindNodeIndex(SelectedNodeId) != INDEX_NONE;
}

bool SSigilDialogueEditor::ShouldHideProperty(
	const TSharedRef<IPropertyHandle>& PropertyHandle) const
{
	const USigilDialogueAsset* DialogueAsset = Model.IsValid() ? Model->GetAsset() : nullptr;
	const FProperty* Property = PropertyHandle->GetProperty();
	if (!DialogueAsset || !Property || !DialogueAsset->Nodes.IsValidIndex(SelectedNodeIndex))
	{
		return false;
	}

	const ESigilDialogueNodeType NodeType = DialogueAsset->Nodes[SelectedNodeIndex].NodeType;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(FSigilDialogueNode, NextNodeId))
	{
		return NodeType == ESigilDialogueNodeType::Choice || NodeType == ESigilDialogueNodeType::End;
	}
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(FSigilDialogueNode, Options))
	{
		return NodeType != ESigilDialogueNodeType::Choice;
	}
	return false;
}

FText SSigilDialogueEditor::GetValidationText() const
{
	const USigilDialogueAsset* DialogueAsset = Model.IsValid() ? Model->GetAsset() : nullptr;
	if (!DialogueAsset)
	{
		return LOCTEXT("MissingDialogueAsset", "Dialogue asset is unavailable.");
	}

	FText ValidationError;
	return DialogueAsset->ValidateDefinition(ValidationError)
		? LOCTEXT("ValidDialogueDefinition", "Definition is valid")
		: FText::Format(
			LOCTEXT("InvalidDialogueDefinition", "Definition needs attention: {0}"),
			ValidationError);
}

FText SSigilDialogueEditor::GetErrorText() const
{
	return ErrorText;
}

EVisibility SSigilDialogueEditor::GetErrorVisibility() const
{
	return ErrorText.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

FText SSigilDialogueEditor::GetNodeRowText(const FName NodeId) const
{
	const USigilDialogueAsset* DialogueAsset = Model.IsValid() ? Model->GetAsset() : nullptr;
	const FSigilDialogueNode* Node = DialogueAsset ? DialogueAsset->FindNode(NodeId) : nullptr;
	if (!Node)
	{
		return FText::FromName(NodeId);
	}

	FString NodeTypeText;
	switch (Node->NodeType)
	{
	case ESigilDialogueNodeType::Choice:
		NodeTypeText = TEXT("Choice");
		break;
	case ESigilDialogueNodeType::End:
		NodeTypeText = TEXT("End");
		break;
	case ESigilDialogueNodeType::Line:
	default:
		NodeTypeText = TEXT("Line");
		break;
	}

	const FString EntryMarker = DialogueAsset->EntryNodeId == NodeId ? TEXT("▶ ") : FString();
	const FString SpeakerText = Node->SpeakerId.IsNone() ? TEXT("-") : Node->SpeakerId.ToString();
	return FText::FromString(FString::Printf(
		TEXT("%s%s  [%s]  %s"),
		*EntryMarker,
		*NodeId.ToString(),
		*NodeTypeText,
		*SpeakerText));
}

#undef LOCTEXT_NAMESPACE
