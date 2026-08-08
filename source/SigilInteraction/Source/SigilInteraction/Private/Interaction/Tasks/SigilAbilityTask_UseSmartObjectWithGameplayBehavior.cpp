// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/Tasks/SigilAbilityTask_UseSmartObjectWithGameplayBehavior.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameplayBehavior.h"
#include "GameplayBehaviorConfig.h"
#include "GameplayBehaviorSmartObjectBehaviorDefinition.h"
#include "GameplayBehaviorSubsystem.h"
#include "SigilInteractionLogChannels.h"
#include "SmartObjectComponent.h"
#include "SmartObjectSubsystem.h"

USigilAbilityTask_UseSmartObjectWithGameplayBehavior::USigilAbilityTask_UseSmartObjectWithGameplayBehavior(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	bBehaviorFinished = false;
}

USigilAbilityTask_UseSmartObjectWithGameplayBehavior* USigilAbilityTask_UseSmartObjectWithGameplayBehavior::UseSmartObjectWithGameplayBehavior(UGameplayAbility* OwningAbility,
	FSmartObjectClaimHandle ClaimHandle, ESmartObjectClaimPriority ClaimPriority)
{
	if (OwningAbility == nullptr)
	{
		return nullptr;
	}


	USigilAbilityTask_UseSmartObjectWithGameplayBehavior* MyTask = NewAbilityTask<USigilAbilityTask_UseSmartObjectWithGameplayBehavior>(OwningAbility);
	if (MyTask == nullptr)
	{
		return nullptr;
	}
	MyTask->SetClaimHandle(ClaimHandle);
	return MyTask;
}

void USigilAbilityTask_UseSmartObjectWithGameplayBehavior::Activate()
{
	Super::Activate();
	bool bSuccess = false;
	ON_SCOPE_EXIT
	{
		if (!bSuccess)
		{
			EndTask();
		}
	};

	if (!ensureMsgf(ClaimedHandle.IsValid(), TEXT("SmartObject handle must be valid at this point.")))
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(GetAvatarActor());
	if (Pawn == nullptr)
	{
		SIGIL_INTERACTION_CLOG(Error, "Pawn required to use GameplayBehavior with claim handle: %s.", *LexToString(ClaimedHandle));
		return;
	}
	USmartObjectSubsystem* SmartObjectSubsystem = USmartObjectSubsystem::GetCurrent(Pawn->GetWorld());
	if (!ensureMsgf(SmartObjectSubsystem != nullptr, TEXT("SmartObjectSubsystem must be accessible at this point.")))
	{
		return;
	}

	// A valid claimed handle can point to an object that is no longer part of the simulation
	if (!SmartObjectSubsystem->IsClaimedSmartObjectValid(ClaimedHandle))
	{
		SIGIL_INTERACTION_CLOG(Log, "Claim handle: %s refers to an object that is no longer available.", *LexToString(ClaimedHandle));
		return;
	}

	// Register a callback to be notified if the claimed slot became unavailable
	SmartObjectSubsystem->RegisterSlotInvalidationCallback(ClaimedHandle, FOnSlotInvalidated::CreateUObject(this, &USigilAbilityTask_UseSmartObjectWithGameplayBehavior::OnSlotInvalidated));

	bSuccess = StartInteraction();
}

bool USigilAbilityTask_UseSmartObjectWithGameplayBehavior::StartInteraction()
{
	UWorld* World = GetWorld();
	USmartObjectSubsystem* SmartObjectSubsystem = USmartObjectSubsystem::GetCurrent(World);
	if (!ensure(SmartObjectSubsystem))
	{
		return false;
	}

	const UGameplayBehaviorSmartObjectBehaviorDefinition* SmartObjectGameplayBehaviorDefinition = SmartObjectSubsystem->MarkSlotAsOccupied<
		UGameplayBehaviorSmartObjectBehaviorDefinition>(ClaimedHandle);
	const UGameplayBehaviorConfig* GameplayBehaviorConfig = SmartObjectGameplayBehaviorDefinition != nullptr ? SmartObjectGameplayBehaviorDefinition->GameplayBehaviorConfig : nullptr;
	GameplayBehavior = GameplayBehaviorConfig != nullptr ? GameplayBehaviorConfig->GetBehavior(*World) : nullptr;
	if (GameplayBehavior == nullptr)
	{
		return false;
	}

	const USmartObjectComponent* SmartObjectComponent = SmartObjectSubsystem->GetSmartObjectComponent(ClaimedHandle);
	AActor& InteractorActor = *GetAvatarActor();
	AActor* InteracteeActor = SmartObjectComponent ? SmartObjectComponent->GetOwner() : nullptr;
	const bool bBehaviorActive = UGameplayBehaviorSubsystem::TriggerBehavior(*GameplayBehavior, InteractorActor, GameplayBehaviorConfig, InteracteeActor);
	// Behavior can be successfully triggered AND ended synchronously. We are only interested to register callback when still running
	if (bBehaviorActive)
	{
		OnBehaviorFinishedNotifyHandle = GameplayBehavior->GetOnBehaviorFinishedDelegate().AddUObject(this, &USigilAbilityTask_UseSmartObjectWithGameplayBehavior::OnSmartObjectBehaviorFinished);
	}

	return bBehaviorActive;
}

void USigilAbilityTask_UseSmartObjectWithGameplayBehavior::OnSmartObjectBehaviorFinished(UGameplayBehavior& Behavior, AActor& Avatar, const bool bInterrupted)
{
	// Adding an ensure in case the assumptions change in the future.
	ensure(GetAvatarActor() != nullptr);

	// make sure we handle the right pawn - we can get this notify for a different
	// Avatar if the behavior sending it out is not instanced (CDO is being used to perform actions)
	if (GetAvatarActor() == &Avatar)
	{
		Behavior.GetOnBehaviorFinishedDelegate().Remove(OnBehaviorFinishedNotifyHandle);
		bBehaviorFinished = true;
		EndTask();
	}
}

void USigilAbilityTask_UseSmartObjectWithGameplayBehavior::OnDestroy(bool bInOwnerFinished)
{
	if (ClaimedHandle.IsValid())
	{
		USmartObjectSubsystem* SmartObjectSubsystem = USmartObjectSubsystem::GetCurrent(GetWorld());
		check(SmartObjectSubsystem);
		SmartObjectSubsystem->MarkSlotAsFree(ClaimedHandle);
		SmartObjectSubsystem->UnregisterSlotInvalidationCallback(ClaimedHandle);
		ClaimedHandle.Invalidate();
	}

	if (TaskState != EGameplayTaskState::Finished)
	{
		if (GameplayBehavior != nullptr && bBehaviorFinished)
		{
			OnSucceeded.Broadcast();
		}
		else
		{
			OnFailed.Broadcast();
		}
	}

	Super::OnDestroy(bInOwnerFinished);
}

void USigilAbilityTask_UseSmartObjectWithGameplayBehavior::OnSlotInvalidated(const FSmartObjectClaimHandle& ClaimHandle, const ESmartObjectSlotState State)
{
	if (!bBehaviorFinished && GameplayBehavior != nullptr)
	{
		check(GetAvatarActor());
		GameplayBehavior->GetOnBehaviorFinishedDelegate().Remove(OnBehaviorFinishedNotifyHandle);
		GameplayBehavior->AbortBehavior(*GetAvatarActor());
	}
	EndTask();
}
