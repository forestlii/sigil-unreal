// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "AssetDefinition_SigilStory.h"

#include "SigilStoryAsset.h"
#include "SigilStoryEditorToolkit.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_SigilStory)

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

FText UAssetDefinition_SigilStory::GetAssetDisplayName() const
{
	return LOCTEXT("SigilStoryAssetName", "Sigil Story");
}

FLinearColor UAssetDefinition_SigilStory::GetAssetColor() const
{
	return FLinearColor(0.46f, 0.34f, 0.86f);
}

TSoftClassPtr<UObject> UAssetDefinition_SigilStory::GetAssetClass() const
{
	return USigilStoryAsset::StaticClass();
}

EAssetCommandResult UAssetDefinition_SigilStory::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	for (USigilStoryAsset* StoryAsset : OpenArgs.LoadObjects<USigilStoryAsset>())
	{
		MakeShared<FSigilStoryEditorToolkit>()->InitStoryEditor(
			OpenArgs.GetToolkitMode(),
			OpenArgs.ToolkitHost,
			StoryAsset);
	}
	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE
