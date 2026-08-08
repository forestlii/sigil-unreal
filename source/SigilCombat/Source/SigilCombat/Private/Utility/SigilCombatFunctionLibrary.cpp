// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utility/SigilCombatFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "SigilCombatInterface.h"
#include "SigilCombatSystemSettings.h"
#include "SigilCombatTeamAgentInterface.h"
#include "SigilCombatLogChannels.h"
#include "AbilitySystem/SigilGameplayEffectContext.h"
#include "CombatFlow/SigilAttackRequest.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/SigilWeaponInterface.h"

TScriptInterface<ISigilCombatTeamAgentInterface> USigilCombatFunctionLibrary::GetCombatTeamAgentInterface(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (Actor->GetClass()->ImplementsInterface(USigilCombatTeamAgentInterface::StaticClass()))
		{
			return Actor;
		}
		TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(USigilCombatTeamAgentInterface::StaticClass());
		return Components.IsValidIndex(0) ? Components[0] : nullptr;
	}
	return nullptr;
}

bool USigilCombatFunctionLibrary::FindCombatTeamAgentInterface(AActor* Actor, TScriptInterface<ISigilCombatTeamAgentInterface>& OutInterface)
{
	OutInterface = GetCombatTeamAgentInterface(Actor);
	return OutInterface != nullptr;
}

TScriptInterface<ISigilCombatInterface> USigilCombatFunctionLibrary::GetCombatInterface(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (Actor->GetClass()->ImplementsInterface(USigilCombatInterface::StaticClass()))
		{
			return Actor;
		}
		else
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(USigilCombatInterface::StaticClass());
			return Components.IsValidIndex(0) ? Components[0] : nullptr;
		}
	}
	return nullptr;
}

UObject* USigilCombatFunctionLibrary::GetCombatInterfaceImplementer(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (Actor->GetClass()->ImplementsInterface(USigilCombatInterface::StaticClass()))
		{
			return Actor;
		}
		else
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(USigilCombatInterface::StaticClass());
			return Components.IsValidIndex(0) ? Components[0] : nullptr;
		}
	}
	return nullptr;
}

TScriptInterface<ISigilWeaponInterface> USigilCombatFunctionLibrary::GetWeaponInterface(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (Actor->GetClass()->ImplementsInterface(USigilWeaponInterface::StaticClass()))
		{
			return Actor;
		}
		else
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(USigilWeaponInterface::StaticClass());
			return Components.IsValidIndex(0) ? Components[0] : nullptr;
		}
	}
	return nullptr;
}

USkeletalMeshComponent* USigilCombatFunctionLibrary::GetMainCharacterMeshComponent(AActor* Actor, FName OverrideMeshLookupTag)
{
	if (IsValid(Actor))
	{
		if (OverrideMeshLookupTag != NAME_None)
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), OverrideMeshLookupTag);
			if (Components.IsValidIndex(0))
			{
				return Cast<USkeletalMeshComponent>(Components[0]);
			}
		}
		else if (const USigilCombatSystemSettings* Settings = USigilCombatSystemSettings::Get())
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), Settings->CharacterMeshLookupTag);
			if (Components.IsValidIndex(0))
			{
				return Cast<USkeletalMeshComponent>(Components[0]);
			}
		}

		if (ACharacter* Char = Cast<ACharacter>(Actor))
		{
			return Char->GetMesh();
		}

		if (USkeletalMeshComponent* Component = Cast<USkeletalMeshComponent>(Actor->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
		{
			return Component;
		}

		UE_LOG(LogSigilCombat, Warning, TEXT("Failed to find main character mesh component on actor class:%s"), *Actor->GetClass()->GetName());
	}

	return nullptr;
}

UMeshComponent* USigilCombatFunctionLibrary::GetMainMeshComponent(AActor* Actor, FName OverrideMeshLookupTag)
{
	if (IsValid(Actor))
	{
		if (OverrideMeshLookupTag != NAME_None)
		{
			if (UActorComponent* Component = Actor->FindComponentByTag(UMeshComponent::StaticClass(), OverrideMeshLookupTag))
			{
				return Cast<UMeshComponent>(Component);
			}
		}
		else if (const USigilCombatSystemSettings* Settings = USigilCombatSystemSettings::Get())
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByTag(UMeshComponent::StaticClass(), Settings->CharacterMeshLookupTag);
			if (Components.IsValidIndex(0))
			{
				return Cast<UMeshComponent>(Components[0]);
			}
		}

		if (UMeshComponent* Component = Cast<UMeshComponent>(Actor->GetComponentByClass(UMeshComponent::StaticClass())))
		{
			return Component;
		}

		UE_LOG(LogSigilCombat, Warning, TEXT("Failed to find main mesh component on actor class:%s"), *Actor->GetClass()->GetName());
	}

	return nullptr;
}

TArray<FName> USigilCombatFunctionLibrary::GetSocketNamesWithPrefix(const USceneComponent* Component, FString Prefix, ESearchCase::Type SearchCase)
{
	if (IsValid(Component))
	{
		return Component->GetAllSocketNames().FilterByPredicate([&](const FName& SocketName)
		{
			return SocketName.ToString().StartsWith(Prefix, SearchCase);
		});
	}
	return {};
}

bool USigilCombatFunctionLibrary::FindCombatInterface(AActor* Actor, TScriptInterface<ISigilCombatInterface>& OutInterface)
{
	OutInterface = GetCombatInterface(Actor);
	return OutInterface.GetObject() != nullptr;
}

bool USigilCombatFunctionLibrary::FindWeaponInterface(AActor* Actor, TScriptInterface<ISigilWeaponInterface>& OutInterface)
{
	OutInterface = GetWeaponInterface(Actor);
	return OutInterface.GetObject() != nullptr;
}

FRotator USigilCombatFunctionLibrary::CalculateAngleBetweenActors(const AActor* From, const AActor* To)
{
	if (IsValid(From) && IsValid(To))
	{
		return UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(From->GetActorLocation(), To->GetActorLocation()), From->GetActorRotation());
	}

	// TODO Warning.
	return FRotator::ZeroRotator;
}

ESigilDirection USigilCombatFunctionLibrary::CalculateDirectionFromAngle(const float Angle)
{
	if (UKismetMathLibrary::InRange_FloatFloat(Angle, -45.0f, 45.0f))
	{
		return ESigilDirection::Forward;
	}

	if (UKismetMathLibrary::InRange_FloatFloat(Angle, 45.0f, 135.f))
	{
		return ESigilDirection::Right;
	}

	if (UKismetMathLibrary::InRange_FloatFloat(Angle, -135.f, -45.f))
	{
		return ESigilDirection::Left;
	}
	return ESigilDirection::Backward;
}

TSoftObjectPtr<UAnimMontage> USigilCombatFunctionLibrary::SelectMontageByDirection(ESigilDirection Direction, TArray<TSoftObjectPtr<UAnimMontage>> Montages)
{
	switch (Direction)
	{
	case ESigilDirection::Forward:
		return Montages.IsValidIndex(0) ? Montages[0] : nullptr;
	case ESigilDirection::Backward:
		return Montages.IsValidIndex(1) ? Montages[1] : nullptr;
	case ESigilDirection::Left:
		return Montages.IsValidIndex(2) ? Montages[2] : nullptr;
	case ESigilDirection::Right:
		return Montages.IsValidIndex(3) ? Montages[3] : nullptr;
	}
	return nullptr;
}

void USigilCombatFunctionLibrary::AddTaggedValue(TArray<FSigilTaggedValue>& TaggedValues, FGameplayTag Tag, float ValueToAdd)
{
	bool bFound = false;
	for (FSigilTaggedValue& TaggedValue : TaggedValues)
	{
		if (TaggedValue.Attribute == Tag)
		{
			TaggedValue.Value += ValueToAdd;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		FSigilTaggedValue Temp;
		Temp.Attribute = Tag;
		Temp.Value = ValueToAdd;
		TaggedValues.Add(Temp);
	}
}

float USigilCombatFunctionLibrary::GetTaggedValue(const TArray<FSigilTaggedValue> TaggedValues, FGameplayTag Tag)
{
	for (const FSigilTaggedValue& TaggedValue : TaggedValues)
	{
		if (TaggedValue.Attribute == Tag)
		{
			return TaggedValue.Value;
		}
	}

	return 0;
}

FGameplayTagContainer USigilCombatFunctionLibrary::FilterGameplayTagContainer(const FGameplayTagContainer& TagContainer, FGameplayTagContainer OtherContainer)
{
	return TagContainer.Filter(OtherContainer);
}

FGameplayEffectSpecHandle USigilCombatFunctionLibrary::AddAttackHandleToGameplayEffectSpec(FGameplayEffectSpecHandle SpecHandle, FDataTableRowHandle AttackHandle)
{
	if (SpecHandle.IsValid() && !AttackHandle.IsNull())
	{
		if (FSigilAttackDefinition* AtkDef = AttackHandle.GetRow<FSigilAttackDefinition>(TEXT("AddAttackHandleToGameplayEffectSpec")))
		{
			AddAttackDefinitionToGameplayEffectSpec(SpecHandle, *AtkDef);
		}

		FGameplayEffectContextHandle ContextHandle = SpecHandle.Data->GetEffectContext();
		EffectContextSetAttackDefinitionHandle(ContextHandle,AttackHandle);
	}
	return SpecHandle;
}

FGameplayEffectSpecHandle USigilCombatFunctionLibrary::AddAttackDefinitionToGameplayEffectSpec(FGameplayEffectSpecHandle SpecHandle, const FSigilAttackDefinition& AtkDefinition)
{
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->AppendDynamicAssetTags(AtkDefinition.AttackTags);

		// apply set by callers from atk definition.

		for (const TTuple<FGameplayTag, float>& ByCallerMagnitude : AtkDefinition.SetByCallerMagnitudes)
		{
			if (ByCallerMagnitude.Key.IsValid() && ByCallerMagnitude.Value > 0)
			{
				SpecHandle.Data->SetSetByCallerMagnitude(ByCallerMagnitude.Key, ByCallerMagnitude.Value);
			}
		}
	}

	return SpecHandle;
}

void USigilCombatFunctionLibrary::AddAttackHandleToGameplayEffectContainerSpec(FSigilGameplayEffectContainerSpec ContainerSpec, FDataTableRowHandle AttackHandle)
{
	if (!AttackHandle.IsNull())
	{
		if (FSigilAttackDefinition* AtkDef = AttackHandle.GetRow<FSigilAttackDefinition>(TEXT("AddAttackHandleToGameplayEffectSpec")))
		{
			for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
			{
				AddAttackDefinitionToGameplayEffectSpec(SpecHandle,*AtkDef);
				FGameplayEffectContextHandle ContextHandle = SpecHandle.Data->GetEffectContext();
				EffectContextSetAttackDefinitionHandle(ContextHandle,AttackHandle);
			}
		}
	}
}

void USigilCombatFunctionLibrary::EffectContextSetAttackDefinitionHandle(FGameplayEffectContextHandle EffectContext, FDataTableRowHandle Handle)
{
	FSigilGameplayEffectContext* Context = static_cast<FSigilGameplayEffectContext*>(EffectContext.Get());
	if (Context)
	{
		Context->SetAttackDefinitionHandle(Handle);
	}
	else
	{
		UE_LOG(LogSigilCombat, Error, TEXT("Can't access SigilGameplayEffectContext! You need to setup SigilCombatAbilitySystemGlobals as AbilitySystemGlobalsClassName."))
	}
}

FDataTableRowHandle USigilCombatFunctionLibrary::EffectContextGetAttackDefinitionHandle(FGameplayEffectContextHandle EffectContext)
{
	const FSigilGameplayEffectContext* Context = static_cast<const FSigilGameplayEffectContext*>(EffectContext.Get());
	if (Context)
	{
		return Context->GetAttackDefinitionHandle();
	}
	else
	{
		UE_LOG(LogSigilCombat, Error, TEXT("Can't access SigilGameplayEffectContext! You need to setup SigilCombatAbilitySystemGlobals as AbilitySystemGlobalsClassName."))
	}

	return FDataTableRowHandle();
}

FSigilAttackDefinition USigilCombatFunctionLibrary::EffectContextGetAttackDefinition(FGameplayEffectContextHandle EffectContext)
{
	FDataTableRowHandle Handle = EffectContextGetAttackDefinitionHandle(EffectContext);
	if (FSigilAttackDefinition* Def = Handle.GetRow<FSigilAttackDefinition>(TEXT("EffectContextGetAttackDefinition")))
	{
		return *Def;
	}
	return FSigilAttackDefinition();
}
