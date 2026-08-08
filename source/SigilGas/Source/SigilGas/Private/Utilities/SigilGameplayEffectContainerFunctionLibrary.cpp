// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utilities/SigilGameplayEffectContainerFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "AbilitySystemGlobals.h"
#include "SigilGasLogChannels.h"
#include "TargetingSystem/TargetingSubsystem.h"

bool USigilGameplayEffectContainerFunctionLibrary::IsValidContainer(const FSigilGameplayEffectContainer& Container)
{
	return !Container.TargetGameplayEffectClasses.IsEmpty() && Container.TargetingPreset != nullptr;
}

bool USigilGameplayEffectContainerFunctionLibrary::HasValidEffects(const FSigilGameplayEffectContainerSpec& ContainerSpec)
{
	return ContainerSpec.HasValidEffects();
}

bool USigilGameplayEffectContainerFunctionLibrary::HasValidTargets(const FSigilGameplayEffectContainerSpec& ContainerSpec)
{
	return ContainerSpec.HasValidTargets();
}

FSigilGameplayEffectContainerSpec USigilGameplayEffectContainerFunctionLibrary::AddTargets(const FSigilGameplayEffectContainerSpec& ContainerSpec, const TArray<FHitResult>& HitResults,
                                                                                         const TArray<AActor*>& TargetActors)
{
	FSigilGameplayEffectContainerSpec NewSpec = ContainerSpec;
	NewSpec.AddTargets(HitResults, TargetActors);
	return NewSpec;
}

FSigilGameplayEffectContainerSpec USigilGameplayEffectContainerFunctionLibrary::MakeEffectContainerSpec(const FSigilGameplayEffectContainer& Container,
                                                                                                      const FGameplayEventData& EventData, int32 OverrideGameplayLevel, UGameplayAbility* SourceAbility)
{
	FSigilGameplayEffectContainerSpec ReturnSpec;

	if (SourceAbility== nullptr && EventData.Instigator == nullptr)
	{
		UE_LOG(LogSigilAbilitySystem, Warning, TEXT("Missing instigator within EventData, It's required to look for Source AbilitySystemComponent!"))
		return ReturnSpec;
	}
	// First figure out our actor info

	UAbilitySystemComponent* SourceASC = SourceAbility?SourceAbility->GetAbilitySystemComponentFromActorInfo():UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EventData.Instigator);

	if (SourceASC && SourceASC->GetAvatarActor())
	{
		if (Container.TargetingPreset.Get())
		{
			if (UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(SourceASC->GetWorld()))
			{
				FTargetingSourceContext SourceContext;
				SourceContext.SourceActor = SourceASC->GetAvatarActor();

				FTargetingRequestHandle TargetingHandle = UTargetingSubsystem::MakeTargetRequestHandle(Container.TargetingPreset, SourceContext);

				FTargetingRequestDelegate Delegate = FTargetingRequestDelegate::CreateLambda([&](FTargetingRequestHandle InTargetingHandle)
				{
					TArray<FHitResult> HitResults;
					TArray<AActor*> TargetActors;
					TargetingSubsystem->GetTargetingResults(InTargetingHandle, HitResults);
					ReturnSpec.AddTargets(HitResults, TargetActors);
				});

				UE_LOG(LogSigilAbilitySystem, VeryVerbose, TEXT("Starting immediate targeting task for EffectContainer."));

				FTargetingImmediateTaskData& ImmediateTaskData = FTargetingImmediateTaskData::FindOrAdd(TargetingHandle);
				ImmediateTaskData.bReleaseOnCompletion = true;

				TargetingSubsystem->ExecuteTargetingRequestWithHandle(TargetingHandle, Delegate);
			}
		}

		int32 Level = OverrideGameplayLevel >= 0 ? OverrideGameplayLevel : EventData.EventMagnitude;

		// Build GameplayEffectSpecs for each applied effect
		for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
		{
			FGameplayEffectSpecHandle SpecHandle = SourceAbility?SourceAbility->MakeOutgoingGameplayEffectSpec(EffectClass, Level):SourceASC->MakeOutgoingSpec(EffectClass, Level, SourceASC->MakeEffectContext());
			FGameplayEffectContextHandle ContextHandle = UAbilitySystemBlueprintLibrary::GetEffectContext(SpecHandle);
			
			if (SourceAbility==nullptr && EventData.OptionalObject)
			{
				ContextHandle.AddSourceObject(EventData.OptionalObject);
			}
			
			ReturnSpec.TargetGameplayEffectSpecs.Add(SpecHandle);
		}
		
		if (EventData.TargetData.Num() > 0)
		{
			ReturnSpec.TargetData.Append(EventData.TargetData);
		}
	}
	return ReturnSpec;
}

TArray<FActiveGameplayEffectHandle> USigilGameplayEffectContainerFunctionLibrary::ApplyEffectContainerSpec(UGameplayAbility* ExecutingAbility, const FSigilGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AllEffects;

	if (!IsValid(ExecutingAbility) || !ExecutingAbility->IsInstantiated())
	{
		UE_LOG(LogSigilAbility, Error, TEXT("Requires \"Executing ability\" to apply effect container spec."))
		return AllEffects;
	}

	const FGameplayAbilityActorInfo* ActorInfo = ExecutingAbility->GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo& ActivationInfo = ExecutingAbility->GetCurrentActivationInfoRef();

	// Iterate list of effect specs and apply them to their target data
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		TArray<FActiveGameplayEffectHandle> EffectHandles;

		if (SpecHandle.IsValid() && ExecutingAbility->HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
		{
			FScopedTargetListLock ActiveScopeLock(*ActorInfo->AbilitySystemComponent, *ExecutingAbility);

			for (TSharedPtr<FGameplayAbilityTargetData> Data : ContainerSpec.TargetData.Data)
			{
				if (Data.IsValid())
				{
					AllEffects.Append(Data->ApplyGameplayEffectSpec(*SpecHandle.Data.Get(), ActorInfo->AbilitySystemComponent->GetPredictionKeyForNewAction()));
				}
				else
				{
					UE_LOG(LogSigilAbility, Warning, TEXT("ApplyGameplayEffectSpecToTarget invalid target data passed in. Ability: %s"), *ExecutingAbility->GetPathName());
				}
			}
		}
	}

	return AllEffects;
}

TArray<FActiveGameplayEffectHandle> USigilGameplayEffectContainerFunctionLibrary::ApplyExternalEffectContainerSpec(const FSigilGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AllEffects;

	// Iterate list of gameplay effects
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		if (SpecHandle.IsValid())
		{
			// If effect is valid, iterate list of targets and apply to all
			for (TSharedPtr<FGameplayAbilityTargetData> Data : ContainerSpec.TargetData.Data)
			{
				AllEffects.Append(Data->ApplyGameplayEffectSpec(*SpecHandle.Data.Get()));
			}
		}
	}
	return AllEffects;
}
