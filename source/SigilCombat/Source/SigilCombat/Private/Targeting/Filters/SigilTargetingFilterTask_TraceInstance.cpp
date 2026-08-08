// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Targeting/Filters/SigilTargetingFilterTask_TraceInstance.h"

#include "Collision/SigilCollisionTraceInstance.h"

bool USigilTargetingFilterTask_TraceInstance::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	const AActor* TargetActor = TargetData.HitResult.GetActor();

	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (const USigilCollisionTraceInstance* TraceInstance = Cast<USigilCollisionTraceInstance>(SourceContext->SourceObject))
		{
			return !TraceInstance->CanHitActor(TargetActor);
		}
	}

	return false;
}
