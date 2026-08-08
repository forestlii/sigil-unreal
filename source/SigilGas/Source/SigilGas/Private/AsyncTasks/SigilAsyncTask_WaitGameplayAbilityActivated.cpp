// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AsyncTasks/SigilAsyncTask_WaitGameplayAbilityActivated.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

USigilAsyncTask_WaitGameplayAbilityActivated* USigilAsyncTask_WaitGameplayAbilityActivated::WaitGameplayAbilityActivated(AActor* TargetActor)
{
	USigilAsyncTask_WaitGameplayAbilityActivated* MyObj = NewObject<USigilAsyncTask_WaitGameplayAbilityActivated>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->SetAbilitySystemComponent(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	return MyObj;
}

void USigilAsyncTask_WaitGameplayAbilityActivated::HandleAbilityActivated(UGameplayAbility* Ability)
{
	if (ShouldBroadcastDelegates())
	{
		OnAbilityActivated.Broadcast(Ability);
	}
	else
	{
		EndAction();
	}
}

bool USigilAsyncTask_WaitGameplayAbilityActivated::ShouldBroadcastDelegates() const
{
	return Super::ShouldBroadcastDelegates();
}

void USigilAsyncTask_WaitGameplayAbilityActivated::Activate()
{
	Super::Activate();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		DelegateHandle = ASC->AbilityActivatedCallbacks.AddUObject(this, &USigilAsyncTask_WaitGameplayAbilityActivated::HandleAbilityActivated);
	}
	else
	{
		EndAction();
	}
}

void USigilAsyncTask_WaitGameplayAbilityActivated::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (DelegateHandle.IsValid())
		{
			ASC->AbilityActivatedCallbacks.Remove(DelegateHandle);
		}
	}
	Super::EndAction();
}
