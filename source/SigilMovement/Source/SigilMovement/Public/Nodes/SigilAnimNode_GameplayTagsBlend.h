// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "AnimNodes/AnimNode_BlendListBase.h"
#include "SigilAnimNode_GameplayTagsBlend.generated.h"

USTRUCT()
struct SIGILMOVEMENT_API FSigilAnimNode_GameplayTagsBlend : public FAnimNode_BlendListBase
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="Settings", Meta = (FoldProperty, PinShownByDefault))
	FGameplayTag ActiveTag;

	UPROPERTY(EditAnywhere, Category="Settings", Meta = (FoldProperty))
	TArray<FGameplayTag> Tags;
#endif

protected:
	virtual int32 GetActiveChildIndex() override;

public:
	const FGameplayTag& GetActiveTag() const;

	const TArray<FGameplayTag>& GetTags() const;

#if WITH_EDITOR
	void RefreshPoses();
#endif
};
