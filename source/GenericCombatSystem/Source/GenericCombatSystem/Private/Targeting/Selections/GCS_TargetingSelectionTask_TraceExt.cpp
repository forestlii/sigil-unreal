// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Targeting/Selections/GCS_TargetingSelectionTask_TraceExt.h"

#include "GCS_LogChannels.h"
#include "Collision/GCS_CollisionTraceInstance.h"
#include "Targeting/GCS_TargetingSourceInterface.h"

UGCS_CollisionTraceInstance* UGCS_TargetingSelectionTask_TraceExt::GetSourceTraceInstance_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceObject)
		{
			return Cast<UGCS_CollisionTraceInstance>(SourceContext->SourceObject);
		}
	}
	UE_LOG(LogGCS, Error, TEXT("No valid CollisionTraceInstance passed in as SourceObject! TargetingPreset:%s"), *GetOuter()->GetName());
	return nullptr;
}

void UGCS_TargetingSelectionTask_TraceExt::Execute(const FTargetingRequestHandle& TargetingHandle) const
{
	if (UGCS_CollisionTraceInstance* TraceInstance = GetSourceTraceInstance(TargetingHandle))
	{
		Super::Execute(TargetingHandle);
	}
	else
	{
		SetTaskAsyncState(TargetingHandle, ETargetingTaskAsyncState::Completed);
	}
}

FVector UGCS_TargetingSelectionTask_TraceExt::GetSourceLocation_Implementation(const FTargetingRequestHandle& TargetingHandle) const
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

FVector UGCS_TargetingSelectionTask_TraceExt::GetTraceDirection_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceObject && SourceContext->SourceObject->GetClass()->ImplementsInterface(UGCS_TargetingSourceInterface::StaticClass()))
		{
			FVector TraceDirection;
			if (IGCS_TargetingSourceInterface::Execute_GetTraceDirection(SourceContext->SourceObject, TraceDirection))
			{
				return TraceDirection;
			}
		}

		return SourceContext->SourceLocation;
	}

	return Super::GetTraceDirection_Implementation(TargetingHandle);
}

void UGCS_TargetingSelectionTask_TraceExt::GetAdditionalActorsToIgnore_Implementation(const FTargetingRequestHandle& TargetingHandle, TArray<AActor*>& OutAdditionalActorsToIgnore) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceObject && SourceContext->SourceObject->GetClass()->ImplementsInterface(UGCS_TargetingSourceInterface::StaticClass()))
		{
			OutAdditionalActorsToIgnore.Append(IGCS_TargetingSourceInterface::Execute_GetAdditionalActorsToIgnore(SourceContext->SourceObject));
		}
	}
}

float UGCS_TargetingSelectionTask_TraceExt::GetTraceLevel_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (!IsValid(SourceContext->SourceObject))
		{
			UE_LOG(LogGCS, Error, TEXT("No valid Context Source Object found! TargetingPreset:%s"), *GetOuter()->GetName());
			return 0;
		}
		if (!SourceContext->SourceObject->GetClass()->ImplementsInterface(UGCS_TargetingSourceInterface::StaticClass()))
		{
			UE_LOG(LogGCS, Error, TEXT("Source Object(%s) doesn't implements GCS_TargetingSourceInterface.! TargetingPreset:%s"),
			       *SourceContext->SourceObject->GetName(), *GetOuter()->GetName());
			return 0;
		}
	}
	return 0;
}

float UGCS_TargetingSelectionTask_TraceExt::GetTraceLength_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return bTraceLengthLevel ? DefaultTraceLength.GetValueAtLevel(GetTraceLevel(TargetingHandle)) : DefaultTraceLength.GetValue();
}

float UGCS_TargetingSelectionTask_TraceExt::GetSweptTraceRadius_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return bSweptTraceRadiusLevel ? DefaultSweptTraceRadius.GetValueAtLevel(GetTraceLevel(TargetingHandle)) : DefaultSweptTraceRadius.GetValue();
}

float UGCS_TargetingSelectionTask_TraceExt::GetSweptTraceCapsuleHalfHeight_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	return bSweptTraceCapsuleHalfHeightLevel ? DefaultSweptTraceCapsuleHalfHeight.GetValueAtLevel(GetTraceLevel(TargetingHandle)) : DefaultSweptTraceCapsuleHalfHeight.GetValue();
}

FVector UGCS_TargetingSelectionTask_TraceExt::GetSweptTraceBoxHalfExtents_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (bSweptTraceBoxHalfExtentLevel)
	{
		float Level = GetTraceLevel(TargetingHandle);
		return FVector(DefaultSweptTraceBoxHalfExtentX.GetValueAtLevel(Level), DefaultSweptTraceBoxHalfExtentY.GetValueAtLevel(Level), DefaultSweptTraceBoxHalfExtentZ.GetValueAtLevel(Level));
	}

	return Super::GetSweptTraceBoxHalfExtents_Implementation(TargetingHandle);
}
