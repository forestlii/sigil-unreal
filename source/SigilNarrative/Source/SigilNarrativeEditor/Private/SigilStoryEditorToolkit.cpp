// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilStoryEditorToolkit.h"

#include "SSigilStoryEditor.h"
#include "SigilStoryAsset.h"

#include "Editor.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

const FName FSigilStoryEditorToolkit::StoryEditorTabId(TEXT("SigilStoryEditor"));

FSigilStoryEditorToolkit::~FSigilStoryEditorToolkit()
{
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}
}

void FSigilStoryEditorToolkit::InitStoryEditor(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	USigilStoryAsset* StoryAsset)
{
	check(StoryAsset);
	EditedStoryAsset = StoryAsset;

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(
		TEXT("Standalone_SigilStoryEditor_Layout_v1"))
		->AddArea(
			FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Vertical)
				->Split(
					FTabManager::NewStack()
						->SetHideTabWell(true)
						->AddTab(StoryEditorTabId, ETabState::OpenedTab)));

	InitAssetEditor(
		Mode,
		InitToolkitHost,
		TEXT("SigilStoryEditorApp"),
		Layout,
		true,
		true,
		StoryAsset);

	if (GEditor)
	{
		GEditor->RegisterForUndo(this);
	}
	RegenerateMenusAndToolbars();
}

void FSigilStoryEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("StoryEditorWorkspace", "Sigil Story Editor"));
	InTabManager->RegisterTabSpawner(
		StoryEditorTabId,
		FOnSpawnTab::CreateSP(this, &FSigilStoryEditorToolkit::SpawnStoryEditorTab))
		.SetDisplayName(LOCTEXT("StoryEditorTab", "Story"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FSigilStoryEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(StoryEditorTabId);
}

FName FSigilStoryEditorToolkit::GetToolkitFName() const
{
	return TEXT("SigilStoryEditor");
}

FText FSigilStoryEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("StoryEditorAppLabel", "Sigil Story Editor");
}

FString FSigilStoryEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("StoryEditorWorldCentricPrefix", "Story ").ToString();
}

FLinearColor FSigilStoryEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.46f, 0.34f, 0.86f, 0.5f);
}

void FSigilStoryEditorToolkit::PostUndo(const bool bSuccess)
{
	if (bSuccess && StoryEditorWidget.IsValid())
	{
		StoryEditorWidget->Refresh();
	}
}

void FSigilStoryEditorToolkit::PostRedo(const bool bSuccess)
{
	PostUndo(bSuccess);
}

TSharedRef<SDockTab> FSigilStoryEditorToolkit::SpawnStoryEditorTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == StoryEditorTabId);
	check(EditedStoryAsset.IsValid());
	return SNew(SDockTab)
		.Label(LOCTEXT("StoryEditorTab", "Story"))
		[
			SAssignNew(StoryEditorWidget, SSigilStoryEditor, EditedStoryAsset.Get())
		];
}

#undef LOCTEXT_NAMESPACE
