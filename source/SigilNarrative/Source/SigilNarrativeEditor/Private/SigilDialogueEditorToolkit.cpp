// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilDialogueEditorToolkit.h"

#include "SigilDialogueAsset.h"

#include "Framework/Docking/TabManager.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

const FName FSigilDialogueEditorToolkit::DialogueEditorTabId(TEXT("SigilDialogueEditor"));

void FSigilDialogueEditorToolkit::InitDialogueEditor(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	USigilDialogueAsset* DialogueAsset)
{
	check(DialogueAsset);
	EditedDialogueAsset = DialogueAsset;

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(
		TEXT("Standalone_SigilDialogueEditor_Layout_v1"))
		->AddArea(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split(
				FTabManager::NewStack()
				->SetHideTabWell(true)
				->AddTab(DialogueEditorTabId, ETabState::OpenedTab)));

	InitAssetEditor(
		Mode,
		InitToolkitHost,
		TEXT("SigilDialogueEditorApp"),
		Layout,
		true,
		true,
		DialogueAsset);

	RegenerateMenusAndToolbars();
}

void FSigilDialogueEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("DialogueEditorWorkspace", "Sigil Dialogue Editor"));
	InTabManager->RegisterTabSpawner(
		DialogueEditorTabId,
		FOnSpawnTab::CreateSP(this, &FSigilDialogueEditorToolkit::SpawnDialogueEditorTab))
		.SetDisplayName(LOCTEXT("DialogueEditorTab", "Dialogue"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FSigilDialogueEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(DialogueEditorTabId);
}

FName FSigilDialogueEditorToolkit::GetToolkitFName() const
{
	return TEXT("SigilDialogueEditor");
}

FText FSigilDialogueEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DialogueEditorAppLabel", "Sigil Dialogue Editor");
}

FString FSigilDialogueEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("DialogueEditorWorldCentricPrefix", "Dialogue ").ToString();
}

FLinearColor FSigilDialogueEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.18f, 0.45f, 0.85f, 0.5f);
}

TSharedRef<SDockTab> FSigilDialogueEditorToolkit::SpawnDialogueEditorTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == DialogueEditorTabId);

	FText ValidationMessage;
	const bool bDefinitionValid = EditedDialogueAsset.IsValid()
		&& EditedDialogueAsset->ValidateDefinition(ValidationMessage);
	const FText AssetName = EditedDialogueAsset.IsValid()
		? FText::FromString(EditedDialogueAsset->GetName())
		: LOCTEXT("MissingDialogueAsset", "Missing dialogue asset");
	const FText StatusText = bDefinitionValid
		? LOCTEXT("ValidDialogueDefinition", "Definition is valid")
		: FText::Format(
			LOCTEXT("InvalidDialogueDefinition", "Definition needs attention: {0}"),
			ValidationMessage);

	return SNew(SDockTab)
		.Label(LOCTEXT("DialogueEditorTab", "Dialogue"))
		[
			SNew(SBorder)
			.Padding(16.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(AssetName)
					.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(StatusText)
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
