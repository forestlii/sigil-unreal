// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilCombat/Public/AbilitySystem/Tasks/SigilAbilityTask_CollisionTrace.h"
#include "SigilCombatLogChannels.h"
#include "Collision/SigilCollisionSystemComponent.h"
#include "Collision/SigilCollisionTraceInstance.h"
#include "CombatFlow/SigilAttackRequest.h"
#include "Utility/SigilCombatFunctionLibrary.h"


USigilAbilityTask_CollisionTrace* USigilAbilityTask_CollisionTrace::HandleCollisionTraces(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool bAdjustVisibilityBasedAnimTickOption)
{
	USigilAbilityTask_CollisionTrace* MyTask = NewAbilityTask<USigilAbilityTask_CollisionTrace>(OwningAbility, TaskInstanceName);
	MyTask->bAdjustAnimTickOption = bAdjustVisibilityBasedAnimTickOption;
	return MyTask;
}

void USigilAbilityTask_CollisionTrace::Activate()
{
	Super::Activate();

	if (bAdjustAnimTickOption && GetAvatarActor()->GetNetMode() == NM_DedicatedServer)
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = USigilCombatFunctionLibrary::GetMainCharacterMeshComponent(GetAvatarActor()))
		{
			if (SkeletalMeshComponent->VisibilityBasedAnimTickOption != EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones)
			{
				PrevAnimTickOption = SkeletalMeshComponent->VisibilityBasedAnimTickOption;
				SkeletalMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
				bAdjustAnimTickOption = true;
			}
		}
	}

	if (USigilCollisionSystemComponent* CollisionSystem = USigilCollisionSystemComponent::GetCollisionSystemComponent(GetAvatarActor()))
	{
		CollisionSystem->OnTraceInstanceHitEvent.AddDynamic(this, &ThisClass::TraceInstanceHitCallback);
	}
}

void USigilAbilityTask_CollisionTrace::OnDestroy(bool bInOwnerFinished)
{
	if (bAdjustAnimTickOption && bAdjustedAnimTickOption)
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = USigilCombatFunctionLibrary::GetMainCharacterMeshComponent(GetAvatarActor()))
		{
			SkeletalMeshComponent->VisibilityBasedAnimTickOption = PrevAnimTickOption;
		}
	}
	if (USigilCollisionSystemComponent* CollisionSystem = USigilCollisionSystemComponent::GetCollisionSystemComponent(GetAvatarActor()))
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

void USigilAbilityTask_CollisionTrace::TraceInstanceHitCallback(USigilCollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
	if (ShouldBroadcastAbilityTaskDelegates() && !MeleeRequests.IsEmpty())
	{
		TObjectPtr<const USigilAttackRequest_Melee> Req = nullptr;
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

void USigilAbilityTask_CollisionTrace::AddMeleeRequest(const USigilAttackRequest_Melee* Request)
{
	if (IsValid(Request) && !MeleeRequests.Contains(Request))
	{
		const FGameplayTagContainer& TracesToControl = Request->TracesToControl;
		if (USigilCollisionSystemComponent* CollisionSystem = USigilCollisionSystemComponent::GetCollisionSystemComponent(GetAvatarActor()))
		{
			TArray<USigilCollisionTraceInstance*> InactiveTraces = CollisionSystem->GetTraceInstances().FilterByPredicate([](const USigilCollisionTraceInstance* TraceInstance)
			{
				return !TraceInstance->bTraceActive && TraceInstance->TraceGameplayTag.IsValid();
			});

			for (USigilCollisionTraceInstance* TraceInstance : InactiveTraces)
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

void USigilAbilityTask_CollisionTrace::RemoveMeleeRequest(const USigilAttackRequest_Melee* Request)
{
	if (IsValid(Request) && MeleeRequests.Contains(Request))
	{
		TArray<TObjectPtr<USigilCollisionTraceInstance>>& Instances = MeleeRequests[Request];

		for (auto& TraceInstance : Instances)
		{
			TraceInstance->ToggleTraceState(false);
		}
		MeleeRequests.Remove(Request);
	}
}
