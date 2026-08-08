// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/Behaviors/SigilGameplayBehavior_InteractionWithAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SigilInteractionLogChannels.h"
#include "Abilities/GameplayAbility.h"
#include "Interaction/Behaviors/SigilGameplayBehaviorConfig_InteractionWithAbility.h"
#include "Interaction/SigilInteractionSystemComponent.h"

bool USigilGameplayBehavior_InteractionWithAbility::Trigger(AActor& InAvatar, const UGameplayBehaviorConfig* Config, AActor* SmartObjectOwner)
{
	bTransientIsTriggering = true;
	bTransientIsActive = false;
	TransientAvatar = &InAvatar;
	TransientSmartObjectOwner = SmartObjectOwner;

	USigilInteractionSystemComponent* InteractionSystem = InAvatar.FindComponentByClass<USigilInteractionSystemComponent>();

	if (!InteractionSystem)
	{
		SIGIL_INTERACTION_CLOG(Error, "Missing InteractionSystem Component!")
		return false;
	}

	UAbilitySystemComponent* Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(&InAvatar);
	if (!Asc)
	{
		SIGIL_INTERACTION_CLOG(Error, "Missing Ability System Component!")
		return false;
	}

	TSubclassOf<UGameplayAbility> AbilityClass{nullptr};
	int32 AbilityLevel = 0;
	if (!CheckValidAbilitySetting(Config, AbilityClass, AbilityLevel))
	{
		return false;
	}

	if (FGameplayAbilitySpec* Handle = Asc->FindAbilitySpecFromClass(AbilityClass))
	{
		SIGIL_INTERACTION_CLOG(Error, "Try granting repeated interaction ability of class:%s, which is not supported!", *AbilityClass->GetName())
		return false;
	}

	GrantedAbilityClass = AbilityClass;

	AbilityEndedDelegateHandle = Asc->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEndedCallback);

	//Ability trigger by event when activation polciy=ServerInitied won't work.
	AbilitySpecHandle = Asc->K2_GiveAbilityAndActivateOnce(AbilityClass, AbilityLevel);

	if (!AbilitySpecHandle.IsValid())
	{
		SIGIL_INTERACTION_CLOG(Error, "Can't active ability of class:%s! Check ability settings!", *AbilityClass->GetName())
		return false;
	}

	// Special case: behavior already interrupted
	if (bBehaviorWasInterrupted && AbilitySpecHandle.IsValid() && !bAbilityEnded)
	{
		Asc->ClearAbility(AbilitySpecHandle);
		return false;
	}

	if (AbilitySpecHandle.IsValid() && bAbilityEnded)
	{
		SIGIL_INTERACTION_CLOG(Verbose, "Instantly executed interaction ability:%s,handle%s", *AbilityClass->GetName(), *AbilitySpecHandle.ToString())
		EndBehavior(InAvatar, false);
		return false;
	}

	SIGIL_INTERACTION_CLOG(Verbose, "Granted and activate interaction ability:%s,handle%s", *AbilityClass->GetName(), *AbilitySpecHandle.ToString())

	//TODO what happens when avatar or target get destoryied?
	// SOOwner销毁的时候, 需要Abort当前行为, 目的是清除赋予的Ability
	SIGIL_INTERACTION_CLOG(Verbose, "Interaction begins with ability:%s", *AbilityClass->GetName())

	bTransientIsTriggering = false;
	bTransientIsActive = true;
	return bTransientIsActive;
}

void USigilGameplayBehavior_InteractionWithAbility::EndBehavior(AActor& Avatar, const bool bInterrupted)
{
	SIGIL_INTERACTION_CLOG(Verbose, "Interaction behavior ended %s", bInterrupted?TEXT("due to interruption!"):TEXT("normally!"))

	// clear ability stuff.
	if (UAbilitySystemComponent* Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(&Avatar))
	{
		if (AbilityEndedDelegateHandle.IsValid())
		{
			Asc->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
			AbilityEndedDelegateHandle.Reset();
		}

		// Special case: behavior interrupting active ability, so cancel ability.
		if (bInterrupted && bTransientIsActive && !bAbilityEnded && AbilitySpecHandle.IsValid())
		{
			if (const FGameplayAbilitySpec* Spec = Asc->FindAbilitySpecFromHandle(AbilitySpecHandle))
			{
				SIGIL_INTERACTION_CLOG(Verbose, "Cancel ability(%s) because behavior was interrupted.", *Spec->Ability.GetClass()->GetName())
				Asc->CancelAbilityHandle(AbilitySpecHandle);
			}
		}

		if (bInterrupted && !bTransientIsActive && AbilitySpecHandle.IsValid())
		{
			Asc->ClearAbility(AbilitySpecHandle);
		}
	}

	Super::EndBehavior(Avatar, bInterrupted);

	bBehaviorWasInterrupted = bInterrupted;
}

bool USigilGameplayBehavior_InteractionWithAbility::CheckValidAbilitySetting(const UGameplayBehaviorConfig* Config, TSubclassOf<UGameplayAbility>& OutAbilityClass, int32& OutAbilityLevel)
{
	// Ability class validation.
	const USigilGameplayBehaviorConfig_InteractionWithAbility* InteractionConfig = Cast<const USigilGameplayBehaviorConfig_InteractionWithAbility>(Config);
	if (!InteractionConfig)
	{
		SIGIL_INTERACTION_CLOG(Error, "Invalid GameplayBehaviorConfig! expect Config be type of %s.", *USigilGameplayBehaviorConfig_InteractionWithAbility::StaticClass()->GetName())
		return false;
	}

	const TSubclassOf<UGameplayAbility> AbilityClass = InteractionConfig->AbilityToGrant.LoadSynchronous();
	if (!AbilityClass)
	{
		SIGIL_INTERACTION_CLOG(Error, "Invalid AbilityToGrant Class!")
		return false;
	}
	OutAbilityClass = AbilityClass;
	OutAbilityLevel = InteractionConfig->AbilityLevel;
	return true;
}

void USigilGameplayBehavior_InteractionWithAbility::OnAbilityEndedCallback(const FAbilityEndedData& EndedData)
{
	if (bAbilityEnded)
	{
		return;
	}
	// check for ability granted by this behavior.
	if (EndedData.AbilitySpecHandle == AbilitySpecHandle || EndedData.AbilityThatEnded->GetClass() == GrantedAbilityClass)
	{
		bAbilityEnded = true;
		bAbilityWasCancelled = EndedData.bWasCancelled;

		// Special case: behavior already active and abilities ended, ending behavior normally.
		if (!bTransientIsTriggering && bTransientIsActive)
		{
			SIGIL_INTERACTION_CLOG(Verbose, "Interaction ability(%s) %s.", *EndedData.AbilityThatEnded.GetClass()->GetName(),
			         EndedData.bWasCancelled?TEXT("was canceled"):TEXT("ended normally"))
			EndBehavior(*GetAvatar(), false);
		}
	}
}
