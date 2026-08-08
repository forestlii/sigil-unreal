// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Targeting/GCS_TargetingFunctionLibrary.h"
#include "Components/MeshComponent.h"

FTargetingSourceContext UGCS_TargetingFunctionLibrary::GetTargetingSourceContext(FTargetingRequestHandle TargetingHandle)
{
	if (TargetingHandle.IsValid())
	{
		FTargetingSourceContext& SourceContext = FTargetingSourceContext::FindOrAdd(TargetingHandle);
		return SourceContext;
	}

	return FTargetingSourceContext();
}

void UGCS_TargetingFunctionLibrary::GetTargetingResultsActors(FTargetingRequestHandle TargetingHandle, TArray<AActor*>& Targets)
{
	if (TargetingHandle.IsValid())
	{
		if (FTargetingDefaultResultsSet* Results = FTargetingDefaultResultsSet::Find(TargetingHandle))
		{
			for (const FTargetingDefaultResultData& ResultData : Results->TargetResults)
			{
				if (AActor* Target = ResultData.HitResult.GetActor())
				{
					Targets.Add(Target);
				}
			}
		}
	}
}

void UGCS_TargetingFunctionLibrary::GetTargetingResults(FTargetingRequestHandle TargetingHandle, TArray<FHitResult>& OutTargets)
{
	if (TargetingHandle.IsValid())
	{
		if (FTargetingDefaultResultsSet* Results = FTargetingDefaultResultsSet::Find(TargetingHandle))
		{
			for (const FTargetingDefaultResultData& ResultData : Results->TargetResults)
			{
				OutTargets.Add(ResultData.HitResult);
			}
		}
	}
}

FTargetingSourceContext UGCS_TargetingFunctionLibrary::ConvertTargetingLocationInfoToSourceContext(FGameplayAbilityTargetingLocationInfo LocationInfo)
{
	FTargetingSourceContext Context = FTargetingSourceContext();

	//Return or calculate based on LocationType.
	switch (LocationInfo.LocationType)
	{
	case EGameplayAbilityTargetingLocationType::ActorTransform:
		if (LocationInfo.SourceActor)
		{
			Context.SourceActor = LocationInfo.SourceActor;
		}
		break;
	case EGameplayAbilityTargetingLocationType::SocketTransform:
		if (LocationInfo.SourceComponent)
		{
			// Bad socket name will just return component transform anyway, so we're safe
			Context.SourceLocation = LocationInfo.SourceComponent->GetSocketTransform(LocationInfo.SourceSocketName).GetLocation();
		}
		break;
	case EGameplayAbilityTargetingLocationType::LiteralTransform:
		Context.SourceLocation = LocationInfo.LiteralTransform.GetLocation();
	default:
		check(false);
		break;
	}

	return Context;
}
