// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Collision/SigilCollisionTraceInstance.h"

#include "SigilCombatLogChannels.h"
#include "Collision/SigilCollisionSystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CombatFlow/SigilAttackRequest.h"

void USigilCollisionTraceInstance::OnTraceBeginPlay_Implementation()
{
	ActiveTime = 0.0f;
	HitActors.Empty();
}

void USigilCollisionTraceInstance::OnTraceEndPlay_Implementation()
{
	ActiveTime = 0.0f;
	bTraceActive = false;
	TracePrimitiveComponent = nullptr;
	TracePrimitiveComponentSocketNames.Empty();
	HitActors.Empty();
}

void USigilCollisionTraceInstance::BroadcastHit(const FHitResult& HitResult)
{
	if (!bTraceActive)
	{
		UE_LOG(LogSigilCombat_Collision, Warning, TEXT("Hit while inactive,%s"), *TraceOwner->GetName());
		return;
	}
	if (USigilCollisionSystemComponent* CSC = TraceOwner->FindComponentByClass<USigilCollisionSystemComponent>())
	{
		CSC->OnTraceInstanceHit(this, HitResult);
	}
	if (USigilCollisionSystemComponent* TargetCSC = HitResult.GetActor()->FindComponentByClass<USigilCollisionSystemComponent>())
	{
		TargetCSC->OnBeTraceInstanceHit(this, HitResult);
	}
	OnHit.Broadcast(HitResult);
}

void USigilCollisionTraceInstance::BroadcastStateChanged(bool bNewState)
{
	if (USigilCollisionSystemComponent* CSC = TraceOwner->FindComponentByClass<USigilCollisionSystemComponent>())
	{
		CSC->OnTraceInstanceStateChanged(this, bNewState);
	}
	OnTraceStateChanged(bNewState);
	OnTraceStateChangedEvent.Broadcast(bNewState);
}

void USigilCollisionTraceInstance::OnTraceTick_Implementation(float DeltaSeconds)
{
	ActiveTime += DeltaSeconds;
}

void USigilCollisionTraceInstance::OnTraceStateChanged_Implementation(bool bNewState)
{
	bTraceActive = bNewState;
	HitActors.Empty();
}

void USigilCollisionTraceInstance::SetTraceMeshInfo(UPrimitiveComponent* NewPrimitiveComponent, TArray<FName> PrimitiveComponentSocketNames)
{
	TracePrimitiveComponent = NewPrimitiveComponent;
	TracePrimitiveComponentSocketNames = PrimitiveComponentSocketNames;
}

bool USigilCollisionTraceInstance::CanHitActor_Implementation(const AActor* ActorToCheck) const
{
	//in active trace can not hit anything. TODO make it checkf?
	if (!bTraceActive)
	{
		return false;
	}

	return ActorToCheck != GetTraceSourceActor() && ActorToCheck != TraceOwner && !HitActors.Contains(ActorToCheck);
}

AActor* USigilCollisionTraceInstance::GetTraceSourceActor() const
{
	return TracePrimitiveComponent->GetOwner();
}

void USigilCollisionTraceInstance::ToggleTraceState(bool bNewState)
{
	if (TraceOwner && bTraceActive != bNewState)
	{
		BroadcastStateChanged(bNewState);
	}
}

void USigilCollisionTraceInstance::OnTraceHit_Implementation(const FHitResult& HitResult)
{
	if (HitResult.GetHitObjectHandle().IsValid())
	{
		if (CanHitActor(HitResult.GetActor()))
		{
			UE_LOG(LogSigilCombat_Collision, VeryVerbose, TEXT("%s's Trace(%s, SourceActor:%s) hit actor(%s,Comp:%s)"), *TraceOwner->GetName(),
			       *(TraceGameplayTag.IsValid()?TraceGameplayTag.ToString():GetClass()->GetName()),
			       *GetTraceSourceActor()->GetName(),
			       *HitResult.GetActor()->GetName(), *(HitResult.Component.IsValid()?HitResult.GetComponent()->GetName():TEXT("Null")));
			HitActors.Add(HitResult.GetActor());
			BroadcastHit(HitResult);
		}
	}
}
