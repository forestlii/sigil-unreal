// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "EditorUndoClient.h"
#include "Toolkits/AssetEditorToolkit.h"

class FSpawnTabArgs;
class SDockTab;
class SSigilStoryEditor;
class USigilStoryAsset;

class FSigilStoryEditorToolkit final : public FAssetEditorToolkit, public FEditorUndoClient
{
public:
	virtual ~FSigilStoryEditorToolkit() override;

	void InitStoryEditor(
		EToolkitMode::Type Mode,
		const TSharedPtr<IToolkitHost>& InitToolkitHost,
		USigilStoryAsset* StoryAsset);

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

private:
	TSharedRef<SDockTab> SpawnStoryEditorTab(const FSpawnTabArgs& Args);

	static const FName StoryEditorTabId;
	TWeakObjectPtr<USigilStoryAsset> EditedStoryAsset;
	TSharedPtr<SSigilStoryEditor> StoryEditorWidget;
};
