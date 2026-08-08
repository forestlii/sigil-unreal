// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utilities/GGA_AbilitySystemFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemLog.h"
#include "GameplayCueManager.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimMontage.h"

bool UGGA_AbilitySystemFunctionLibrary::FindAbilitySystemComponent(AActor* Actor, TSubclassOf<UAbilitySystemComponent> DesiredClass, UAbilitySystemComponent*& ASC)
{
	if (UAbilitySystemComponent* Instance = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		if (Instance->GetClass()->IsChildOf(DesiredClass))
		{
			ASC = Instance;
			return true;
		}
	}
	return false;
}

UAbilitySystemComponent* UGGA_AbilitySystemFunctionLibrary::GetAbilitySystemComponent(AActor* Actor, TSubclassOf<UAbilitySystemComponent> DesiredClass)
{
	if (UAbilitySystemComponent* Instance = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		if (Instance->GetClass()->IsChildOf(DesiredClass))
		{
			return Instance;
		}
	}
	return nullptr;
}

void UGGA_AbilitySystemFunctionLibrary::InitAbilityActorInfo(UAbilitySystemComponent* AbilitySystem, AActor* InOwnerActor, AActor* InAvatarActor)
{
	if (!IsValid(AbilitySystem) || !IsValid(InOwnerActor) || !IsValid(InAvatarActor))
	{
		return;
	}
	AbilitySystem->InitAbilityActorInfo(InOwnerActor, InAvatarActor);
}

AActor* UGGA_AbilitySystemFunctionLibrary::GetOwnerActor(UAbilitySystemComponent* AbilitySystem)
{
	if (!IsValid(AbilitySystem))
	{
		return nullptr;
	}
	return AbilitySystem->GetOwnerActor();
}

AActor* UGGA_AbilitySystemFunctionLibrary::GetAvatarActor(UAbilitySystemComponent* AbilitySystem)
{
	if (!IsValid(AbilitySystem))
	{
		return nullptr;
	}
	return AbilitySystem->GetAvatarActor_Direct();
}

int32 UGGA_AbilitySystemFunctionLibrary::HandleGameplayEvent(UAbilitySystemComponent* AbilitySystem, FGameplayTag EventTag, const FGameplayEventData& Payload)
{
	if (!IsValid(AbilitySystem) || !EventTag.IsValid())
	{
		return false;
	}

	return AbilitySystem->HandleGameplayEvent(EventTag, &Payload);
}

bool UGGA_AbilitySystemFunctionLibrary::TryActivateAbilities(UAbilitySystemComponent* AbilitySystem, TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate, bool bAllowRemoteActivation,
                                                             bool bFirstOnly)
{
	if (!IsValid(AbilitySystem) || AbilitiesToActivate.IsEmpty())
	{
		return false;
	}
	int32 Num = 0;
	for (int32 i = 0; i < AbilitiesToActivate.Num(); i++)
	{
		if (AbilitySystem->TryActivateAbility(AbilitiesToActivate[i], bAllowRemoteActivation))
		{
			Num++;
			if (bFirstOnly)
			{
				return true;
			}
		}
	}
	return Num == AbilitiesToActivate.Num();
}

void UGGA_AbilitySystemFunctionLibrary::GetActivatableGameplayAbilitySpecsByAllMatchingTags(const UAbilitySystemComponent* AbilitySystem, const FGameplayTagContainer& Tags,
                                                                                            TArray<FGameplayAbilitySpecHandle>& MatchingGameplayAbilities,
                                                                                            bool bOnlyAbilitiesThatSatisfyTagRequirements)
{
	if (!IsValid(AbilitySystem) || Tags.IsEmpty())
	{
		return;
	}
	MatchingGameplayAbilities.Empty();
	TArray<FGameplayAbilitySpec*> AbilitiesToActivate;

	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(Tags, AbilitiesToActivate, bOnlyAbilitiesThatSatisfyTagRequirements);

	for (FGameplayAbilitySpec* GameplayAbilitySpec : AbilitiesToActivate)
	{
		MatchingGameplayAbilities.Add(GameplayAbilitySpec->Handle);
	}
}

bool UGGA_AbilitySystemFunctionLibrary::GetFirstActivatableAbilityByAllMatchingTags(const UAbilitySystemComponent* AbilitySystem, FGameplayTagContainer& Tags,
                                                                                    FGameplayAbilitySpecHandle& MatchingGameplayAbility, bool bOnlyAbilitiesThatSatisfyTagRequirements)
{
	if (!IsValid(AbilitySystem) || Tags.IsEmpty())
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : AbilitySystem->GetActivatableAbilities())
	{
#if ENGINE_MINOR_VERSION > 4
		if (Spec.Ability && Spec.Ability->GetAssetTags().HasAll(Tags))
#else
		if (Spec.Ability && Spec.Ability->AbilityTags.HasAll(Tags))
#endif
		{
			// Consider abilities that are blocked by tags currently if we're supposed to (default behavior).  
			// That way, we can use the blocking to find an appropriate ability based on tags when we have more than 
			// one ability that match the GameplayTagContainer.
			if (!bOnlyAbilitiesThatSatisfyTagRequirements || Spec.Ability->DoesAbilitySatisfyTagRequirements(*AbilitySystem))
			{
				MatchingGameplayAbility = Spec.Handle;
				return true;
			}
		}
	}
	return false;
}

void UGGA_AbilitySystemFunctionLibrary::GetActiveAbilityInstancesWithTags(const UAbilitySystemComponent* AbilitySystem, const FGameplayTagContainer& Tags,
                                                                          TArray<UGameplayAbility*>& MatchingAbilityInstances)
{
	if (!IsValid(AbilitySystem) || Tags.IsEmpty())
	{
		return;
	}

	TArray<FGameplayAbilitySpec*> AbilitiesToActivate;
	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(Tags, AbilitiesToActivate, false);

	// Iterate the list of all ability specs
	for (FGameplayAbilitySpec* Spec : AbilitiesToActivate)
	{
		// Iterate all instances on this ability spec
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			if (ActiveAbility->IsActive())
			{
				MatchingAbilityInstances.Add(ActiveAbility);
			}
		}
	}
}

UGameplayAbility* UGGA_AbilitySystemFunctionLibrary::GetAnimatingAbility(UAbilitySystemComponent* AbilitySystem)
{
	if (!IsValid(AbilitySystem))
	{
		return nullptr;
	}
	return AbilitySystem->GetAnimatingAbility();
}

bool UGGA_AbilitySystemFunctionLibrary::IsAnimatingAbility(UAbilitySystemComponent* AbilitySystem, UGameplayAbility* Ability)
{
	if (!IsValid(AbilitySystem))
	{
		return false;
	}
	return AbilitySystem->IsAnimatingAbility(Ability);
}

UGameplayAbility* UGGA_AbilitySystemFunctionLibrary::GetAnimatingAbilityFromActor(AActor* Actor, UAnimSequenceBase* Animation)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		if (ASC->GetCurrentMontage() == Animation)
		{
			if (UGameplayAbility* Ability = ASC->GetAnimatingAbility())
			{
				return Ability;
			}
		}
	}
	return nullptr;
}

bool UGGA_AbilitySystemFunctionLibrary::FindAnimatingAbilityFromActor(AActor* Actor, UAnimSequenceBase* Animation, TSubclassOf<UGameplayAbility> DesiredClass, UGameplayAbility*& AbilityInstance)
{
	UGameplayAbility* Ability = GetAnimatingAbilityFromActor(Actor, Animation);
	if (UClass* ActualClass = DesiredClass)
	{
		if (Ability && Ability->GetClass()->IsChildOf(ActualClass))
		{
			AbilityInstance = Ability;
			return true;
		}
	}
	return false;
}

bool UGGA_AbilitySystemFunctionLibrary::SendGameplayEventToActor(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	if (IsValid(Actor))
	{
		UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
		if (IsValid(AbilitySystemComponent))
		{
			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			if (AbilitySystemComponent->HandleGameplayEvent(EventTag, &Payload) > 0)
			{
				return true;
			}
		}
		else
		{
			ABILITY_LOG(Error, TEXT("UAbilitySystemBPLibrary::SendGameplayEventToActor: Invalid ability system component retrieved from Actor %s. EventTag was %s"), *Actor->GetName(),
			            *EventTag.ToString());
		}
	}
	return false;
}

void UGGA_AbilitySystemFunctionLibrary::BreakAbilityEndedData(const FAbilityEndedData& AbilityEndedData, UGameplayAbility*& AbilityThatEnded, FGameplayAbilitySpecHandle& AbilitySpecHandle,
                                                              bool& bReplicateEndAbility, bool& bWasCancelled)
{
	AbilityThatEnded = AbilityEndedData.AbilityThatEnded;
	AbilitySpecHandle = AbilityEndedData.AbilitySpecHandle;
	bReplicateEndAbility = AbilityEndedData.bReplicateEndAbility;
	bWasCancelled = AbilityEndedData.bWasCancelled;
}

bool UGGA_AbilitySystemFunctionLibrary::FindAllAbilitiesWithTagsInOrder(const UAbilitySystemComponent* AbilitySystem, TArray<FGameplayTag> Tags, TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles,
                                                                        bool bExactMatch)
{
	if (!IsValid(AbilitySystem) || Tags.IsEmpty())
	{
		return false;
	}
	// ensure the output array is empty
	OutAbilityHandles.Empty();

	for (const FGameplayTag& Tag : Tags)
	{
		TArray<FGameplayAbilitySpecHandle> Handles;
		AbilitySystem->FindAllAbilitiesWithTags(Handles, Tag.GetSingleTagContainer(), bExactMatch);
		OutAbilityHandles.Append(Handles);
	}

	return !OutAbilityHandles.IsEmpty();
}

bool UGGA_AbilitySystemFunctionLibrary::FindAbilityMatchingQuery(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle& OutAbilityHandle, FGameplayTagQuery Query)
{
	if (!IsValid(AbilitySystem) || Query.IsEmpty())
	{
		return false;
	}

	// iterate through all Ability Specs
	for (const FGameplayAbilitySpec& CurrentSpec : AbilitySystem->GetActivatableAbilities())
	{
		// try to get the ability instance
		UGameplayAbility* AbilityInstance = CurrentSpec.GetPrimaryInstance();

		// default to the CDO if we can't
		if (!AbilityInstance)
		{
			AbilityInstance = CurrentSpec.Ability;
		}

		// ensure the ability instance is valid
		if (IsValid(AbilityInstance))
		{
#if ENGINE_MINOR_VERSION > 4
			if (AbilityInstance->GetAssetTags().MatchesQuery(Query))
#else
			if (AbilityInstance->AbilityTags.MatchesQuery(Query))
#endif
			{
				OutAbilityHandle = CurrentSpec.Handle;
				return true;
			}
		}
	}
	return false;
}

bool UGGA_AbilitySystemFunctionLibrary::FindAbilityWithTags(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle& OutAbilityHandle, FGameplayTagContainer Tags, bool bExactMatch)
{
	if (!IsValid(AbilitySystem) || Tags.IsEmpty())
	{
		return false;
	}
	// iterate through all Ability Specs
	for (const FGameplayAbilitySpec& CurrentSpec : AbilitySystem->GetActivatableAbilities())
	{
		// try to get the ability instance
		UGameplayAbility* AbilityInstance = CurrentSpec.GetPrimaryInstance();

		// default to the CDO if we can't
		if (!AbilityInstance)
		{
			AbilityInstance = CurrentSpec.Ability;
		}

		// ensure the ability instance is valid
		if (IsValid(AbilityInstance))
		{
			// do we want an exact match?
			if (bExactMatch)
			{
				// check if we match all tags
#if ENGINE_MINOR_VERSION > 4
				if (AbilityInstance->GetAssetTags().HasAll(Tags))
#else
				if (AbilityInstance->AbilityTags.HasAll(Tags))
#endif
				{
					OutAbilityHandle = CurrentSpec.Handle;
					return true;
				}
			}
			else
			{
				// check if we match any tags
#if ENGINE_MINOR_VERSION > 4
				if (AbilityInstance->GetAssetTags().HasAny(Tags))
#else
				if (AbilityInstance->AbilityTags.HasAny(Tags))
#endif
				{
					OutAbilityHandle = CurrentSpec.Handle;
					return true;
				}
			}
		}
	}
	return false;
}

void UGGA_AbilitySystemFunctionLibrary::AddLooseGameplayTag(UAbilitySystemComponent* AbilitySystem, const FGameplayTag& GameplayTag, int32 Count)
{
	if (!IsValid(AbilitySystem) || !GameplayTag.IsValid())
	{
		return;
	}
	AbilitySystem->AddLooseGameplayTag(GameplayTag, Count);
}

void UGGA_AbilitySystemFunctionLibrary::AddLooseGameplayTags(UAbilitySystemComponent* AbilitySystem, const FGameplayTagContainer& GameplayTags, int32 Count)
{
	if (!IsValid(AbilitySystem) || GameplayTags.IsEmpty())
	{
		return;
	}
	AbilitySystem->AddLooseGameplayTags(GameplayTags, Count);
}

void UGGA_AbilitySystemFunctionLibrary::RemoveLooseGameplayTag(UAbilitySystemComponent* AbilitySystem, const FGameplayTag& GameplayTag, int32 Count)
{
	if (!IsValid(AbilitySystem) || !GameplayTag.IsValid())
	{
		return;
	}
	AbilitySystem->RemoveLooseGameplayTag(GameplayTag, Count);
}

void UGGA_AbilitySystemFunctionLibrary::RemoveLooseGameplayTags(UAbilitySystemComponent* AbilitySystem, const FGameplayTagContainer& GameplayTags, int32 Count)
{
	if (!IsValid(AbilitySystem) || GameplayTags.IsEmpty())
	{
		return;
	}
	AbilitySystem->RemoveLooseGameplayTags(GameplayTags, Count);
}

void UGGA_AbilitySystemFunctionLibrary::RemoveAllGameplayCues(UAbilitySystemComponent* AbilitySystem)
{
	if (!IsValid(AbilitySystem))
	{
		return;
	}
	AbilitySystem->RemoveAllGameplayCues();
}

void UGGA_AbilitySystemFunctionLibrary::SetAbilityInputPressed(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return;
	}

	FScopedAbilityListLock ActiveScopeLock(*AbilitySystem);
	if (FGameplayAbilitySpec* Spec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		if (Spec->Ability)
		{
			Spec->InputPressed = true;
			if (Spec->IsActive())
			{
				if (Spec->Ability->bReplicateInputDirectly && AbilitySystem->IsOwnerActorAuthoritative() == false)
				{
					AbilitySystem->ServerSetInputPressed(Spec->Handle);
				}

				AbilitySystem->AbilitySpecInputPressed(*Spec);

				PRAGMA_DISABLE_DEPRECATION_WARNINGS
#if ENGINE_MINOR_VERSION > 4
				// Fixing this up to use the instance activation, but this function should be deprecated as it cannot work with InstancedPerExecution
				UE_CLOG(Spec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution, LogAbilitySystem, Warning,
				        TEXT("%hs: %s is InstancedPerExecution. This is unreliable for Input as you may only interact with the latest spawned Instance"), __func__, *GetNameSafe(Spec->Ability));
				TArray<UGameplayAbility*> Instances = Spec->GetAbilityInstances();
				const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? Spec->ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
				// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
				AbilitySystem->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec->Handle, ActivationInfo.GetActivationPredictionKey());
#else
				// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
				AbilitySystem->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec->Handle, Spec->ActivationInfo.GetActivationPredictionKey());
#endif
				PRAGMA_ENABLE_DEPRECATION_WARNINGS
			}
			else
			{
				// Ability is not active, so try to activate it
				AbilitySystem->TryActivateAbility(Spec->Handle);
			}
		}
	}
}

void UGGA_AbilitySystemFunctionLibrary::SetAbilityInputReleased(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return;
	}

	FScopedAbilityListLock ActiveScopeLock(*AbilitySystem);

	if (FGameplayAbilitySpec* Spec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		Spec->InputPressed = false;
		if (Spec->Ability && Spec->IsActive())
		{
			if (Spec->Ability->bReplicateInputDirectly && AbilitySystem->IsOwnerActorAuthoritative() == false)
			{
				AbilitySystem->ServerSetInputReleased(Spec->Handle);
			}

			AbilitySystem->AbilitySpecInputReleased(*Spec);


			PRAGMA_DISABLE_DEPRECATION_WARNINGS
#if ENGINE_MINOR_VERSION > 4
			// Fixing this up to use the instance activation, but this function should be deprecated as it cannot work with InstancedPerExecution
			UE_CLOG(Spec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution, LogAbilitySystem, Warning,
			        TEXT("%hs: %s is InstancedPerExecution. This is unreliable for Input as you may only interact with the latest spawned Instance"), __func__, *GetNameSafe(Spec->Ability));
			TArray<UGameplayAbility*> Instances = Spec->GetAbilityInstances();
			const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? Spec->ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
			AbilitySystem->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec->Handle, ActivationInfo.GetActivationPredictionKey());
#else
			AbilitySystem->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec->Handle, Spec->ActivationInfo.GetActivationPredictionKey());
#endif
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
		}
	}
}

bool UGGA_AbilitySystemFunctionLibrary::CanActivateAbility(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle AbilityToActivate, FGameplayTagContainer& RelevantTags)
{
	if (!IsValid(AbilitySystem) || !AbilityToActivate.IsValid())
	{
		return false;
	}

	FGameplayTagContainer FailureTags;
	FGameplayAbilitySpec* Spec = AbilitySystem->FindAbilitySpecFromHandle(AbilityToActivate);
	if (!Spec)
	{
		ABILITY_LOG(Warning, TEXT("CanActivateAbility called with invalid Handle"));
		return false;
	}

	// Lock ability list so our Spec doesn't get destroyed while activating

	const FGameplayAbilityActorInfo* ActorInfo = AbilitySystem->AbilityActorInfo.Get();

	UGameplayAbility* AbilityCDO = Spec->Ability;

	if (!AbilityCDO)
	{
		ABILITY_LOG(Warning, TEXT("CanActivateAbility called with invalid AbilityCDO"));
		return false;
	}

	// If it's instance once the instanced ability will be set, otherwise it will be null
	UGameplayAbility* InstancedAbility = Spec->GetPrimaryInstance();

	// If we have an instanced ability, call CanActivateAbility on it.
	// Otherwise we always do a non instanced CanActivateAbility check using the CDO of the Ability.
	UGameplayAbility* const CanActivateAbilitySource = InstancedAbility ? InstancedAbility : AbilityCDO;

	return CanActivateAbilitySource->CanActivateAbility(AbilityToActivate, ActorInfo, nullptr, nullptr, &RelevantTags);
}

bool UGGA_AbilitySystemFunctionLibrary::SelectFirstCanActivateAbility(const UAbilitySystemComponent* AbilitySystem, TArray<FGameplayAbilitySpecHandle> Abilities,
                                                                      FGameplayAbilitySpecHandle& OutAbilityHandle)
{
	for (int32 i = 0; i < Abilities.Num(); i++)
	{
		static FGameplayTagContainer DummyTags;
		DummyTags.Reset();
		if (CanActivateAbility(AbilitySystem, Abilities[i], DummyTags))
		{
			OutAbilityHandle = Abilities[i];
			return true;
		}
	}
	return false;
}

void UGGA_AbilitySystemFunctionLibrary::CancelAbility(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return;
	}
	AbilitySystem->CancelAbilityHandle(Ability);
}

void UGGA_AbilitySystemFunctionLibrary::CancelAbilities(UAbilitySystemComponent* AbilitySystem, FGameplayTagContainer WithTags, FGameplayTagContainer WithoutTags)
{
	if (!IsValid(AbilitySystem))
	{
		return;
	}
	AbilitySystem->CancelAbilities(&WithTags, &WithoutTags);
}

bool UGGA_AbilitySystemFunctionLibrary::IsAbilityInstanceActive(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return false;
	}
	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		if (UGameplayAbility* AbilityInstance = AbilitySpec->GetPrimaryInstance())
		{
			return AbilityInstance->IsActive();
		}
	}
	return false;
}

bool UGGA_AbilitySystemFunctionLibrary::IsAbilityActive(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return false;
	}

	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		return AbilitySpec->IsActive();
	}

	return false;
}

TArray<UGameplayAbility*> UGGA_AbilitySystemFunctionLibrary::GetAbilityInstances(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	TArray<UGameplayAbility*> Abilities;
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return Abilities;
	}

	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		Abilities = AbilitySpec->GetAbilityInstances();
		return Abilities;
	}

	return Abilities;
}

const UGameplayAbility* UGGA_AbilitySystemFunctionLibrary::GetAbilityCDO(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return nullptr;
	}

	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		return AbilitySpec->Ability;
	}
	return nullptr;
}

int32 UGGA_AbilitySystemFunctionLibrary::GetAbilityLevel(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return 0;
	}

	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		return AbilitySpec->Level;
	}
	return 0;
}

int32 UGGA_AbilitySystemFunctionLibrary::GetAbilityInputId(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return -1;
	}

	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		return AbilitySpec->InputID;
	}
	return -1;
}

UObject* UGGA_AbilitySystemFunctionLibrary::GetAbilitySourceObject(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return nullptr;
	}

	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		if (AbilitySpec->SourceObject.IsValid())
		{
			return AbilitySpec->SourceObject.Get();
		}
	}
	return nullptr;
}

FGameplayTagContainer UGGA_AbilitySystemFunctionLibrary::GetAbilityDynamicTags(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return FGameplayTagContainer::EmptyContainer;
	}

	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
#if ENGINE_MINOR_VERSION > 4
		return AbilitySpec->GetDynamicSpecSourceTags();
#else
		return AbilitySpec->DynamicAbilityTags;
#endif
	}
	return FGameplayTagContainer::EmptyContainer;
}

UGameplayAbility* UGGA_AbilitySystemFunctionLibrary::GetAbilityPrimaryInstance(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability)
{
	if (!IsValid(AbilitySystem) || !Ability.IsValid())
	{
		return nullptr;
	}

	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromHandle(Ability))
	{
		return AbilitySpec->GetPrimaryInstance();
	}

	return nullptr;
}

UAttributeSet* UGGA_AbilitySystemFunctionLibrary::GetAttributeSetByClass(const UAbilitySystemComponent* AbilitySystem, const TSubclassOf<UAttributeSet> AttributeSetClass)
{
	if (!IsValid(AbilitySystem) || !AttributeSetClass)
	{
		return nullptr;
	}

	for (UAttributeSet* SpawnedAttribute : AbilitySystem->GetSpawnedAttributes())
	{
		if (SpawnedAttribute && SpawnedAttribute->IsA(AttributeSetClass))
		{
			return SpawnedAttribute;
		}
	}

	return nullptr;
}

float UGGA_AbilitySystemFunctionLibrary::GetValueAtLevel(const FScalableFloat& ScalableFloat, float Level, FString ContextString)
{
	return ScalableFloat.GetValueAtLevel(Level, &ContextString);
}
