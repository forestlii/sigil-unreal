// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AsyncTasks/GGA_AsyncTask_WaitGameplayAbilityActivated.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UGGA_AsyncTask_WaitGameplayAbilityActivated* UGGA_AsyncTask_WaitGameplayAbilityActivated::WaitGameplayAbilityActivated(AActor* TargetActor)
{
	UGGA_AsyncTask_WaitGameplayAbilityActivated* MyObj = NewObject<UGGA_AsyncTask_WaitGameplayAbilityActivated>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->SetAbilitySystemComponent(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	return MyObj;
}

void UGGA_AsyncTask_WaitGameplayAbilityActivated::HandleAbilityActivated(UGameplayAbility* Ability)
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

bool UGGA_AsyncTask_WaitGameplayAbilityActivated::ShouldBroadcastDelegates() const
{
	return Super::ShouldBroadcastDelegates();
}

void UGGA_AsyncTask_WaitGameplayAbilityActivated::Activate()
{
	Super::Activate();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		DelegateHandle = ASC->AbilityActivatedCallbacks.AddUObject(this, &UGGA_AsyncTask_WaitGameplayAbilityActivated::HandleAbilityActivated);
	}
	else
	{
		EndAction();
	}
}

void UGGA_AsyncTask_WaitGameplayAbilityActivated::EndAction()
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
