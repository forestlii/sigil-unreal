// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Targeting/Selections/SigilTargetingSelectionTask_LineTrace.h"

#include "CollisionQueryParams.h"
#include "KismetTraceUtils.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TargetingSystem/TargetingSubsystem.h"

#if ENABLE_DRAW_DEBUG
#include "Engine/Canvas.h"
#endif // ENABLE_DRAW_DEBUG


USigilTargetingSelectionTask_LineTrace::USigilTargetingSelectionTask_LineTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bComplexTrace = false;
	bIgnoreSourceActor = false;
	bIgnoreInstigatorActor = false;
	bGenerateDefaultHitResult = true;
}

void USigilTargetingSelectionTask_LineTrace::Execute(const FTargetingRequestHandle& TargetingHandle) const
{
	Super::Execute(TargetingHandle);

	SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Executing);

	if (IsAsyncTargetingRequest(TargetingHandle))
	{
		ExecuteAsyncTrace(TargetingHandle);
	}
	else
	{
		ExecuteImmediateTrace(TargetingHandle);
	}
}

FVector USigilTargetingSelectionTask_LineTrace::GetSourceLocation_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceActor)
		{
			return SourceContext->SourceActor->GetActorLocation();
		}

		return SourceContext->SourceLocation;
	}

	return FVector::ZeroVector;
}

FVector USigilTargetingSelectionTask_LineTrace::GetSourceOffset_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return DefaultSourceOffset;
}

FVector USigilTargetingSelectionTask_LineTrace::GetTraceDirection_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceActor)
		{
			if (APawn* Pawn = Cast<APawn>(SourceContext->SourceActor))
			{
				return Pawn->GetControlRotation().Vector();
			}
			else
			{
				return SourceContext->SourceActor->GetActorForwardVector();
			}
		}
	}

	return FVector::ZeroVector;
}

float USigilTargetingSelectionTask_LineTrace::GetTraceLength_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return DefaultTraceLength.GetValueAtLevel(GetTraceLevel(TargetingHandle));
}

void USigilTargetingSelectionTask_LineTrace::GetAdditionalActorsToIgnore_Implementation(const FTargetingRequestHandle& TargetingHandle, TArray<AActor*>& OutAdditionalActorsToIgnore) const
{
}

float USigilTargetingSelectionTask_LineTrace::GetTraceLevel_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return 0.0f;
}

void USigilTargetingSelectionTask_LineTrace::ExecuteImmediateTrace(const FTargetingRequestHandle& TargetingHandle) const
{
	if (UWorld* World = GetSourceContextWorld(TargetingHandle))
	{
#if ENABLE_DRAW_DEBUG
		ResetTraceResultsDebugString(TargetingHandle);
#endif // ENABLE_DRAW_DEBUG

		const FVector Direction = GetTraceDirection(TargetingHandle).GetSafeNormal();
		const FVector Start = (GetSourceLocation(TargetingHandle) + GetSourceOffset(TargetingHandle));
		const FVector End = Start + (Direction * GetTraceLength(TargetingHandle));

		FCollisionQueryParams Params(SCENE_QUERY_STAT(ExecuteImmediateTrace), bComplexTrace);
		InitCollisionParams(TargetingHandle, Params);

		bool bHasBlockingHit = false;
		TArray<FHitResult> Hits;
		if (CollisionProfileName.Name != TEXT("NoCollision"))
		{
			bHasBlockingHit = World->LineTraceMultiByProfile(Hits, Start, End, CollisionProfileName.Name, Params);
		}
		else
		{
			const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);
			bHasBlockingHit = World->LineTraceMultiByChannel(Hits, Start, End, CollisionChannel, Params);
		}

#if ENABLE_DRAW_DEBUG
		DrawDebugTrace(TargetingHandle, Start, End, bHasBlockingHit, Hits);
#endif // ENABLE_DRAW_DEBUG

		ProcessHitResults(TargetingHandle, Hits);
	}

	SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);
}

void USigilTargetingSelectionTask_LineTrace::ExecuteAsyncTrace(const FTargetingRequestHandle& TargetingHandle) const
{
	if (UWorld* World = GetSourceContextWorld(TargetingHandle))
	{
		AActor* SourceActor = nullptr;
		if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
		{
			SourceActor = SourceContext->SourceActor;
		}
		const FVector Direction = GetTraceDirection(TargetingHandle).GetSafeNormal();
		const FVector Start = (GetSourceLocation(TargetingHandle) + GetSourceOffset(TargetingHandle));
		const FVector End = Start + (Direction * GetTraceLength(TargetingHandle));

		FCollisionQueryParams Params(SCENE_QUERY_STAT(ExecuteAsyncTrace), bComplexTrace);
		InitCollisionParams(TargetingHandle, Params);

		FTraceDelegate Delegate = FTraceDelegate::CreateUObject(this, &USigilTargetingSelectionTask_LineTrace::HandleAsyncTraceComplete, TargetingHandle);
		if (CollisionProfileName.Name != TEXT("NoCollision"))
		{
			World->AsyncLineTraceByProfile(EAsyncTraceType::Multi, Start, End, CollisionProfileName.Name, Params, &Delegate);
		}
		else
		{
			const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);
			World->AsyncLineTraceByChannel(EAsyncTraceType::Multi, Start, End, CollisionChannel, Params, FCollisionResponseParams::DefaultResponseParam, &Delegate);
		}
	}
	else
	{
		SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);
	}
}

void USigilTargetingSelectionTask_LineTrace::HandleAsyncTraceComplete(const FTraceHandle& InTraceHandle, FTraceDatum& InTraceDatum, FTargetingRequestHandle TargetingHandle) const
{
	if (TargetingHandle.IsValid())
	{
#if ENABLE_DRAW_DEBUG
		ResetTraceResultsDebugString(TargetingHandle);

		// We have to manually find if there is a blocking hit.
		bool bHasBlockingHit = false;
		for (const FHitResult& HitResult : InTraceDatum.OutHits)
		{
			if (HitResult.bBlockingHit)
			{
				bHasBlockingHit = true;
				break;
			}
		}

		DrawDebugTrace(TargetingHandle, InTraceDatum.Start, InTraceDatum.End, bHasBlockingHit, InTraceDatum.OutHits);

#endif // ENABLE_DRAW_DEBUG

		ProcessHitResults(TargetingHandle, InTraceDatum.OutHits);
	}

	SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);
}

void USigilTargetingSelectionTask_LineTrace::ProcessHitResults(const FTargetingRequestHandle& TargetingHandle, const TArray<FHitResult>& Hits) const
{
	if (!TargetingHandle.IsValid())
	{
		return;
	}
	FTargetingDefaultResultsSet& TargetingResults = FTargetingDefaultResultsSet::FindOrAdd(TargetingHandle);

	if (Hits.Num() > 0)
	{
		for (const FHitResult& HitResult : Hits)
		{
			if (!HitResult.GetActor())
			{
				continue;
			}

			bool bAddResult = true;
			for (const FTargetingDefaultResultData& ResultData : TargetingResults.TargetResults)
			{
				if (ResultData.HitResult.GetActor() == HitResult.GetActor())
				{
					bAddResult = false;
					break;
				}
			}

			if (bAddResult)
			{
				FTargetingDefaultResultData* ResultData = new(TargetingResults.TargetResults) FTargetingDefaultResultData();
				ResultData->HitResult = HitResult;
			}
		}

#if ENABLE_DRAW_DEBUG
		BuildTraceResultsDebugString(TargetingHandle, TargetingResults.TargetResults);
#endif // ENABLE_DRAW_DEBUG
	}
	else if (bGenerateDefaultHitResult)
	{
		// If there were no hits, add a default HitResult at the end of the trace
		FHitResult HitResult;
		const FVector Start = (GetSourceLocation(TargetingHandle) + GetSourceOffset(TargetingHandle));
		const FVector End = Start + (GetTraceDirection(TargetingHandle) * GetTraceLength(TargetingHandle));
		// Start param could be player ViewPoint. We want HitResult to always display the StartLocation.
		HitResult.TraceStart = Start;
		HitResult.TraceEnd = End;
		HitResult.Location = End;
		HitResult.ImpactPoint = End;
		FTargetingDefaultResultData* ResultData = new(TargetingResults.TargetResults) FTargetingDefaultResultData();
		ResultData->HitResult = HitResult;
	}
}

void USigilTargetingSelectionTask_LineTrace::InitCollisionParams(const FTargetingRequestHandle& TargetingHandle, FCollisionQueryParams& OutParams) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (bIgnoreSourceActor && SourceContext->SourceActor)
		{
			OutParams.AddIgnoredActor(SourceContext->SourceActor);
		}

		if (bIgnoreInstigatorActor && SourceContext->InstigatorActor)
		{
			OutParams.AddIgnoredActor(SourceContext->InstigatorActor);
		}

		TArray<AActor*> AdditionalActorsToIgnoreArray;
		GetAdditionalActorsToIgnore(TargetingHandle, AdditionalActorsToIgnoreArray);

		if (AdditionalActorsToIgnoreArray.Num() > 0)
		{
			OutParams.AddIgnoredActors(AdditionalActorsToIgnoreArray);
		}
	}
}

#if WITH_EDITOR
bool USigilTargetingSelectionTask_LineTrace::CanEditChange(const FProperty* InProperty) const
{
	bool bCanEdit = Super::CanEditChange(InProperty);

	if (bCanEdit && InProperty)
	{
		const FName PropertyName = InProperty->GetFName();

		if (PropertyName == GET_MEMBER_NAME_CHECKED(USigilTargetingSelectionTask_LineTrace, TraceChannel))
		{
			return (CollisionProfileName.Name == TEXT("NoCollision"));
		}
	}

	return true;
}
#endif // WITH_EDITOR


#if ENABLE_DRAW_DEBUG
void USigilTargetingSelectionTask_LineTrace::DrawDebug(UTargetingSubsystem* TargetingSubsystem, FTargetingDebugInfo& Info, const FTargetingRequestHandle& TargetingHandle, float XOffset, float YOffset,
                                                  int32 MinTextRowsToAdvance) const
{
#if WITH_EDITORONLY_DATA
	if (UTargetingSubsystem::IsTargetingDebugEnabled())
	{
		FTargetingDebugData& DebugData = FTargetingDebugData::FindOrAdd(TargetingHandle);
		FString& ScratchPadString = DebugData.DebugScratchPadStrings.FindOrAdd(GetNameSafe(this));
		if (!ScratchPadString.IsEmpty())
		{
			if (Info.Canvas)
			{
				Info.Canvas->SetDrawColor(FColor::Yellow);
			}

			FString TaskString = FString::Printf(TEXT("Results : %s"), *ScratchPadString);
			TargetingSubsystem->DebugLine(Info, TaskString, XOffset, YOffset, MinTextRowsToAdvance);
		}
	}
#endif // WITH_EDITORONLY_DATA
}

void USigilTargetingSelectionTask_LineTrace::DrawDebugTrace(const FTargetingRequestHandle TargetingHandle, const FVector& StartLocation, const FVector& EndLocation, const bool bHit,
                                                       const TArray<FHitResult>& Hits) const
{
	if (UTargetingSubsystem::IsTargetingDebugEnabled())
	{
		if (UWorld* World = GetSourceContextWorld(TargetingHandle))
		{
			const float DrawTime = UTargetingSubsystem::GetOverrideTargetingLifeTime();
			const EDrawDebugTrace::Type DrawDebugType = DrawTime <= 0.0f ? EDrawDebugTrace::Type::ForOneFrame : EDrawDebugTrace::Type::ForDuration;
			const FLinearColor TraceColor = FLinearColor::Red;
			const FLinearColor TraceHitColor = FLinearColor::Green;
			DrawDebugLineTraceMulti(World, StartLocation, EndLocation, DrawDebugType, bHit, Hits, TraceColor, TraceHitColor, DrawTime);
		}
	}
}

void USigilTargetingSelectionTask_LineTrace::BuildTraceResultsDebugString(const FTargetingRequestHandle& TargetingHandle, const TArray<FTargetingDefaultResultData>& TargetResults) const
{
#if WITH_EDITORONLY_DATA
	if (UTargetingSubsystem::IsTargetingDebugEnabled())
	{
		FTargetingDebugData& DebugData = FTargetingDebugData::FindOrAdd(TargetingHandle);
		FString& ScratchPadString = DebugData.DebugScratchPadStrings.FindOrAdd(GetNameSafe(this));

		for (const FTargetingDefaultResultData& TargetData : TargetResults)
		{
			if (const AActor* Target = TargetData.HitResult.GetActor())
			{
				if (ScratchPadString.IsEmpty())
				{
					ScratchPadString = FString::Printf(TEXT("%s"), *GetNameSafe(Target));
				}
				else
				{
					ScratchPadString += FString::Printf(TEXT(", %s"), *GetNameSafe(Target));
				}
			}
		}
	}
#endif // WITH_EDITORONLY_DATA
}

void USigilTargetingSelectionTask_LineTrace::ResetTraceResultsDebugString(const FTargetingRequestHandle& TargetingHandle) const
{
#if WITH_EDITORONLY_DATA
	FTargetingDebugData& DebugData = FTargetingDebugData::FindOrAdd(TargetingHandle);
	FString& ScratchPadString = DebugData.DebugScratchPadStrings.FindOrAdd(GetNameSafe(this));
	ScratchPadString.Reset();
#endif // WITH_EDITORONLY_DATA
}

#endif // ENABLE_DRAW_DEBUG
