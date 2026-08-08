// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AbilityTasks/GGA_AbilityTask_WaitInputPressWithTags.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"

UGGA_AbilityTask_WaitInputPressWithTags::UGGA_AbilityTask_WaitInputPressWithTags(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StartTime = 0.f;
	bTestInitialState = false;
}

UGGA_AbilityTask_WaitInputPressWithTags* UGGA_AbilityTask_WaitInputPressWithTags::WaitInputPressWithTags(UGameplayAbility* OwningAbility, FGameplayTagContainer RequiredTags, FGameplayTagContainer IgnoredTags,
                                                                                   bool bTestAlreadyPressed)
{
	UGGA_AbilityTask_WaitInputPressWithTags* Task = NewAbilityTask<UGGA_AbilityTask_WaitInputPressWithTags>(OwningAbility);
	Task->bTestInitialState = bTestAlreadyPressed;
	Task->RequiredTags = RequiredTags;
	Task->IgnoredTags = IgnoredTags;
	return Task;
}

void UGGA_AbilityTask_WaitInputPressWithTags::Activate()
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
			this, &UGGA_AbilityTask_WaitInputPressWithTags::OnPressCallback);
		if (IsForRemoteClient())
		{
			if (!AbilitySystemComponent->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()))
			{
				SetWaitingOnRemotePlayerData();
			}
		}
	}
}

void UGGA_AbilityTask_WaitInputPressWithTags::OnPressCallback()
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
	if (ASC->GetTagCount(FGameplayTag::RequestGameplayTag("State.Interacting"))
		> ASC->GetTagCount(FGameplayTag::RequestGameplayTag("State.InteractingRemoval")))
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

void UGGA_AbilityTask_WaitInputPressWithTags::OnDestroy(bool AbilityEnded)
{
	AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	ClearWaitingOnRemotePlayerData();

	Super::OnDestroy(AbilityEnded);
}

void UGGA_AbilityTask_WaitInputPressWithTags::Reset()
{
	AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	DelegateHandle = AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(
		this, &UGGA_AbilityTask_WaitInputPressWithTags::OnPressCallback);
	if (IsForRemoteClient())
	{
		if (!AbilitySystemComponent->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()))
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}
