// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Targeting/Filters/SigilTargetingFilterTask_Affiliation.h"

#include "SigilCombatSystemSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "SigilCombatTeamAgentInterface.h"
#include "Utility/SigilCombatFunctionLibrary.h"

bool USigilTargetingFilterTask_Affiliation::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	if (const USigilCombatSystemSettings*  Settings = USigilCombatSystemSettings::Get())
	{
		if (Settings->bDisableAffiliationCheck)
		{
			return false;
		}
	}
	FGenericTeamId SourceTeamId = GetSourceTeamId(TargetingHandle, TargetData);
	FGenericTeamId TargetTeamId = GetTargetTeamId(TargetingHandle, TargetData);

	return FAISenseAffiliationFilter::ShouldSenseTeam(SourceTeamId, TargetTeamId, DetectionByAffiliation.GetAsFlags()) == false;
}

FGenericTeamId USigilTargetingFilterTask_Affiliation::GetSourceTeamId(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		AActor* Actor = SourceContext->InstigatorActor?SourceContext->InstigatorActor:SourceContext->SourceActor;	
		if (IsValid(Actor))
		{
			if (bLookCombatTeamAgentInterface)
			{
				if (Actor->GetClass()->ImplementsInterface(USigilCombatTeamAgentInterface::StaticClass()))
				{
					return ISigilCombatTeamAgentInterface::Execute_GetCombatTeamId(Actor);
				}
				if (bLookCombatTeamAgentInterfaceInComponents)
				{
					TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(USigilCombatTeamAgentInterface::StaticClass());
					if (Components.IsValidIndex(0))
					{
						return ISigilCombatTeamAgentInterface::Execute_GetCombatTeamId(Components[0]);
					}
				}
			}

			if (bLookGenericTeamAgentInterface)
			{
				if (const IGenericTeamAgentInterface* GenericTeamAgentInterface = Cast<IGenericTeamAgentInterface>(Actor))
				{
					return GenericTeamAgentInterface->GetGenericTeamId();
				}
				if (bLookController)
				{
					if (const APawn* Pawn = Cast<APawn>(Actor))
					{
						if (const IGenericTeamAgentInterface* GenericTeamAgentInterface = Cast<IGenericTeamAgentInterface>(Pawn->GetController()))
						{
							return GenericTeamAgentInterface->GetGenericTeamId();
						}
					}
				}
			}
		}
	}
	return FGenericTeamId::NoTeam;
}

FGenericTeamId USigilTargetingFilterTask_Affiliation::GetTargetTeamId(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	if (const AActor* TargetActor = TargetData.HitResult.GetActor())
	{
		if (bLookCombatTeamAgentInterface)
		{
			if (TargetActor->GetClass()->ImplementsInterface(USigilCombatTeamAgentInterface::StaticClass()))
			{
				return ISigilCombatTeamAgentInterface::Execute_GetCombatTeamId(TargetActor);
			}
			if (bLookCombatTeamAgentInterfaceInComponents)
			{
				TArray<UActorComponent*> Components = TargetActor->GetComponentsByInterface(USigilCombatTeamAgentInterface::StaticClass());
				if (Components.IsValidIndex(0))
				{
					return ISigilCombatTeamAgentInterface::Execute_GetCombatTeamId(Components[0]);
				}
			}
		}

		if (bLookGenericTeamAgentInterface)
		{
			if (const IGenericTeamAgentInterface* GenericTeamAgentInterface = Cast<IGenericTeamAgentInterface>(TargetActor))
			{
				return GenericTeamAgentInterface->GetGenericTeamId();
			}

			if (bLookController)
			{
				if (const APawn* Pawn = Cast<APawn>(TargetActor))
				{
					if (const IGenericTeamAgentInterface* GenericTeamAgentInterface = Cast<IGenericTeamAgentInterface>(Pawn->GetController()))
					{
						return GenericTeamAgentInterface->GetGenericTeamId();
					}
				}
			}
		}
	}
	return FGenericTeamId::NoTeam;
}
