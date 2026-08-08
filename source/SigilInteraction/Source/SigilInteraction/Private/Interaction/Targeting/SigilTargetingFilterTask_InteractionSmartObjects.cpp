// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/Targeting/SigilTargetingFilterTask_InteractionSmartObjects.h"
#include "Interaction/SigilInteractionSystemComponent.h"
#include "Interaction/SigilSmartObjectFunctionLibrary.h"

bool USigilTargetingFilterTask_InteractionSmartObjects::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (AActor* Actor = TargetData.HitResult.GetActor())
		{
			if (USigilInteractionSystemComponent* InteractionSys = USigilInteractionSystemComponent::GetInteractionSystemComponent(SourceContext->SourceActor))
			{
				TArray<FSmartObjectRequestResult> Results;

				return !USigilSmartObjectFunctionLibrary::FindSmartObjectsWithInteractionEntranceInActor(InteractionSys->GetSmartObjectRequestFilter(), Actor, Results, InteractionSys->GetOwner());
			}
		}
	}
	return true;
}
