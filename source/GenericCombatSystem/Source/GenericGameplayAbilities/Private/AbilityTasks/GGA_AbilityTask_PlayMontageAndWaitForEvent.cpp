// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AbilityTasks/GGA_AbilityTask_PlayMontageAndWaitForEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GGA_LogChannels.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"

static bool GUseAggressivePlayMontageAndWaitEndTask = true;
static FAutoConsoleVariableRef CVarAggressivePlayMontageAndWaitEndTask(
	TEXT("GGA_AbilitySystem.PlayMontage.AggressiveEndTask"), GUseAggressivePlayMontageAndWaitEndTask,
	TEXT("This should be set to true in order to avoid multiple callbacks off an GGA_AbilityTask_PlayMontageAndWaitForEvent node"));


UGGA_AbilityTask_PlayMontageAndWaitForEvent::UGGA_AbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Rate = 1.f;
	bAllowInterruptAfterBlendOut = false;
	bStopWhenAbilityEnds = true;
}

void UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	const bool bPlayingThisMontage = (Montage == MontageToPlay) && Ability && Ability->GetCurrentMontage() == MontageToPlay;

	if (bPlayingThisMontage)
	{
		if (Montage == MontageToPlay)
		{
			// Reset AnimRootMotionTranslationScale
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}
		}
	}

	if (bPlayingThisMontage && (bInterrupted || !bAllowInterruptAfterBlendOut))
	{
		if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
		{
			ASC->ClearAnimatingAbility(Ability);
		}
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bInterrupted)
		{
			OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());

			if (GUseAggressivePlayMontageAndWaitEndTask)
			{
				EndTask();
			}
		}
		else
		{
			OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnGameplayAbilityCancelled()
{
	if (StopPlayingMontage() || bAllowInterruptAfterBlendOut)
	{
		// Let the BP handle the interrupt as well
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	if (GUseAggressivePlayMontageAndWaitEndTask)
	{
		EndTask();
	}
}

void UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	EndTask();
}

void UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = *Payload;
		TempData.EventTag = EventTag;

		EventReceived.Broadcast(EventTag, TempData);
	}
}

UGGA_AbilityTask_PlayMontageAndWaitForEvent* UGGA_AbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(UGameplayAbility* OwningAbility,
                                                                                                                     FName TaskInstanceName, UAnimMontage* MontageToPlay,
                                                                                                                     FGameplayTagContainer EventTags, float Rate, FName StartSection,
                                                                                                                     bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale,
                                                                                                                     float StartTimeSeconds, bool bAllowInterruptAfterBlendOut)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	UGGA_AbilityTask_PlayMontageAndWaitForEvent* MyObj = NewAbilityTask<UGGA_AbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, TaskInstanceName);
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->EventTags = EventTags;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	MyObj->bAllowInterruptAfterBlendOut = bAllowInterruptAfterBlendOut;
	MyObj->StartTimeSeconds = StartTimeSeconds;

	return MyObj;
}

UGGA_AbilityTask_PlayMontageAndWaitForEvent* UGGA_AbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEventExt(UGameplayAbility* OwningAbility,
                                                                                                                        FGGA_PlayMontageAndWaitForEventTaskParams Params)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Params.Rate);

	UGGA_AbilityTask_PlayMontageAndWaitForEvent* MyObj = NewAbilityTask<UGGA_AbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, Params.TaskInstanceName);
	MyObj->MontageToPlay = Params.MontageToPlay;
	MyObj->EventTags = Params.EventTags;
	MyObj->Rate = Params.Rate;
	MyObj->StartSection = Params.StartSection;
	MyObj->AnimRootMotionTranslationScale = Params.AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = Params.bStopWhenAbilityEnds;
	MyObj->bAllowInterruptAfterBlendOut = Params.bAllowInterruptAfterBlendOut;
	MyObj->StartTimeSeconds = Params.StartTimeSeconds;

	return MyObj;
}

void UGGA_AbilityTask_PlayMontageAndWaitForEvent::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	bool bPlayedMontage = false;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			// Bind to event callback
			EventHandle = ASC->AddGameplayEventTagContainerDelegate(
				EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent));

			if (ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection) > 0.f)
			{
				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnGameplayAbilityCancelled);

				BlendingOutDelegate.BindUObject(this, &UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			UE_LOG(LogGGA_Tasks, Warning, TEXT("GGA_AbilityTask_PlayMontageAndWaitForEvent call to PlayMontage failed!"));
		}
	}
	else
	{
		UE_LOG(LogGGA_Tasks, Warning, TEXT("GGA_AbilityTask_PlayMontageAndWaitForEvent called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		UE_LOG(LogGGA_Tasks, Warning, TEXT("GGA_AbilityTask_PlayMontageAndWaitForEvent called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(),
		       *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	SetWaitingOnAvatar();
}

void UGGA_AbilityTask_PlayMontageAndWaitForEvent::ExternalCancel()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
	}

	Super::ExternalCancel();
}

void UGGA_AbilityTask_PlayMontageAndWaitForEvent::OnDestroy(bool AbilityEnded)
{
	// Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next montage plays.
	// (If we are destroyed, it will detect this and not do anything)

	// This delegate, however, should be cleared as it is a multicast
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(InterruptedHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

void UGGA_AbilityTask_PlayMontageAndWaitForEvent::EndTaskByOwner()
{
	TaskOwnerEnded();
}

bool UGGA_AbilityTask_PlayMontageAndWaitForEvent::StopPlayingMontage()
{
	if (Ability == nullptr)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (ActorInfo == nullptr)
	{
		return false;
	}

	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && Ability)
	{
		if (ASC->GetAnimatingAbility() == Ability
			&& ASC->GetCurrentMontage() == MontageToPlay)
		{
			// Unbind delegates so they don't get called as well
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			ASC->CurrentMontageStop();
			return true;
		}
	}

	return false;
}

FString UGGA_AbilityTask_PlayMontageAndWaitForEvent::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? ToRawPtr(MontageToPlay) : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}
