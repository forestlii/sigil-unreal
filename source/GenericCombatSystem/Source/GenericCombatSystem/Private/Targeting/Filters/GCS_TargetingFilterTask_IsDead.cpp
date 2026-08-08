// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Targeting/Filters/GCS_TargetingFilterTask_IsDead.h"

#include "GCS_CombatInterface.h"
#include "Utility/GCS_CombatFunctionLibrary.h"

bool UGCS_TargetingFilterTask_IsDead::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	AActor* TargetActor = TargetData.HitResult.GetActor();

	if (UObject* Implementer = UGCS_CombatFunctionLibrary::GetCombatInterfaceImplementer(TargetActor))
	{
		return IGCS_CombatInterface::Execute_IsDead(Implementer);
	}

	return false;
}
