// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GenericCombatSystem/Public/AbilitySystem/Tasks/GCS_AbilityTask_CollisionTrace.h"
#include "GCS_LogChannels.h"
#include "Collision/GCS_CollisionSystemComponent.h"
#include "Collision/GCS_CollisionTraceInstance.h"
#include "CombatFlow/GCS_AttackRequest.h"
#include "Utility/GCS_CombatFunctionLibrary.h"


UGCS_AbilityTask_CollisionTrace* UGCS_AbilityTask_CollisionTrace::HandleCollisionTraces(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool bAdjustVisibilityBasedAnimTickOption)
{
	UGCS_AbilityTask_CollisionTrace* MyTask = NewAbilityTask<UGCS_AbilityTask_CollisionTrace>(OwningAbility, TaskInstanceName);
	MyTask->bAdjustAnimTickOption = bAdjustVisibilityBasedAnimTickOption;
	return MyTask;
}

void UGCS_AbilityTask_CollisionTrace::Activate()
{
	Super::Activate();

	if (bAdjustAnimTickOption && GetAvatarActor()->GetNetMode() == NM_DedicatedServer)
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = UGCS_CombatFunctionLibrary::GetMainCharacterMeshComponent(GetAvatarActor()))
		{
			if (SkeletalMeshComponent->VisibilityBasedAnimTickOption != EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones)
			{
				PrevAnimTickOption = SkeletalMeshComponent->VisibilityBasedAnimTickOption;
				SkeletalMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
				bAdjustAnimTickOption = true;
			}
		}
	}

	if (UGCS_CollisionSystemComponent* CollisionSystem = UGCS_CollisionSystemComponent::GetCollisionSystemComponent(GetAvatarActor()))
	{
		CollisionSystem->OnTraceInstanceHitEvent.AddDynamic(this, &ThisClass::TraceInstanceHitCallback);
	}
}

void UGCS_AbilityTask_CollisionTrace::OnDestroy(bool bInOwnerFinished)
{
	if (bAdjustAnimTickOption && bAdjustedAnimTickOption)
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = UGCS_CombatFunctionLibrary::GetMainCharacterMeshComponent(GetAvatarActor()))
		{
			SkeletalMeshComponent->VisibilityBasedAnimTickOption = PrevAnimTickOption;
		}
	}
	if (UGCS_CollisionSystemComponent* CollisionSystem = UGCS_CollisionSystemComponent::GetCollisionSystemComponent(GetAvatarActor()))
	{
		CollisionSystem->OnTraceInstanceHitEvent.RemoveDynamic(this, &ThisClass::TraceInstanceHitCallback);
	}

	for (auto& MeleeRequest : MeleeRequests)
	{
		for (auto& TraceInstance : MeleeRequest.Value)
		{
			TraceInstance->ToggleTraceState(false);
		}
	}

	MeleeRequests.Empty();

	Super::OnDestroy(bInOwnerFinished);
}

void UGCS_AbilityTask_CollisionTrace::TraceInstanceHitCallback(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
	if (ShouldBroadcastAbilityTaskDelegates() && !MeleeRequests.IsEmpty())
	{
		TObjectPtr<const UGCS_AttackRequest_Melee> Req = nullptr;
		bool bFound = false;
		for (auto& MeleeRequest : MeleeRequests)
		{
			if (MeleeRequest.Value.Contains(TraceInstance))
			{
				Req  = MeleeRequest.Key;
				bFound = true;
				break;
			}
		}
		if (bFound)
		{
			OnTargetsFound.Broadcast(Req, TraceInstance, HitResult);
		}
	}
}

void UGCS_AbilityTask_CollisionTrace::AddMeleeRequest(const UGCS_AttackRequest_Melee* Request)
{
	if (IsValid(Request) && !MeleeRequests.Contains(Request))
	{
		const FGameplayTagContainer& TracesToControl = Request->TracesToControl;
		if (UGCS_CollisionSystemComponent* CollisionSystem = UGCS_CollisionSystemComponent::GetCollisionSystemComponent(GetAvatarActor()))
		{
			TArray<UGCS_CollisionTraceInstance*> InactiveTraces = CollisionSystem->GetTraceInstances().FilterByPredicate([](const UGCS_CollisionTraceInstance* TraceInstance)
			{
				return !TraceInstance->bTraceActive && TraceInstance->TraceGameplayTag.IsValid();
			});

			for (UGCS_CollisionTraceInstance* TraceInstance : InactiveTraces)
			{
				if (TracesToControl.HasTagExact(TraceInstance->TraceGameplayTag))
				{
					TraceInstance->ToggleTraceState(true);
					auto& Instances = MeleeRequests.FindOrAdd(Request);
					Instances.Add(TraceInstance);
				}
			}
		}
	}
}

void UGCS_AbilityTask_CollisionTrace::RemoveMeleeRequest(const UGCS_AttackRequest_Melee* Request)
{
	if (IsValid(Request) && MeleeRequests.Contains(Request))
	{
		TArray<TObjectPtr<UGCS_CollisionTraceInstance>>& Instances = MeleeRequests[Request];

		for (auto& TraceInstance : Instances)
		{
			TraceInstance->ToggleTraceState(false);
		}
		MeleeRequests.Remove(Request);
	}
}
