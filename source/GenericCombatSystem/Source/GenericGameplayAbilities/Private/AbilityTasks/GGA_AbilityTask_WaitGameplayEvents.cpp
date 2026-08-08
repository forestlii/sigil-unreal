// Copyright 2025 RedMoonGames All Rights Reserved.

#include "AbilityTasks/GGA_AbilityTask_WaitGameplayEvents.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GGA_AbilityTask_WaitGameplayEvents)

// ----------------------------------------------------------------

UGGA_AbilityTask_WaitGameplayEvents::UGGA_AbilityTask_WaitGameplayEvents(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGGA_AbilityTask_WaitGameplayEvents* UGGA_AbilityTask_WaitGameplayEvents::WaitGameplayEvents(UGameplayAbility* OwningAbility, FGameplayTagContainer EventTags, AActor* OptionalExternalTarget,
                                                                                             bool OnlyTriggerOnce)
{
	UGGA_AbilityTask_WaitGameplayEvents* MyObj = NewAbilityTask<UGGA_AbilityTask_WaitGameplayEvents>(OwningAbility);
	MyObj->EventTags = EventTags;
	MyObj->SetExternalTarget(OptionalExternalTarget);
	MyObj->OnlyTriggerOnce = OnlyTriggerOnce;

	return MyObj;
}

void UGGA_AbilityTask_WaitGameplayEvents::Activate()
{
	UAbilitySystemComponent* ASC = GetTargetASC();
	if (ASC)
	{
		MyHandle = ASC->AddGameplayEventTagContainerDelegate(
			EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UGGA_AbilityTask_WaitGameplayEvents::GameplayEventContainerCallback));
	}

	Super::Activate();
}

void UGGA_AbilityTask_WaitGameplayEvents::GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ensureMsgf(Payload, TEXT("GameplayEventCallback expected non-null Payload"));
		FGameplayEventData TempPayload = Payload ? *Payload : FGameplayEventData{};
		TempPayload.EventTag = MatchingTag;
		EventReceived.Broadcast(MatchingTag, TempPayload);
	}
	if (OnlyTriggerOnce)
	{
		EndTask();
	}
}

void UGGA_AbilityTask_WaitGameplayEvents::SetExternalTarget(AActor* Actor)
{
	if (Actor)
	{
		UseExternalTarget = true;
		OptionalExternalTarget = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	}
}

UAbilitySystemComponent* UGGA_AbilityTask_WaitGameplayEvents::GetTargetASC()
{
	if (UseExternalTarget)
	{
		return OptionalExternalTarget;
	}

	return AbilitySystemComponent.Get();
}

void UGGA_AbilityTask_WaitGameplayEvents::OnDestroy(bool AbilityEnding)
{
	UAbilitySystemComponent* ASC = GetTargetASC();
	if (ASC && MyHandle.IsValid())
	{
		ASC->RemoveGameplayEventTagContainerDelegate(EventTags, MyHandle);
	}

	Super::OnDestroy(AbilityEnding);
}
