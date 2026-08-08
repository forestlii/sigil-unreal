// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AbilityTasks/SigilAbilityTask_WaitTargetDataUsingActor.h"

#include "AbilitySystemComponent.h"
#include "TargetActors/SigilAbilityTargetActor_Trace.h"

USigilAbilityTask_WaitTargetDataUsingActor* USigilAbilityTask_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor(
	UGameplayAbility* OwningAbility, FName TaskInstanceName,
	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor,
	bool bCreateKeyIfNotValidForMorePrediction)
{
	USigilAbilityTask_WaitTargetDataUsingActor* MyObj = NewAbilityTask<USigilAbilityTask_WaitTargetDataUsingActor>(
		OwningAbility, TaskInstanceName); //Register for task list here, providing a given FName as a key
	MyObj->TargetActor = InTargetActor;
	MyObj->ConfirmationType = ConfirmationType;
	MyObj->bCreateKeyIfNotValidForMorePrediction = bCreateKeyIfNotValidForMorePrediction;
	return MyObj;
}

void USigilAbilityTask_WaitTargetDataUsingActor::Activate()
{
	if (!IsValid(this))
	{
		return;
	}

	if (Ability && TargetActor)
	{
		/** server&client 注册TargetActor上的Ready(Confirm)/Cancel事件 */
		InitializeTargetActor();
		/** server注册TargetDeta */
		RegisterTargetDataCallbacks();

		FinalizeTargetActor();
	}
	else
	{
		EndTask();
	}
}

void USigilAbilityTask_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data,
                                                                   FGameplayTag ActivationTag)
{
	FGameplayAbilityTargetDataHandle MutableData = Data;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	/**
	*  Call into the TargetActor to sanitize/verify the data. If this returns false, we are rejecting
	*	the replicated target data and will treat this as a cancel.
	*
	*	This can also be used for bandwidth optimizations. OnReplicatedTargetDataReceived could do an actual
	*	trace/check/whatever server side and use that data. So rather than having the client send that data
	*	explicitly, the client is basically just sending a 'confirm' and the server is now going to do the work
	*	in OnReplicatedTargetDataReceived.
	*/
	if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			Cancelled.Broadcast(MutableData);
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			ValidData.Broadcast(MutableData);
		}
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

void USigilAbilityTask_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	EndTask();
}

void USigilAbilityTask_WaitTargetDataUsingActor::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data)
{

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();

	if (!Ability || !ASC)
	{
		return;
	}

	// client path
	FScopedPredictionWindow ScopedPrediction(ASC,
	                                         ShouldReplicateDataToServer() && (bCreateKeyIfNotValidForMorePrediction &&
		                                         !ASC->ScopedPredictionKey.IsValidForMorePrediction()
	                                         ));

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();

	// client path
	if (IsPredictingClient())
	{
		// Rpc发送TargetData到服务器
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			FGameplayTag ApplicationTag; // Fixme: where would this be useful?
			ASC->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(),
			                                                          GetActivationPredictionKey(), Data,
			                                                          ApplicationTag,
			                                                          AbilitySystemComponent->ScopedPredictionKey);
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// Rpc告诉服务器确认了。
			// We aren't going to send the target data, but we will send a generic confirmed message.
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm,
			                                                 GetAbilitySpecHandle(), GetActivationPredictionKey(),
			                                                 AbilitySystemComponent->ScopedPredictionKey);
		}
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(Data);
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

void USigilAbilityTask_WaitTargetDataUsingActor::OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();

	if(!ASC)
	{
		return;
	}

	//client path
	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	//client path
	if (IsPredictingClient())
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			ASC->ServerSetReplicatedTargetDataCancelled(
				GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
		else
		{
			// We aren't going to send the target data, but we will send a generic confirmed message.
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel,
			                                                 GetAbilitySpecHandle(), GetActivationPredictionKey(),
			                                                 ASC->ScopedPredictionKey);
		}
	}

	// client&& server path.
	Cancelled.Broadcast(Data);
	EndTask();
}

void USigilAbilityTask_WaitTargetDataUsingActor::ExternalConfirm(bool bEndTask)
{
	if (TargetActor)
	{
		if (TargetActor->ShouldProduceTargetData())
		{
			TargetActor->ConfirmTargetingAndContinue();
		}
	}
	Super::ExternalConfirm(bEndTask);
}

void USigilAbilityTask_WaitTargetDataUsingActor::ExternalCancel()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	Super::ExternalCancel();
}

void USigilAbilityTask_WaitTargetDataUsingActor::InitializeTargetActor() const
{
	check(TargetActor);
	check(Ability);

	TargetActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

	TargetActor->TargetDataReadyDelegate.AddUObject(
		const_cast<USigilAbilityTask_WaitTargetDataUsingActor*>(this), &USigilAbilityTask_WaitTargetDataUsingActor::OnTargetDataReadyCallback);
	TargetActor->CanceledDelegate.AddUObject(
		const_cast<USigilAbilityTask_WaitTargetDataUsingActor*>(this), &USigilAbilityTask_WaitTargetDataUsingActor::OnTargetDataCancelledCallback);
}

void USigilAbilityTask_WaitTargetDataUsingActor::RegisterTargetDataCallbacks()
{
	if (!ensure(IsValid(this) == true))
	{
		return;
	}

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();

	if (!ASC)
	{
		return;
	}
	
	check(Ability);

	const bool bIsLocalControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	const bool bShouldProduceTargetDataOnServer = TargetActor->ShouldProduceTargetDataOnServer;

	/** server path.  若不是本地控制的(server for remote client),查看TargetData是否已发送，否则在到达此处时注册回调     */
	if (!bIsLocalControlled)
	{
		//如果我们希望客户端发送TargetData回调，就注册TargetData回调
		if (!bShouldProduceTargetDataOnServer) // produce on client
		{
			FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
			FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

			/** 注册TargetDataSet事件 */
			ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
				this, &USigilAbilityTask_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback);

			/** 注册TargetDataCancel事件*/
			ASC->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
				this, &USigilAbilityTask_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback);

			// 检查TargetData是否已经Confirm/Cancel并执行相关操作。
			ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

			SetWaitingOnRemotePlayerData();
		}
	}
}

void USigilAbilityTask_WaitTargetDataUsingActor::FinalizeTargetActor() const
{
	check(TargetActor);
	check(Ability);

	TargetActor->StartTargeting(Ability);

	if (TargetActor->ShouldProduceTargetData())
	{
		// If instant confirm, then stop targeting immediately.
		// Note this is kind of bad: we should be able to just call a static func on the CDO to do this. 
		// But then we wouldn't get to set ExposeOnSpawnParameters.
		if (ConfirmationType == EGameplayTargetingConfirmation::Instant)
		{
			TargetActor->ConfirmTargeting();
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// Bind to the Cancel/Confirm Delegates (called from local confirm or from repped confirm)
			TargetActor->BindToConfirmCancelInputs();
		}
	}
}


void USigilAbilityTask_WaitTargetDataUsingActor::OnDestroy(bool AbilityEnded)
{
	if (TargetActor)
	{
		ASigilAbilityTargetActor_Trace* TraceTargetActor = Cast<ASigilAbilityTargetActor_Trace>(TargetActor);
		if (TraceTargetActor)
		{
			// TargetActor 基类没有StopTracing函数.
			TraceTargetActor->StopTargeting();
		}
		else
		{
			// TargetActor doesn't have a StopTargeting function
			TargetActor->SetActorTickEnabled(false);

			// Clear added callbacks
			TargetActor->TargetDataReadyDelegate.RemoveAll(this);
			TargetActor->CanceledDelegate.RemoveAll(this);

			AbilitySystemComponent->GenericLocalConfirmCallbacks.RemoveDynamic(
				TargetActor, &AGameplayAbilityTargetActor::ConfirmTargeting);
			AbilitySystemComponent->GenericLocalCancelCallbacks.RemoveDynamic(
				TargetActor, &AGameplayAbilityTargetActor::CancelTargeting);
			TargetActor->GenericDelegateBoundASC = nullptr;
		}
	}

	Super::OnDestroy(AbilityEnded);
}

bool USigilAbilityTask_WaitTargetDataUsingActor::ShouldReplicateDataToServer() const
{
	if (!Ability || !TargetActor)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (!Info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
	{
		return true;
	}

	return false;
}
