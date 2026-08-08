// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Targeting/SigilTargetingFunctionLibrary.h"
#include "Components/MeshComponent.h"

FTargetingSourceContext USigilTargetingFunctionLibrary::GetTargetingSourceContext(FTargetingRequestHandle TargetingHandle)
{
	if (TargetingHandle.IsValid())
	{
		FTargetingSourceContext& SourceContext = FTargetingSourceContext::FindOrAdd(TargetingHandle);
		return SourceContext;
	}

	return FTargetingSourceContext();
}

void USigilTargetingFunctionLibrary::GetTargetingResultsActors(FTargetingRequestHandle TargetingHandle, TArray<AActor*>& Targets)
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

void USigilTargetingFunctionLibrary::GetTargetingResults(FTargetingRequestHandle TargetingHandle, TArray<FHitResult>& OutTargets)
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

FTargetingSourceContext USigilTargetingFunctionLibrary::ConvertTargetingLocationInfoToSourceContext(FGameplayAbilityTargetingLocationInfo LocationInfo)
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
		break;
	default:
		checkNoEntry();
		break;
	}

	return Context;
}
