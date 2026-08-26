// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilDialogueEditorToolkit.h"

#include "SSigilDialogueEditor.h"
#include "SigilDialogueAsset.h"

#include "Editor.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

const FName FSigilDialogueEditorToolkit::DialogueEditorTabId(TEXT("SigilDialogueEditor"));

FSigilDialogueEditorToolkit::~FSigilDialogueEditorToolkit()
{
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}
}

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

	GEditor->RegisterForUndo(this);
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

void FSigilDialogueEditorToolkit::PostUndo(const bool bSuccess)
{
	if (bSuccess && DialogueEditorWidget.IsValid())
	{
		DialogueEditorWidget->Refresh();
	}
}

void FSigilDialogueEditorToolkit::PostRedo(const bool bSuccess)
{
	PostUndo(bSuccess);
}

TSharedRef<SDockTab> FSigilDialogueEditorToolkit::SpawnDialogueEditorTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == DialogueEditorTabId);
	check(EditedDialogueAsset.IsValid());

	return SNew(SDockTab)
		.Label(LOCTEXT("DialogueEditorTab", "Dialogue"))
		[
			SAssignNew(DialogueEditorWidget, SSigilDialogueEditor, EditedDialogueAsset.Get())
		];
}

#undef LOCTEXT_NAMESPACE
