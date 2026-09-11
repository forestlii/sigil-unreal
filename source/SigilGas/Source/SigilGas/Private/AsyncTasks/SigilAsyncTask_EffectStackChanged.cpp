// Copyright 2020 Dan Kestranek. Licensed under the MIT License (GASDocumentation: https://github.com/tranek/GASDocumentation).
// Modifications Copyright (c) 2026 Likeon. All Rights Reserved.

#include "AsyncTasks/SigilAsyncTask_EffectStackChanged.h"

USigilAsyncTask_EffectStackChanged* USigilAsyncTask_EffectStackChanged::ListenForGameplayEffectStackChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag InEffectGameplayTag)
{
	if (!IsValid(AbilitySystemComponent) || !InEffectGameplayTag.IsValid())
	{
		return nullptr;
	}

	USigilAsyncTask_EffectStackChanged* Task = NewObject<USigilAsyncTask_EffectStackChanged>();
	Task->SetAbilitySystemComponent(AbilitySystemComponent);
	Task->EffectGameplayTag = InEffectGameplayTag;
	return Task;
}

void USigilAsyncTask_EffectStackChanged::Activate()
{
	Super::Activate();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		EndAction();
		return;
	}

	ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ThisClass::OnActiveGameplayEffectAddedCallback);
	ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ThisClass::OnRemoveGameplayEffectCallback);
}

void USigilAsyncTask_EffectStackChanged::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
		ASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);

		if (ActiveEffectHandle.IsValid())
		{
			// Sigil change: the delegate pointer is null once the effect is gone; GASDocumentation dereferenced it unconditionally.
			if (FOnActiveGameplayEffectStackChange* StackDelegate = ASC->OnGameplayEffectStackChangeDelegate(ActiveEffectHandle))
			{
				StackDelegate->RemoveAll(this);
			}
		}
	}

	Super::EndAction();
}

bool USigilAsyncTask_EffectStackChanged::SpecMatchesEffectTag(const FGameplayEffectSpec& Spec) const
{
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	Spec.GetAllGrantedTags(GrantedTags);

	return AssetTags.HasTagExact(EffectGameplayTag) || GrantedTags.HasTagExact(EffectGameplayTag);
}

void USigilAsyncTask_EffectStackChanged::OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	if (!ShouldBroadcastDelegates() || !SpecMatchesEffectTag(SpecApplied))
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (FOnActiveGameplayEffectStackChange* StackDelegate = ASC->OnGameplayEffectStackChangeDelegate(ActiveHandle))
		{
			StackDelegate->AddUObject(this, &ThisClass::GameplayEffectStackChanged);
		}
	}

	ActiveEffectHandle = ActiveHandle;
	OnGameplayEffectStackChange.Broadcast(EffectGameplayTag, ActiveHandle, 1, 0);
}

void USigilAsyncTask_EffectStackChanged::OnRemoveGameplayEffectCallback(const FActiveGameplayEffect& EffectRemoved)
{
	if (!ShouldBroadcastDelegates() || !SpecMatchesEffectTag(EffectRemoved.Spec))
	{
		return;
	}

	OnGameplayEffectStackChange.Broadcast(EffectGameplayTag, EffectRemoved.Handle, 0, EffectRemoved.Spec.GetStackCount());
}

void USigilAsyncTask_EffectStackChanged::GameplayEffectStackChanged(FActiveGameplayEffectHandle EffectHandle, int32 NewStackCount, int32 PreviousStackCount)
{
	if (ShouldBroadcastDelegates())
	{
		OnGameplayEffectStackChange.Broadcast(EffectGameplayTag, EffectHandle, NewStackCount, PreviousStackCount);
	}
}
