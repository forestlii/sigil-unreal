// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Interaction/Targeting/GGS_TargetingFilterTask_InteractionSmartObjects.h"
#include "Interaction/GGS_InteractionSystemComponent.h"
#include "Interaction/GGS_SmartObjectFunctionLibrary.h"

bool UGGS_TargetingFilterTask_InteractionSmartObjects::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (AActor* Actor = TargetData.HitResult.GetActor())
		{
			if (UGGS_InteractionSystemComponent* InteractionSys = UGGS_InteractionSystemComponent::GetInteractionSystemComponent(SourceContext->SourceActor))
			{
				TArray<FSmartObjectRequestResult> Results;

				return !UGGS_SmartObjectFunctionLibrary::FindSmartObjectsWithInteractionEntranceInActor(InteractionSys->GetSmartObjectRequestFilter(), Actor, Results, InteractionSys->GetOwner());
			}
		}
	}
	return true;
}
