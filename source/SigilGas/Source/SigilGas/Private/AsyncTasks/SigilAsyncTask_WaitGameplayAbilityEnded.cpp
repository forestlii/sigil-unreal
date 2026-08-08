// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AsyncTasks/SigilAsyncTask_WaitGameplayAbilityEnded.h"
#include "Runtime/Launch/Resources/Version.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

USigilAsyncTask_WaitGameplayAbilityEnded* USigilAsyncTask_WaitGameplayAbilityEnded::WaitGameplayAbilityEnded(AActor* TargetActor,
                                                                                                           FGameplayTagQuery AbilityQuery)
{
	USigilAsyncTask_WaitGameplayAbilityEnded* MyObj = NewObject<USigilAsyncTask_WaitGameplayAbilityEnded>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->SetAbilitySystemComponent(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	MyObj->AbilityQuery = AbilityQuery;
	return MyObj;
}

USigilAsyncTask_WaitGameplayAbilityEnded* USigilAsyncTask_WaitGameplayAbilityEnded::WaitAbilitySpecHandleEnded(AActor* TargetActor, FGameplayAbilitySpecHandle AbilitySpecHandle)
{
	USigilAsyncTask_WaitGameplayAbilityEnded* MyObj = NewObject<USigilAsyncTask_WaitGameplayAbilityEnded>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->SetAbilitySystemComponent(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	MyObj->AbilitySpecHandle = AbilitySpecHandle;
	return MyObj;
}

void USigilAsyncTask_WaitGameplayAbilityEnded::HandleAbilityEnded(const FAbilityEndedData& Data)
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

void USigilAsyncTask_WaitGameplayAbilityEnded::Activate()
{
	Super::Activate();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		DelegateHandle = ASC->OnAbilityEnded.AddUObject(this, &USigilAsyncTask_WaitGameplayAbilityEnded::HandleAbilityEnded);
	}
	else
	{
		EndAction();
	}
}

void USigilAsyncTask_WaitGameplayAbilityEnded::EndAction()
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
