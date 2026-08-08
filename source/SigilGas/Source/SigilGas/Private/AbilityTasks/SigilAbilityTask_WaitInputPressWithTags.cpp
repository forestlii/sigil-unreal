// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AbilityTasks/SigilAbilityTask_WaitInputPressWithTags.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "SigilGasTags.h"

USigilAbilityTask_WaitInputPressWithTags::USigilAbilityTask_WaitInputPressWithTags(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StartTime = 0.f;
	bTestInitialState = false;
}

USigilAbilityTask_WaitInputPressWithTags* USigilAbilityTask_WaitInputPressWithTags::WaitInputPressWithTags(UGameplayAbility* OwningAbility, FGameplayTagContainer RequiredTags, FGameplayTagContainer IgnoredTags,
                                                                                   bool bTestAlreadyPressed)
{
	USigilAbilityTask_WaitInputPressWithTags* Task = NewAbilityTask<USigilAbilityTask_WaitInputPressWithTags>(OwningAbility);
	Task->bTestInitialState = bTestAlreadyPressed;
	Task->RequiredTags = RequiredTags;
	Task->IgnoredTags = IgnoredTags;
	return Task;
}

void USigilAbilityTask_WaitInputPressWithTags::Activate()
{
	StartTime = GetWorld()->GetTimeSeconds();
	if (Ability)
	{
		if (bTestInitialState && IsLocallyControlled())
		{
			FGameplayAbilitySpec* Spec = Ability->GetCurrentAbilitySpec();
			if (Spec && Spec->InputPressed)
			{
				OnPressCallback();
				return;
			}
		}

		DelegateHandle = AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(
			this, &USigilAbilityTask_WaitInputPressWithTags::OnPressCallback);
		if (IsForRemoteClient())
		{
			if (!AbilitySystemComponent->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()))
			{
				SetWaitingOnRemotePlayerData();
			}
		}
	}
}

void USigilAbilityTask_WaitInputPressWithTags::OnPressCallback()
{
	float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();

	if (!Ability || !ASC)
	{
		EndTask();
		return;
	}

	//TODO move this into a tag query
	if (ASC->HasAnyMatchingGameplayTags(IgnoredTags) || !ASC->HasAllMatchingGameplayTags(RequiredTags))
	{
		Reset();
		return;
	}

	//TODO extend tag query to support this and move this into it
	// Hardcoded for GA_InteractPassive to ignore input while already interacting
	if (ASC->GetTagCount(SigilStateTags::Interacting)
		> ASC->GetTagCount(SigilStateTags::InteractingRemoval))
	{
		Reset();
		return;
	}

	ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	if (IsPredictingClient())
	{
		// Tell the server about this
		ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey(),
		                                                 ASC->ScopedPredictionKey);
	}
	else
	{
		ASC->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	// We are done. Kill us so we don't keep getting broadcast messages
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnPress.Broadcast(ElapsedTime);
	}

	EndTask();
}

void USigilAbilityTask_WaitInputPressWithTags::OnDestroy(bool AbilityEnded)
{
	AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	ClearWaitingOnRemotePlayerData();

	Super::OnDestroy(AbilityEnded);
}

void USigilAbilityTask_WaitInputPressWithTags::Reset()
{
	AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	DelegateHandle = AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(
		this, &USigilAbilityTask_WaitInputPressWithTags::OnPressCallback);
	if (IsForRemoteClient())
	{
		if (!AbilitySystemComponent->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()))
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}
