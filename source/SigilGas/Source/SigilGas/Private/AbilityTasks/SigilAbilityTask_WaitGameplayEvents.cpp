// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "AbilityTasks/SigilAbilityTask_WaitGameplayEvents.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAbilityTask_WaitGameplayEvents)

// ----------------------------------------------------------------

USigilAbilityTask_WaitGameplayEvents::USigilAbilityTask_WaitGameplayEvents(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

USigilAbilityTask_WaitGameplayEvents* USigilAbilityTask_WaitGameplayEvents::WaitGameplayEvents(UGameplayAbility* OwningAbility, FGameplayTagContainer EventTags, AActor* OptionalExternalTarget,
                                                                                             bool OnlyTriggerOnce)
{
	USigilAbilityTask_WaitGameplayEvents* MyObj = NewAbilityTask<USigilAbilityTask_WaitGameplayEvents>(OwningAbility);
	MyObj->EventTags = EventTags;
	MyObj->SetExternalTarget(OptionalExternalTarget);
	MyObj->OnlyTriggerOnce = OnlyTriggerOnce;

	return MyObj;
}

void USigilAbilityTask_WaitGameplayEvents::Activate()
{
	UAbilitySystemComponent* ASC = GetTargetASC();
	if (ASC)
	{
		MyHandle = ASC->AddGameplayEventTagContainerDelegate(
			EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &USigilAbilityTask_WaitGameplayEvents::GameplayEventContainerCallback));
	}

	Super::Activate();
}

void USigilAbilityTask_WaitGameplayEvents::GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
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

void USigilAbilityTask_WaitGameplayEvents::SetExternalTarget(AActor* Actor)
{
	if (Actor)
	{
		UseExternalTarget = true;
		OptionalExternalTarget = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	}
}

UAbilitySystemComponent* USigilAbilityTask_WaitGameplayEvents::GetTargetASC()
{
	if (UseExternalTarget)
	{
		return OptionalExternalTarget;
	}

	return AbilitySystemComponent.Get();
}

void USigilAbilityTask_WaitGameplayEvents::OnDestroy(bool AbilityEnding)
{
	UAbilitySystemComponent* ASC = GetTargetASC();
	if (ASC && MyHandle.IsValid())
	{
		ASC->RemoveGameplayEventTagContainerDelegate(EventTags, MyHandle);
	}

	Super::OnDestroy(AbilityEnding);
}
