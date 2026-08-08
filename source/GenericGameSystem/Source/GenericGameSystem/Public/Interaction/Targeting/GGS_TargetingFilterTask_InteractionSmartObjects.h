// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/TargetingFilterTask_BasicFilterTemplate.h"
#include "GGS_TargetingFilterTask_InteractionSmartObjects.generated.h"

/**
 * Filter task for selecting interactable smart objects.
 * 选择可交互智能对象的过滤任务。
 */
UCLASS(meta=(DisplayName="(GGS)FilterTask:InteractionSmartObject"))
class GENERICGAMESYSTEM_API UGGS_TargetingFilterTask_InteractionSmartObjects : public UTargetingFilterTask_BasicFilterTemplate
{
	GENERATED_BODY()

protected:
	/**
	 * Determines if a target should be filtered based on interaction criteria.
	 * 根据交互标准确定是否过滤目标。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param TargetData The target data. 目标数据。
	 * @return True if the target should be filtered, false otherwise. 如果目标应被过滤返回true，否则返回false。
	 */
	virtual bool ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const override;
};