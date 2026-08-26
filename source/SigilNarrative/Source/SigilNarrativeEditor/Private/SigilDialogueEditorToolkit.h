// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Toolkits/AssetEditorToolkit.h"

class SDockTab;
class USigilDialogueAsset;
class FSpawnTabArgs;

class FSigilDialogueEditorToolkit final : public FAssetEditorToolkit
{
public:
	void InitDialogueEditor(
		EToolkitMode::Type Mode,
		const TSharedPtr<IToolkitHost>& InitToolkitHost,
		USigilDialogueAsset* DialogueAsset);

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

private:
	TSharedRef<SDockTab> SpawnDialogueEditorTab(const FSpawnTabArgs& Args);

	static const FName DialogueEditorTabId;

	TWeakObjectPtr<USigilDialogueAsset> EditedDialogueAsset;
};
