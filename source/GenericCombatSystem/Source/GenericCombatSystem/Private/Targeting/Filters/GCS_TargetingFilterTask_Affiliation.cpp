// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Targeting/Filters/GCS_TargetingFilterTask_Affiliation.h"

#include "GCS_CombatSystemSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "GCS_CombatTeamAgentInterface.h"
#include "Utility/GCS_CombatFunctionLibrary.h"

bool UGCS_TargetingFilterTask_Affiliation::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	if (const UGCS_CombatSystemSettings*  Settings = UGCS_CombatSystemSettings::Get())
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

FGenericTeamId UGCS_TargetingFilterTask_Affiliation::GetSourceTeamId(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		AActor* Actor = SourceContext->InstigatorActor?SourceContext->InstigatorActor:SourceContext->SourceActor;	
		if (IsValid(Actor))
		{
			if (bLookCombatTeamAgentInterface)
			{
				if (Actor->GetClass()->ImplementsInterface(UGCS_CombatTeamAgentInterface::StaticClass()))
				{
					return IGCS_CombatTeamAgentInterface::Execute_GetCombatTeamId(Actor);
				}
				if (bLookCombatTeamAgentInterfaceInComponents)
				{
					TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(UGCS_CombatTeamAgentInterface::StaticClass());
					if (Components.IsValidIndex(0))
					{
						return IGCS_CombatTeamAgentInterface::Execute_GetCombatTeamId(Components[0]);
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

FGenericTeamId UGCS_TargetingFilterTask_Affiliation::GetTargetTeamId(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	if (const AActor* TargetActor = TargetData.HitResult.GetActor())
	{
		if (bLookCombatTeamAgentInterface)
		{
			if (TargetActor->GetClass()->ImplementsInterface(UGCS_CombatTeamAgentInterface::StaticClass()))
			{
				return IGCS_CombatTeamAgentInterface::Execute_GetCombatTeamId(TargetActor);
			}
			if (bLookCombatTeamAgentInterfaceInComponents)
			{
				TArray<UActorComponent*> Components = TargetActor->GetComponentsByInterface(UGCS_CombatTeamAgentInterface::StaticClass());
				if (Components.IsValidIndex(0))
				{
					return IGCS_CombatTeamAgentInterface::Execute_GetCombatTeamId(Components[0]);
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
