// Copyright 2020 Dan Kestranek. Licensed under the MIT License (GASDocumentation: https://github.com/tranek/GASDocumentation).
// Modifications Copyright (c) 2026 Likeon. All Rights Reserved.

#include "AsyncTasks/SigilAsyncTask_CooldownChanged.h"

USigilAsyncTask_CooldownChanged* USigilAsyncTask_CooldownChanged::ListenForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer InCooldownTags,
                                                                                         bool bInUseServerCooldown)
{
	if (!IsValid(AbilitySystemComponent) || InCooldownTags.Num() < 1)
	{
		return nullptr;
	}

	USigilAsyncTask_CooldownChanged* Task = NewObject<USigilAsyncTask_CooldownChanged>();
	Task->SetAbilitySystemComponent(AbilitySystemComponent);
	Task->CooldownTags = InCooldownTags;
	Task->bUseServerCooldown = bInUseServerCooldown;
	return Task;
}

void USigilAsyncTask_CooldownChanged::Activate()
{
	Super::Activate();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		EndAction();
		return;
	}

	ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ThisClass::OnActiveGameplayEffectAddedCallback);

	for (const FGameplayTag& CooldownTag : CooldownTags)
	{
		ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::CooldownTagChanged);
	}
}

void USigilAsyncTask_CooldownChanged::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);

		for (const FGameplayTag& CooldownTag : CooldownTags)
		{
			ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	Super::EndAction();
}

void USigilAsyncTask_CooldownChanged::OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	if (!ShouldBroadcastDelegates())
	{
		return;
	}

	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	for (const FGameplayTag& CooldownTag : CooldownTags)
	{
		if (!AssetTags.HasTagExact(CooldownTag) && !GrantedTags.HasTagExact(CooldownTag))
		{
			continue;
		}

		float TimeRemaining = 0.0f;
		float Duration = 0.0f;
		// Sigil change: query by the matched cooldown tag itself; GASDocumentation assumed the cooldown tag is always GrantedTags[0].
		GetCooldownRemainingForTag(FGameplayTagContainer(CooldownTag), TimeRemaining, Duration);

		const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
		const bool bIsAuthority = ASC && ASC->GetOwnerRole() == ROLE_Authority;
		const bool bIsPredictedEffect = SpecApplied.GetContext().GetAbilityInstance_NotReplicated() != nullptr;

		if (bIsAuthority)
		{
			// Player is the server (or standalone).
			OnCooldownBegin.Broadcast(CooldownTag, TimeRemaining, Duration);
		}
		else if (!bUseServerCooldown && bIsPredictedEffect)
		{
			// Client using the predicted cooldown.
			OnCooldownBegin.Broadcast(CooldownTag, TimeRemaining, Duration);
		}
		else if (bUseServerCooldown && !bIsPredictedEffect)
		{
			// Client using the server's cooldown. This is the server's corrective cooldown GE.
			OnCooldownBegin.Broadcast(CooldownTag, TimeRemaining, Duration);
		}
		else if (bUseServerCooldown && bIsPredictedEffect)
		{
			// Client using the server's cooldown but this is the predicted cooldown GE.
			// Useful to gray out abilities until the server's cooldown arrives.
			OnCooldownBegin.Broadcast(CooldownTag, -1.0f, -1.0f);
		}
	}
}

void USigilAsyncTask_CooldownChanged::CooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount)
{
	if (NewCount == 0 && ShouldBroadcastDelegates())
	{
		OnCooldownEnd.Broadcast(CooldownTag, -1.0f, -1.0f);
	}
}

bool USigilAsyncTask_CooldownChanged::GetCooldownRemainingForTag(const FGameplayTagContainer& InCooldownTags, float& TimeRemaining, float& CooldownDuration) const
{
	TimeRemaining = 0.f;
	CooldownDuration = 0.f;

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC || InCooldownTags.Num() < 1)
	{
		return false;
	}

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(InCooldownTags);
	const TArray<TPair<float, float>> DurationAndTimeRemaining = ASC->GetActiveEffectsTimeRemainingAndDuration(Query);
	if (DurationAndTimeRemaining.Num() < 1)
	{
		return false;
	}

	int32 BestIdx = 0;
	float LongestTime = DurationAndTimeRemaining[0].Key;
	for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
	{
		if (DurationAndTimeRemaining[Idx].Key > LongestTime)
		{
			LongestTime = DurationAndTimeRemaining[Idx].Key;
			BestIdx = Idx;
		}
	}

	TimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
	CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;
	return true;
}
