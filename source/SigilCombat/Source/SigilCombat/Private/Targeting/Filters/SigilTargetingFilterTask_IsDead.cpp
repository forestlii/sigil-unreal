// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Targeting/Filters/SigilTargetingFilterTask_IsDead.h"

#include "SigilCombatInterface.h"
#include "Utility/SigilCombatFunctionLibrary.h"

bool USigilTargetingFilterTask_IsDead::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	AActor* TargetActor = TargetData.HitResult.GetActor();

	if (UObject* Implementer = USigilCombatFunctionLibrary::GetCombatInterfaceImplementer(TargetActor))
	{
		return ISigilCombatInterface::Execute_IsDead(Implementer);
	}

	return false;
}
