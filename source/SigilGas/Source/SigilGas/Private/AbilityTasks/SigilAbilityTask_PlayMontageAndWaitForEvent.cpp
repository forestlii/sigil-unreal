// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AbilityTasks/SigilAbilityTask_PlayMontageAndWaitForEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SigilAbilitySystemComponent.h"
#include "SigilGasLogChannels.h"
#include "Abilities/SigilGameplayAbility.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

static bool GUseAggressivePlayMontageAndWaitEndTask = true;
static FAutoConsoleVariableRef CVarAggressivePlayMontageAndWaitEndTask(
	TEXT("SigilAbilitySystem.PlayMontage.AggressiveEndTask"), GUseAggressivePlayMontageAndWaitEndTask,
	TEXT("This should be set to true in order to avoid multiple callbacks off an SigilAbilityTask_PlayMontageAndWaitForEvent node"));


USigilAbilityTask_PlayMontageAndWaitForEvent::USigilAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Rate = 1.f;
	bAllowInterruptAfterBlendOut = false;
	bStopWhenAbilityEnds = true;
}

USigilAbilitySystemComponent* USigilAbilityTask_PlayMontageAndWaitForEvent::GetSigilAbilitySystemComponent() const
{
	return Cast<USigilAbilitySystemComponent>(AbilitySystemComponent.Get());
}

bool USigilAbilityTask_PlayMontageAndWaitForEvent::UsesSecondaryMesh() const
{
	if (!Mesh)
	{
		return false;
	}

	const USigilAbilitySystemComponent* SigilASC = GetSigilAbilitySystemComponent();
	return !SigilASC || !SigilASC->IsAvatarMainMesh(Mesh);
}

UAnimInstance* USigilAbilityTask_PlayMontageAndWaitForEvent::GetTargetAnimInstance() const
{
	if (Mesh)
	{
		return Mesh->GetAnimInstance();
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability ? Ability->GetCurrentActorInfo() : nullptr;
	return ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
}

UAnimMontage* USigilAbilityTask_PlayMontageAndWaitForEvent::GetAbilityCurrentMontage() const
{
	if (!Ability)
	{
		return nullptr;
	}

	if (Mesh)
	{
		if (const USigilGameplayAbility* SigilAbility = Cast<USigilGameplayAbility>(Ability))
		{
			return SigilAbility->GetCurrentMontageForMesh(Mesh);
		}
		if (const USigilAbilitySystemComponent* SigilASC = GetSigilAbilitySystemComponent())
		{
			return SigilASC->GetCurrentMontageForMesh(Mesh);
		}
	}

	return Ability->GetCurrentMontage();
}

void USigilAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	const bool bPlayingThisMontage = (Montage == MontageToPlay) && Ability && GetAbilityCurrentMontage() == MontageToPlay;

	if (bPlayingThisMontage && !UsesSecondaryMesh())
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
		if (Mesh)
		{
			if (USigilAbilitySystemComponent* SigilASC = GetSigilAbilitySystemComponent())
			{
				SigilASC->ClearAnimatingAbilityForMesh(Mesh, Ability);
			}
		}
		else if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
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

void USigilAbilityTask_PlayMontageAndWaitForEvent::OnGameplayAbilityCancelled()
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

void USigilAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
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

void USigilAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = *Payload;
		TempData.EventTag = EventTag;

		EventReceived.Broadcast(EventTag, TempData);
	}
}

USigilAbilityTask_PlayMontageAndWaitForEvent* USigilAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(UGameplayAbility* OwningAbility,
                                                                                                                     FName TaskInstanceName, UAnimMontage* MontageToPlay,
                                                                                                                     FGameplayTagContainer EventTags, float Rate, FName StartSection,
                                                                                                                     bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale,
                                                                                                                     float StartTimeSeconds, bool bAllowInterruptAfterBlendOut)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	USigilAbilityTask_PlayMontageAndWaitForEvent* MyObj = NewAbilityTask<USigilAbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, TaskInstanceName);
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

USigilAbilityTask_PlayMontageAndWaitForEvent* USigilAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEventExt(UGameplayAbility* OwningAbility,
                                                                                                                        FSigilPlayMontageAndWaitForEventTaskParams Params)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Params.Rate);

	USigilAbilityTask_PlayMontageAndWaitForEvent* MyObj = NewAbilityTask<USigilAbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, Params.TaskInstanceName);
	MyObj->MontageToPlay = Params.MontageToPlay;
	MyObj->EventTags = Params.EventTags;
	MyObj->Rate = Params.Rate;
	MyObj->StartSection = Params.StartSection;
	MyObj->AnimRootMotionTranslationScale = Params.AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = Params.bStopWhenAbilityEnds;
	MyObj->bAllowInterruptAfterBlendOut = Params.bAllowInterruptAfterBlendOut;
	MyObj->StartTimeSeconds = Params.StartTimeSeconds;
	MyObj->Mesh = Params.Mesh;

	return MyObj;
}

USigilAbilityTask_PlayMontageAndWaitForEvent* USigilAbilityTask_PlayMontageAndWaitForEvent::PlayMontageForMeshAndWaitForEvent(UGameplayAbility* OwningAbility, FName TaskInstanceName,
                                                                                                                            USkeletalMeshComponent* InMesh, UAnimMontage* MontageToPlay,
                                                                                                                            FGameplayTagContainer EventTags, float Rate, FName StartSection,
                                                                                                                            bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale,
                                                                                                                            float StartTimeSeconds, bool bAllowInterruptAfterBlendOut)
{
	USigilAbilityTask_PlayMontageAndWaitForEvent* MyObj = PlayMontageAndWaitForEvent(OwningAbility, TaskInstanceName, MontageToPlay, EventTags, Rate, StartSection, bStopWhenAbilityEnds,
	                                                                                 AnimRootMotionTranslationScale, StartTimeSeconds, bAllowInterruptAfterBlendOut);
	MyObj->Mesh = InMesh;
	return MyObj;
}

void USigilAbilityTask_PlayMontageAndWaitForEvent::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	bool bPlayedMontage = false;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		USigilAbilitySystemComponent* SigilASC = GetSigilAbilitySystemComponent();
		UAnimInstance* AnimInstance = GetTargetAnimInstance();
		if (Mesh && !SigilASC)
		{
			UE_LOG(LogSigilTasks, Warning, TEXT("SigilAbilityTask_PlayMontageAndWaitForEvent: playing on mesh [%s] requires a USigilAbilitySystemComponent."), *GetNameSafe(Mesh));
		}
		else if (Mesh && UsesSecondaryMesh() && !SigilASC->ShouldPlaySecondaryMeshMontages())
		{
			// Not rendered on this machine: no montage, no cancellation - just keep the ability's timing (review P1-B).
			EventHandle = ASC->AddGameplayEventTagContainerDelegate(
				EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &USigilAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent));
			StartSkippedSecondaryMeshTimer();
			bPlayedMontage = true;
		}
		else if (AnimInstance != nullptr)
		{
			// Bind to event callback
			EventHandle = ASC->AddGameplayEventTagContainerDelegate(
				EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &USigilAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent));

			const float Duration = Mesh
				? SigilASC->PlayMontageForMesh(Ability, Mesh, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection, StartTimeSeconds)
				: ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection);
			if (Duration > 0.f)
			{
				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &USigilAbilityTask_PlayMontageAndWaitForEvent::OnGameplayAbilityCancelled);

				BlendingOutDelegate.BindUObject(this, &USigilAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &USigilAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				// Root motion is driven by the avatar's main mesh only; secondary (cosmetic) meshes leave it alone.
				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && !UsesSecondaryMesh() && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			UE_LOG(LogSigilTasks, Warning, TEXT("SigilAbilityTask_PlayMontageAndWaitForEvent call to PlayMontage failed!"));
		}
	}
	else
	{
		UE_LOG(LogSigilTasks, Warning, TEXT("SigilAbilityTask_PlayMontageAndWaitForEvent called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		UE_LOG(LogSigilTasks, Warning, TEXT("SigilAbilityTask_PlayMontageAndWaitForEvent called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(),
		       *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	SetWaitingOnAvatar();
}

void USigilAbilityTask_PlayMontageAndWaitForEvent::StartSkippedSecondaryMeshTimer()
{
	bSecondaryMeshSkipped = true;

	const float PlayLength = MontageToPlay ? MontageToPlay->GetPlayLength() : 0.f;
	const float ScaledLength = Rate > KINDA_SMALL_NUMBER ? FMath::Max(0.f, PlayLength - FMath::Max(0.f, StartTimeSeconds)) / Rate : 0.f;

	UWorld* World = GetWorld();
	if (ScaledLength <= 0.f || !World)
	{
		OnSkippedSecondaryMeshFinished();
		return;
	}

	World->GetTimerManager().SetTimer(SkippedSecondaryMeshTimerHandle, this, &USigilAbilityTask_PlayMontageAndWaitForEvent::OnSkippedSecondaryMeshFinished, ScaledLength, false);
}

void USigilAbilityTask_PlayMontageAndWaitForEvent::OnSkippedSecondaryMeshFinished()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
	}

	EndTask();
}

void USigilAbilityTask_PlayMontageAndWaitForEvent::ExternalCancel()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
	}

	Super::ExternalCancel();
}

void USigilAbilityTask_PlayMontageAndWaitForEvent::OnDestroy(bool AbilityEnded)
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

	if (SkippedSecondaryMeshTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(SkippedSecondaryMeshTimerHandle);
		}
	}

	Super::OnDestroy(AbilityEnded);
}

void USigilAbilityTask_PlayMontageAndWaitForEvent::EndTaskByOwner()
{
	TaskOwnerEnded();
}

bool USigilAbilityTask_PlayMontageAndWaitForEvent::StopPlayingMontage()
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

	UAnimInstance* AnimInstance = GetTargetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	if (Mesh)
	{
		USigilAbilitySystemComponent* SigilASC = GetSigilAbilitySystemComponent();
		if (SigilASC && SigilASC->GetAnimatingAbilityForMesh(Mesh) == Ability && SigilASC->GetCurrentMontageForMesh(Mesh) == MontageToPlay)
		{
			// Unbind delegates so they don't get called as well
			if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			SigilASC->CurrentMontageStopForMesh(Mesh);
			return true;
		}

		return false;
	}

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

FString USigilAbilityTask_PlayMontageAndWaitForEvent::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability)
	{
		UAnimInstance* AnimInstance = GetTargetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? ToRawPtr(MontageToPlay) : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  Mesh: %s  (Currently Playing): %s%s"), *GetNameSafe(MontageToPlay), *GetNameSafe(Mesh), *GetNameSafe(PlayingMontage),
	                       bSecondaryMeshSkipped ? TEXT("  [secondary mesh skipped: not locally controlled]") : TEXT(""));
}
