// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "EditorUndoClient.h"
#include "Toolkits/AssetEditorToolkit.h"

class FSpawnTabArgs;
class SDockTab;
class SSigilQuestEditor;
class USigilQuestAsset;

class FSigilQuestEditorToolkit final : public FAssetEditorToolkit, public FEditorUndoClient
{
public:
	virtual ~FSigilQuestEditorToolkit() override;

	void InitQuestEditor(
		EToolkitMode::Type Mode,
		const TSharedPtr<IToolkitHost>& InitToolkitHost,
		USigilQuestAsset* QuestAsset);

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

private:
	TSharedRef<SDockTab> SpawnQuestEditorTab(const FSpawnTabArgs& Args);

	static const FName QuestEditorTabId;
	TWeakObjectPtr<USigilQuestAsset> EditedQuestAsset;
	TSharedPtr<SSigilQuestEditor> QuestEditorWidget;
};
