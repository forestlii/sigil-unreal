// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Tasks/TargetingFilterTask_BasicFilterTemplate.h"
#include "GCS_TargetingFilterTask_TagsRequirements.generated.h"

/**
 * Filters targets based on a gameplay tag query.
 * 根据游戏标签查询过滤目标。
 */
UCLASS(meta=(DisplayName="GCS:FilterTask (TagsRequirements)"))
class GENERICCOMBATSYSTEM_API UGCS_TargetingFilterTask_TagsRequirements : public UTargetingFilterTask_BasicFilterTemplate
{
	GENERATED_BODY()

protected:
	/**
	 * Whether to invert the filter result.
	 * 是否反转过滤结果。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	bool bInvert{false};

	/**
	 * The tag query that targets must match.
	 * 目标必须匹配的标签查询。
	 * @note If empty, no filtering is applied. 如果为空，不应用过滤。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter", meta = (DisplayName = "Query Must Match"))
	FGameplayTagQuery TagQuery;

	/**
	 * Whether to fall back to GameplayTagAssetInterface if AbilitySystemComponent fails.
	 * 如果AbilitySystemComponent失败，是否回退到GameplayTagAssetInterface。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	bool bLookingForTagAssetInterface{false};

	/**
	 * Determines if a target should be filtered.
	 * 确定是否应过滤目标。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param TargetData The target data. 目标数据。
	 * @return True if the target should be filtered. 如果应过滤目标返回true。
	 */
	virtual bool ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const override;
};