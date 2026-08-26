// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "AssetDefinition_SigilDialogue.h"

#include "SigilDialogueAsset.h"
#include "SigilDialogueEditorToolkit.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_SigilDialogue)

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

FText UAssetDefinition_SigilDialogue::GetAssetDisplayName() const
{
	return LOCTEXT("SigilDialogueAssetName", "Sigil Dialogue");
}

FLinearColor UAssetDefinition_SigilDialogue::GetAssetColor() const
{
	return FLinearColor(0.18f, 0.45f, 0.85f);
}

TSoftClassPtr<UObject> UAssetDefinition_SigilDialogue::GetAssetClass() const
{
	return USigilDialogueAsset::StaticClass();
}

EAssetCommandResult UAssetDefinition_SigilDialogue::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	for (USigilDialogueAsset* DialogueAsset : OpenArgs.LoadObjects<USigilDialogueAsset>())
	{
		MakeShared<FSigilDialogueEditorToolkit>()->InitDialogueEditor(
			OpenArgs.GetToolkitMode(),
			OpenArgs.ToolkitHost,
			DialogueAsset);
	}

	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE
