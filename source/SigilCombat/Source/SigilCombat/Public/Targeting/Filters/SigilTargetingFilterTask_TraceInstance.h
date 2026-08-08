// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/TargetingFilterTask_BasicFilterTemplate.h"
#include "SigilTargetingFilterTask_TraceInstance.generated.h"

/**
 * Filters targets based on the CanHitActor check of a collision trace instance.
 * 根据碰撞检测实例的CanHitActor检查过滤目标。
 * @note Requires SourceObject to be a collision trace instance.
 * @注意 需要SourceObject是碰撞检测实例。
 */
UCLASS(meta=(DisplayName="GCS:Filter Task (TraceInstance CanHitActor)"))
class SIGILCOMBAT_API USigilTargetingFilterTask_TraceInstance : public UTargetingFilterTask_BasicFilterTemplate
{
	GENERATED_BODY()

protected:
	/**
	 * Determines if a target should be filtered.
	 * 确定是否应过滤目标。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param TargetData The target data. 目标数据。
	 * @return True if the target should be filtered. 如果应过滤目标返回true。
	 */
	virtual bool ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const override;
};