// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AsyncTasks/GGA_AsyncTask_WaitGameplayAbilityEnded.h"
#include "Runtime/Launch/Resources/Version.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UGGA_AsyncTask_WaitGameplayAbilityEnded* UGGA_AsyncTask_WaitGameplayAbilityEnded::WaitGameplayAbilityEnded(AActor* TargetActor,
                                                                                                           FGameplayTagQuery AbilityQuery)
{
	UGGA_AsyncTask_WaitGameplayAbilityEnded* MyObj = NewObject<UGGA_AsyncTask_WaitGameplayAbilityEnded>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->SetAbilitySystemComponent(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	MyObj->AbilityQuery = AbilityQuery;
	return MyObj;
}

UGGA_AsyncTask_WaitGameplayAbilityEnded* UGGA_AsyncTask_WaitGameplayAbilityEnded::WaitAbilitySpecHandleEnded(AActor* TargetActor, FGameplayAbilitySpecHandle AbilitySpecHandle)
{
	UGGA_AsyncTask_WaitGameplayAbilityEnded* MyObj = NewObject<UGGA_AsyncTask_WaitGameplayAbilityEnded>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->SetAbilitySystemComponent(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	MyObj->AbilitySpecHandle = AbilitySpecHandle;
	return MyObj;
}

void UGGA_AsyncTask_WaitGameplayAbilityEnded::HandleAbilityEnded(const FAbilityEndedData& Data)
{
	if (ShouldBroadcastDelegates())
	{
		if (!AbilityQuery.IsEmpty())
		{
#if ENGINE_MINOR_VERSION > 4
			if (AbilityQuery.Matches(Data.AbilityThatEnded->GetAssetTags()))
#else
			if (AbilityQuery.Matches(Data.AbilityThatEnded->AbilityTags))
#endif
			{
				OnAbilityEnded.Broadcast(Data);
			}
		}

		if (AbilitySpecHandle.IsValid() && AbilitySpecHandle == Data.AbilitySpecHandle)
		{
			OnAbilityEnded.Broadcast(Data);
		}
	}
	else
	{
		EndAction();
	}
}

void UGGA_AsyncTask_WaitGameplayAbilityEnded::Activate()
{
	Super::Activate();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		DelegateHandle = ASC->OnAbilityEnded.AddUObject(this, &UGGA_AsyncTask_WaitGameplayAbilityEnded::HandleAbilityEnded);
	}
	else
	{
		EndAction();
	}
}

void UGGA_AsyncTask_WaitGameplayAbilityEnded::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (DelegateHandle.IsValid())
		{
			ASC->AbilityEndedCallbacks.Remove(DelegateHandle);
		}
	}
	Super::EndAction();
}
