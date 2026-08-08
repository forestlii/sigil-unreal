// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Targeting/Selections/SigilTargetingSelectionTask_TraceExt.h"

#include "SigilCombatLogChannels.h"
#include "Collision/SigilCollisionTraceInstance.h"
#include "Targeting/SigilTargetingSourceInterface.h"

USigilCollisionTraceInstance* USigilTargetingSelectionTask_TraceExt::GetSourceTraceInstance_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceObject)
		{
			return Cast<USigilCollisionTraceInstance>(SourceContext->SourceObject);
		}
	}
	UE_LOG(LogSigilCombat, Error, TEXT("No valid CollisionTraceInstance passed in as SourceObject! TargetingPreset:%s"), *GetOuter()->GetName());
	return nullptr;
}

void USigilTargetingSelectionTask_TraceExt::Execute(const FTargetingRequestHandle& TargetingHandle) const
{
	if (USigilCollisionTraceInstance* TraceInstance = GetSourceTraceInstance(TargetingHandle))
	{
		Super::Execute(TargetingHandle);
	}
	else
	{
		SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);
	}
}

FVector USigilTargetingSelectionTask_TraceExt::GetSourceLocation_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (bUseContextLocationAsSourceLocation)
		{
			return SourceContext->SourceLocation;
		}
		if (SourceContext->SourceActor)
		{
			return SourceContext->SourceActor->GetActorLocation();
		}

		return SourceContext->SourceLocation;
	}

	return FVector::ZeroVector;
}

FVector USigilTargetingSelectionTask_TraceExt::GetTraceDirection_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceObject && SourceContext->SourceObject->GetClass()->ImplementsInterface(USigilTargetingSourceInterface::StaticClass()))
		{
			FVector TraceDirection;
			if (ISigilTargetingSourceInterface::Execute_GetTraceDirection(SourceContext->SourceObject, TraceDirection))
			{
				return TraceDirection;
			}
		}

		return SourceContext->SourceLocation;
	}

	return Super::GetTraceDirection_Implementation(TargetingHandle);
}

void USigilTargetingSelectionTask_TraceExt::GetAdditionalActorsToIgnore_Implementation(const FTargetingRequestHandle& TargetingHandle, TArray<AActor*>& OutAdditionalActorsToIgnore) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceObject && SourceContext->SourceObject->GetClass()->ImplementsInterface(USigilTargetingSourceInterface::StaticClass()))
		{
			OutAdditionalActorsToIgnore.Append(ISigilTargetingSourceInterface::Execute_GetAdditionalActorsToIgnore(SourceContext->SourceObject));
		}
	}
}

float USigilTargetingSelectionTask_TraceExt::GetTraceLevel_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (!IsValid(SourceContext->SourceObject))
		{
			UE_LOG(LogSigilCombat, Error, TEXT("No valid Context Source Object found! TargetingPreset:%s"), *GetOuter()->GetName());
			return 0;
		}
		if (!SourceContext->SourceObject->GetClass()->ImplementsInterface(USigilTargetingSourceInterface::StaticClass()))
		{
			UE_LOG(LogSigilCombat, Error, TEXT("Source Object(%s) doesn't implements SigilTargetingSourceInterface.! TargetingPreset:%s"),
			       *SourceContext->SourceObject->GetName(), *GetOuter()->GetName());
			return 0;
		}
	}
	return 0;
}

float USigilTargetingSelectionTask_TraceExt::GetTraceLength_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return bTraceLengthLevel ? DefaultTraceLength.GetValueAtLevel(GetTraceLevel(TargetingHandle)) : DefaultTraceLength.GetValue();
}

float USigilTargetingSelectionTask_TraceExt::GetSweptTraceRadius_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return bSweptTraceRadiusLevel ? DefaultSweptTraceRadius.GetValueAtLevel(GetTraceLevel(TargetingHandle)) : DefaultSweptTraceRadius.GetValue();
}

float USigilTargetingSelectionTask_TraceExt::GetSweptTraceCapsuleHalfHeight_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return bSweptTraceCapsuleHalfHeightLevel ? DefaultSweptTraceCapsuleHalfHeight.GetValueAtLevel(GetTraceLevel(TargetingHandle)) : DefaultSweptTraceCapsuleHalfHeight.GetValue();
}

FVector USigilTargetingSelectionTask_TraceExt::GetSweptTraceBoxHalfExtents_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (bSweptTraceBoxHalfExtentLevel)
	{
		float Level = GetTraceLevel(TargetingHandle);
		return FVector(DefaultSweptTraceBoxHalfExtentX.GetValueAtLevel(Level), DefaultSweptTraceBoxHalfExtentY.GetValueAtLevel(Level), DefaultSweptTraceBoxHalfExtentZ.GetValueAtLevel(Level));
	}

	return Super::GetSweptTraceBoxHalfExtents_Implementation(TargetingHandle);
}
