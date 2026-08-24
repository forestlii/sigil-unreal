// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilNarrativeCatalog.h"

#include "SigilQuestAsset.h"
#include "SigilStoryAsset.h"

bool USigilNarrativeCatalog::ValidateDefinition(FText& OutError) const
{
	auto Fail = [&OutError](const FString& Message)
	{
		OutError = FText::FromString(Message);
		return false;
	};

	OutError = FText::GetEmpty();
	TSet<FName> QuestIds;
	for (const USigilQuestAsset* QuestAsset : QuestAssets)
	{
		if (!QuestAsset)
		{
			return Fail(TEXT("QuestAssets contains a null asset."));
		}

		FText ValidationError;
		if (!QuestAsset->ValidateDefinition(ValidationError))
		{
			return Fail(FString::Printf(
				TEXT("Quest asset %s is invalid: %s"),
				*QuestAsset->GetName(),
				*ValidationError.ToString()));
		}
		if (QuestIds.Contains(QuestAsset->QuestId))
		{
			return Fail(FString::Printf(TEXT("Duplicate QuestId: %s."), *QuestAsset->QuestId.ToString()));
		}
		QuestIds.Add(QuestAsset->QuestId);
	}

	TSet<FName> StoryIds;
	for (const USigilStoryAsset* StoryAsset : StoryAssets)
	{
		if (!StoryAsset)
		{
			return Fail(TEXT("StoryAssets contains a null asset."));
		}

		FText ValidationError;
		if (!StoryAsset->ValidateDefinition(ValidationError))
		{
			return Fail(FString::Printf(
				TEXT("Story asset %s is invalid: %s"),
				*StoryAsset->GetName(),
				*ValidationError.ToString()));
		}
		if (StoryIds.Contains(StoryAsset->StoryId))
		{
			return Fail(FString::Printf(TEXT("Duplicate StoryId: %s."), *StoryAsset->StoryId.ToString()));
		}
		StoryIds.Add(StoryAsset->StoryId);
	}

	return true;
}

USigilQuestAsset* USigilNarrativeCatalog::FindQuest(const FName QuestId) const
{
	const TObjectPtr<USigilQuestAsset>* FoundAsset = QuestAssets.FindByPredicate([QuestId](const USigilQuestAsset* QuestAsset)
	{
		return QuestAsset && QuestAsset->QuestId == QuestId;
	});
	return FoundAsset ? FoundAsset->Get() : nullptr;
}

USigilStoryAsset* USigilNarrativeCatalog::FindStory(const FName StoryId) const
{
	const TObjectPtr<USigilStoryAsset>* FoundAsset = StoryAssets.FindByPredicate([StoryId](const USigilStoryAsset* StoryAsset)
	{
		return StoryAsset && StoryAsset->StoryId == StoryId;
	});
	return FoundAsset ? FoundAsset->Get() : nullptr;
}
