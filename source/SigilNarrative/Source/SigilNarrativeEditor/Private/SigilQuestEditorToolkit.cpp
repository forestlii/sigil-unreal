// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilQuestEditorToolkit.h"

#include "SSigilQuestEditor.h"
#include "SigilQuestAsset.h"

#include "Editor.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

const FName FSigilQuestEditorToolkit::QuestEditorTabId(TEXT("SigilQuestEditor"));

FSigilQuestEditorToolkit::~FSigilQuestEditorToolkit()
{
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}
}

void FSigilQuestEditorToolkit::InitQuestEditor(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	USigilQuestAsset* QuestAsset)
{
	check(QuestAsset);
	EditedQuestAsset = QuestAsset;

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(
		TEXT("Standalone_SigilQuestEditor_Layout_v1"))
		->AddArea(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split(
				FTabManager::NewStack()
				->SetHideTabWell(true)
				->AddTab(QuestEditorTabId, ETabState::OpenedTab)));

	InitAssetEditor(
		Mode,
		InitToolkitHost,
		TEXT("SigilQuestEditorApp"),
		Layout,
		true,
		true,
		QuestAsset);

	if (GEditor)
	{
		GEditor->RegisterForUndo(this);
	}
	RegenerateMenusAndToolbars();
}

void FSigilQuestEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("QuestEditorWorkspace", "Sigil Quest Editor"));
	InTabManager->RegisterTabSpawner(
		QuestEditorTabId,
		FOnSpawnTab::CreateSP(this, &FSigilQuestEditorToolkit::SpawnQuestEditorTab))
		.SetDisplayName(LOCTEXT("QuestEditorTab", "Quest"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FSigilQuestEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(QuestEditorTabId);
}

FName FSigilQuestEditorToolkit::GetToolkitFName() const
{
	return TEXT("SigilQuestEditor");
}

FText FSigilQuestEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("QuestEditorAppLabel", "Sigil Quest Editor");
}

FString FSigilQuestEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("QuestEditorWorldCentricPrefix", "Quest ").ToString();
}

FLinearColor FSigilQuestEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.85f, 0.48f, 0.16f, 0.5f);
}

void FSigilQuestEditorToolkit::PostUndo(const bool bSuccess)
{
	if (bSuccess && QuestEditorWidget.IsValid())
	{
		QuestEditorWidget->Refresh();
	}
}

void FSigilQuestEditorToolkit::PostRedo(const bool bSuccess)
{
	PostUndo(bSuccess);
}

TSharedRef<SDockTab> FSigilQuestEditorToolkit::SpawnQuestEditorTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == QuestEditorTabId);
	check(EditedQuestAsset.IsValid());
	return SNew(SDockTab)
		.Label(LOCTEXT("QuestEditorTab", "Quest"))
		[
			SAssignNew(QuestEditorWidget, SSigilQuestEditor, EditedQuestAsset.Get())
		];
}

#undef LOCTEXT_NAMESPACE
