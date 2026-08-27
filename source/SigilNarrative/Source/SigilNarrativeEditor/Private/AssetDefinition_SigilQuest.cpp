// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "AssetDefinition_SigilQuest.h"

#include "SigilQuestAsset.h"
#include "SigilQuestEditorToolkit.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_SigilQuest)

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

FText UAssetDefinition_SigilQuest::GetAssetDisplayName() const
{
	return LOCTEXT("SigilQuestAssetName", "Sigil Quest");
}

FLinearColor UAssetDefinition_SigilQuest::GetAssetColor() const
{
	return FLinearColor(0.85f, 0.48f, 0.16f);
}

TSoftClassPtr<UObject> UAssetDefinition_SigilQuest::GetAssetClass() const
{
	return USigilQuestAsset::StaticClass();
}

EAssetCommandResult UAssetDefinition_SigilQuest::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	for (USigilQuestAsset* QuestAsset : OpenArgs.LoadObjects<USigilQuestAsset>())
	{
		MakeShared<FSigilQuestEditorToolkit>()->InitQuestEditor(
			OpenArgs.GetToolkitMode(),
			OpenArgs.ToolkitHost,
			QuestAsset);
	}
	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE
