// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Targeting/Filters/GCS_TargetingFilterTask_TraceInstance.h"

#include "Collision/GCS_CollisionTraceInstance.h"

bool UGCS_TargetingFilterTask_TraceInstance::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	const AActor* TargetActor = TargetData.HitResult.GetActor();

	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (const UGCS_CollisionTraceInstance* TraceInstance = Cast<UGCS_CollisionTraceInstance>(SourceContext->SourceObject))
		{
			return !TraceInstance->CanHitActor(TargetActor);
		}
	}

	return false;
}
