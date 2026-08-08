// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/TargetingFilterTask_BasicFilterTemplate.h"
#include "GCS_TargetingFilterTask_IsDead.generated.h"

/**
 * Filters out dead targets.
 * 过滤掉已死亡的目标。
 */
UCLASS(meta=(DisplayName="GCS:FilterTask (IsDead)"))
class GENERICCOMBATSYSTEM_API UGCS_TargetingFilterTask_IsDead : public UTargetingFilterTask_BasicFilterTemplate
{
	GENERATED_BODY()

protected:
	/**
	 * Determines if a target should be filtered based on death state.
	 * 根据死亡状态确定是否应过滤目标。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param TargetData The target data. 目标数据。
	 * @return True if the target is dead and should be filtered. 如果目标已死亡且应过滤返回true。
	 */
	virtual bool ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const override;
};