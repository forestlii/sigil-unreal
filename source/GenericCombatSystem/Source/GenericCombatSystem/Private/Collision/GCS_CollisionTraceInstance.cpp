// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Collision/GCS_CollisionTraceInstance.h"

#include "GCS_LogChannels.h"
#include "Collision/GCS_CollisionSystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CombatFlow/GCS_AttackRequest.h"

void UGCS_CollisionTraceInstance::OnTraceBeginPlay_Implementation()
{
	ActiveTime = 0.0f;
	HitActors.Empty();
}

void UGCS_CollisionTraceInstance::OnTraceEndPlay_Implementation()
{
	ActiveTime = 0.0f;
	bTraceActive = false;
	TracePrimitiveComponent = nullptr;
	TracePrimitiveComponentSocketNames.Empty();
	HitActors.Empty();
}

void UGCS_CollisionTraceInstance::BroadcastHit(const FHitResult& HitResult)
{
	if (!bTraceActive)
	{
		UE_LOG(LogGCS_Collision, Warning, TEXT("Hit while inactive,%s"), *TraceOwner->GetName());
		return;
	}
	if (UGCS_CollisionSystemComponent* CSC = TraceOwner->FindComponentByClass<UGCS_CollisionSystemComponent>())
	{
		CSC->OnTraceInstanceHit(this, HitResult);
	}
	if (UGCS_CollisionSystemComponent* TargetCSC = HitResult.GetActor()->FindComponentByClass<UGCS_CollisionSystemComponent>())
	{
		TargetCSC->OnBeTraceInstanceHit(this, HitResult);
	}
	OnHit.Broadcast(HitResult);
}

void UGCS_CollisionTraceInstance::BroadcastStateChanged(bool bNewState)
{
	if (UGCS_CollisionSystemComponent* CSC = TraceOwner->FindComponentByClass<UGCS_CollisionSystemComponent>())
	{
		CSC->OnTraceInstanceStateChanged(this, bNewState);
	}
	OnTraceStateChanged(bNewState);
	OnTraceStateChangedEvent.Broadcast(bNewState);
}

void UGCS_CollisionTraceInstance::OnTraceTick_Implementation(float DeltaSeconds)
{
	ActiveTime += DeltaSeconds;
}

void UGCS_CollisionTraceInstance::OnTraceStateChanged_Implementation(bool bNewState)
{
	bTraceActive = bNewState;
	HitActors.Empty();
}

void UGCS_CollisionTraceInstance::SetTraceMeshInfo(UPrimitiveComponent* NewPrimitiveComponent, TArray<FName> PrimitiveComponentSocketNames)
{
	TracePrimitiveComponent = NewPrimitiveComponent;
	TracePrimitiveComponentSocketNames = PrimitiveComponentSocketNames;
}

bool UGCS_CollisionTraceInstance::CanHitActor_Implementation(const AActor* ActorToCheck) const
{
	//in active trace can not hit anything. TODO make it checkf?
	if (!bTraceActive)
	{
		return false;
	}

	return ActorToCheck != GetTraceSourceActor() && ActorToCheck != TraceOwner && !HitActors.Contains(ActorToCheck);
}

AActor* UGCS_CollisionTraceInstance::GetTraceSourceActor() const
{
	return TracePrimitiveComponent->GetOwner();
}

void UGCS_CollisionTraceInstance::ToggleTraceState(bool bNewState)
{
	if (TraceOwner && bTraceActive != bNewState)
	{
		BroadcastStateChanged(bNewState);
	}
}

void UGCS_CollisionTraceInstance::OnTraceHit_Implementation(const FHitResult& HitResult)
{
	if (HitResult.GetHitObjectHandle().IsValid())
	{
		if (CanHitActor(HitResult.GetActor()))
		{
			UE_LOG(LogGCS_Collision, VeryVerbose, TEXT("%s's Trace(%s, SourceActor:%s) hit actor(%s,Comp:%s)"), *TraceOwner->GetName(),
			       *(TraceGameplayTag.IsValid()?TraceGameplayTag.ToString():GetClass()->GetName()),
			       *GetTraceSourceActor()->GetName(),
			       *HitResult.GetActor()->GetName(), *(HitResult.Component.IsValid()?HitResult.GetComponent()->GetName():TEXT("Null")));
			HitActors.Add(HitResult.GetActor());
			BroadcastHit(HitResult);
		}
	}
}
