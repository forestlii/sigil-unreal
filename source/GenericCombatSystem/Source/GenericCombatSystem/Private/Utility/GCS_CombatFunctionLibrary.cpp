// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utility/GCS_CombatFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GCS_CombatInterface.h"
#include "GCS_CombatSystemSettings.h"
#include "GCS_CombatTeamAgentInterface.h"
#include "GCS_LogChannels.h"
#include "AbilitySystem/GCS_GameplayEffectContext.h"
#include "CombatFlow/GCS_AttackRequest.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/GCS_WeaponInterface.h"

TScriptInterface<IGCS_CombatTeamAgentInterface> UGCS_CombatFunctionLibrary::GetCombatTeamAgentInterface(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (Actor->GetClass()->ImplementsInterface(UGCS_CombatTeamAgentInterface::StaticClass()))
		{
			return Actor;
		}
		TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(UGCS_CombatTeamAgentInterface::StaticClass());
		return Components.IsValidIndex(0) ? Components[0] : nullptr;
	}
	return nullptr;
}

bool UGCS_CombatFunctionLibrary::FindCombatTeamAgentInterface(AActor* Actor, TScriptInterface<IGCS_CombatTeamAgentInterface>& OutInterface)
{
	OutInterface = GetCombatTeamAgentInterface(Actor);
	return OutInterface != nullptr;
}

TScriptInterface<IGCS_CombatInterface> UGCS_CombatFunctionLibrary::GetCombatInterface(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (Actor->GetClass()->ImplementsInterface(UGCS_CombatInterface::StaticClass()))
		{
			return Actor;
		}
		else
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(UGCS_CombatInterface::StaticClass());
			return Components.IsValidIndex(0) ? Components[0] : nullptr;
		}
	}
	return nullptr;
}

UObject* UGCS_CombatFunctionLibrary::GetCombatInterfaceImplementer(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (Actor->GetClass()->ImplementsInterface(UGCS_CombatInterface::StaticClass()))
		{
			return Actor;
		}
		else
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(UGCS_CombatInterface::StaticClass());
			return Components.IsValidIndex(0) ? Components[0] : nullptr;
		}
	}
	return nullptr;
}

TScriptInterface<IGCS_WeaponInterface> UGCS_CombatFunctionLibrary::GetWeaponInterface(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (Actor->GetClass()->ImplementsInterface(UGCS_WeaponInterface::StaticClass()))
		{
			return Actor;
		}
		else
		{
			TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(UGCS_WeaponInterface::StaticClass());
			return Components.IsValidIndex(0) ? Components[0] : nullptr;
		}
	}
	return nullptr;
}

USkeletalMeshComponent* UGCS_CombatFunctionLibrary::GetMainCharacterMeshComponent(AActor* Actor, FName OverrideMeshLookupTag)
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
		else if (const UGCS_CombatSystemSettings* Settings = UGCS_CombatSystemSettings::Get())
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

		UE_LOG(LogGCS, Warning, TEXT("Failed to find main character mesh component on actor class:%s"), *Actor->GetClass()->GetName());
	}

	return nullptr;
}

UMeshComponent* UGCS_CombatFunctionLibrary::GetMainMeshComponent(AActor* Actor, FName OverrideMeshLookupTag)
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
		else if (const UGCS_CombatSystemSettings* Settings = UGCS_CombatSystemSettings::Get())
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

		UE_LOG(LogGCS, Warning, TEXT("Failed to find main mesh component on actor class:%s"), *Actor->GetClass()->GetName());
	}

	return nullptr;
}

TArray<FName> UGCS_CombatFunctionLibrary::GetSocketNamesWithPrefix(const USceneComponent* Component, FString Prefix, ESearchCase::Type SearchCase)
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

bool UGCS_CombatFunctionLibrary::FindCombatInterface(AActor* Actor, TScriptInterface<IGCS_CombatInterface>& OutInterface)
{
	OutInterface = GetCombatInterface(Actor);
	return OutInterface.GetObject() != nullptr;
}

bool UGCS_CombatFunctionLibrary::FindWeaponInterface(AActor* Actor, TScriptInterface<IGCS_WeaponInterface>& OutInterface)
{
	OutInterface = GetWeaponInterface(Actor);
	return OutInterface.GetObject() != nullptr;
}

FRotator UGCS_CombatFunctionLibrary::CalculateAngleBetweenActors(const AActor* From, const AActor* To)
{
	if (IsValid(From) && IsValid(To))
	{
		return UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(From->GetActorLocation(), To->GetActorLocation()), From->GetActorRotation());
	}

	// TODO Warning.
	return FRotator::ZeroRotator;
}

EGCS_Direction UGCS_CombatFunctionLibrary::CalculateDirectionFromAngle(const float Angle)
{
	if (UKismetMathLibrary::InRange_FloatFloat(Angle, -45.0f, 45.0f))
	{
		return EGCS_Direction::Forward;
	}

	if (UKismetMathLibrary::InRange_FloatFloat(Angle, 45.0f, 135.f))
	{
		return EGCS_Direction::Right;
	}

	if (UKismetMathLibrary::InRange_FloatFloat(Angle, -135.f, -45.f))
	{
		return EGCS_Direction::Left;
	}
	return EGCS_Direction::Backward;
}

TSoftObjectPtr<UAnimMontage> UGCS_CombatFunctionLibrary::SelectMontageByDirection(EGCS_Direction Direction, TArray<TSoftObjectPtr<UAnimMontage>> Montages)
{
	switch (Direction)
	{
	case EGCS_Direction::Forward:
		return Montages.IsValidIndex(0) ? Montages[0] : nullptr;
	case EGCS_Direction::Backward:
		return Montages.IsValidIndex(1) ? Montages[1] : nullptr;
	case EGCS_Direction::Left:
		return Montages.IsValidIndex(2) ? Montages[2] : nullptr;
	case EGCS_Direction::Right:
		return Montages.IsValidIndex(3) ? Montages[3] : nullptr;
	}
	return nullptr;
}

void UGCS_CombatFunctionLibrary::AddTaggedValue(TArray<FGCS_TaggedValue>& TaggedValues, FGameplayTag Tag, float ValueToAdd)
{
	bool bFound = false;
	for (FGCS_TaggedValue& TaggedValue : TaggedValues)
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
		FGCS_TaggedValue Temp;
		Temp.Attribute = Tag;
		Temp.Value = ValueToAdd;
		TaggedValues.Add(Temp);
	}
}

float UGCS_CombatFunctionLibrary::GetTaggedValue(const TArray<FGCS_TaggedValue> TaggedValues, FGameplayTag Tag)
{
	for (const FGCS_TaggedValue& TaggedValue : TaggedValues)
	{
		if (TaggedValue.Attribute == Tag)
		{
			return TaggedValue.Value;
		}
	}

	return 0;
}

FGameplayTagContainer UGCS_CombatFunctionLibrary::FilterGameplayTagContainer(const FGameplayTagContainer& TagContainer, FGameplayTagContainer OtherContainer)
{
	return TagContainer.Filter(OtherContainer);
}

FGameplayEffectSpecHandle UGCS_CombatFunctionLibrary::AddAttackHandleToGameplayEffectSpec(FGameplayEffectSpecHandle SpecHandle, FDataTableRowHandle AttackHandle)
{
	if (SpecHandle.IsValid() && !AttackHandle.IsNull())
	{
		if (FGCS_AttackDefinition* AtkDef = AttackHandle.GetRow<FGCS_AttackDefinition>(TEXT("AddAttackHandleToGameplayEffectSpec")))
		{
			AddAttackDefinitionToGameplayEffectSpec(SpecHandle, *AtkDef);
		}

		FGameplayEffectContextHandle ContextHandle = SpecHandle.Data->GetEffectContext();
		EffectContextSetAttackDefinitionHandle(ContextHandle,AttackHandle);
	}
	return SpecHandle;
}

FGameplayEffectSpecHandle UGCS_CombatFunctionLibrary::AddAttackDefinitionToGameplayEffectSpec(FGameplayEffectSpecHandle SpecHandle, const FGCS_AttackDefinition& AtkDefinition)
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

void UGCS_CombatFunctionLibrary::AddAttackHandleToGameplayEffectContainerSpec(FGGA_GameplayEffectContainerSpec ContainerSpec, FDataTableRowHandle AttackHandle)
{
	if (!AttackHandle.IsNull())
	{
		if (FGCS_AttackDefinition* AtkDef = AttackHandle.GetRow<FGCS_AttackDefinition>(TEXT("AddAttackHandleToGameplayEffectSpec")))
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

void UGCS_CombatFunctionLibrary::EffectContextSetAttackDefinitionHandle(FGameplayEffectContextHandle EffectContext, FDataTableRowHandle Handle)
{
	FGCS_GameplayEffectContext* Context = static_cast<FGCS_GameplayEffectContext*>(EffectContext.Get());
	if (Context)
	{
		Context->SetAttackDefinitionHandle(Handle);
	}
	else
	{
		UE_LOG(LogGCS, Error, TEXT("Can't access GCS_GameplayEffectContext! You need to setup GCS_AbilitySystemGlobals as AbilitySystemGlobalsClassName."))
	}
}

FDataTableRowHandle UGCS_CombatFunctionLibrary::EffectContextGetAttackDefinitionHandle(FGameplayEffectContextHandle EffectContext)
{
	const FGCS_GameplayEffectContext* Context = static_cast<const FGCS_GameplayEffectContext*>(EffectContext.Get());
	if (Context)
	{
		return Context->GetAttackDefinitionHandle();
	}
	else
	{
		UE_LOG(LogGCS, Error, TEXT("Can't access GCS_GameplayEffectContext! You need to setup GCS_AbilitySystemGlobals as AbilitySystemGlobalsClassName."))
	}

	return FDataTableRowHandle();
}

FGCS_AttackDefinition UGCS_CombatFunctionLibrary::EffectContextGetAttackDefinition(FGameplayEffectContextHandle EffectContext)
{
	FDataTableRowHandle Handle = EffectContextGetAttackDefinitionHandle(EffectContext);
	if (FGCS_AttackDefinition* Def = Handle.GetRow<FGCS_AttackDefinition>(TEXT("EffectContextGetAttackDefinition")))
	{
		return *Def;
	}
	return FGCS_AttackDefinition();
}
