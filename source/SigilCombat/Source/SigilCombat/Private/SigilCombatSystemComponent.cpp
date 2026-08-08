// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilCombatSystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "TimerManager.h"
#include "SigilCombatLogChannels.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "CombatFlow/SigilAttackRequest.h"
#include "CombatFlow/SigilCombatFlow.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Utility/SigilCombatFunctionLibrary.h"
#include "SigilCombatSystemSettings.h"


USigilCombatSystemComponent::USigilCombatSystemComponent(): AttackResultContainer(this, 10)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
	bReplicateUsingRegisteredSubObjectList = true;
}

void USigilCombatSystemComponent::InitializeComponent()
{
	AttackResultContainer.SetOwningCombatSystem(this);
	Super::InitializeComponent();
}

void USigilCombatSystemComponent::BeginPlay()
{
	if (GetWorld()->IsGameWorld())
	{
		if (GetOwner()->HasAuthority() && CombatFlowClass)
		{
			USigilCombatFlow* LocalNewProperty = NewObject<USigilCombatFlow>(GetOwner(), CombatFlowClass);
			LocalNewProperty->Initialize(GetOwner());
			CombatFlow = LocalNewProperty;
			AddReplicatedSubObject(CombatFlow);
		}
		USigilAbilitySystemGlobals::RegisterEventReceiver(this);
	}
	Super::BeginPlay();
}

void USigilCombatSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	USigilAbilitySystemGlobals::UnregisterEventReceiver(this);
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (CombatFlow)
		{
			RemoveReplicatedSubObject(CombatFlow);
		}
	}
}

void USigilCombatSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CombatFlow);
	DOREPLIFETIME(ThisClass, AttackResultContainer);
	DOREPLIFETIME(ThisClass, ReplicatedMontageInfo);
}

USigilCombatSystemComponent* USigilCombatSystemComponent::GetCombatSystemComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<USigilCombatSystemComponent>() : nullptr;
}

bool USigilCombatSystemComponent::FindCombatSystemComponent(const AActor* Actor, USigilCombatSystemComponent*& CombatComponent)
{
	CombatComponent = Actor ? Actor->FindComponentByClass<USigilCombatSystemComponent>() : nullptr;
	return IsValid(CombatComponent);
}

bool USigilCombatSystemComponent::FindTypedCombatSystemComponent(AActor* Actor, TSubclassOf<USigilCombatSystemComponent> DesiredClass, USigilCombatSystemComponent*& Component)
{
	if (USigilCombatSystemComponent* Instance = GetCombatSystemComponent(Actor))
	{
		if (Instance->GetClass()->IsChildOf(DesiredClass))
		{
			Component = Instance;
			return true;
		}
	}
	return false;
}

USigilCombatFlow* USigilCombatSystemComponent::GetCombatFlow() const
{
	return CombatFlow;
}

void USigilCombatSystemComponent::RegisterAttackResult(FSigilAttackResult& Payload)
{
	//TODO Should make this server only?
	AttackResultContainer.AddEntry(Payload);
}

FSigilAttackResult USigilCombatSystemComponent::GetLastProcessedAttackResult() const
{
	return LastProcessedAttackResult;
}

void USigilCombatSystemComponent::SetLastProcessedAttackResult(const FSigilAttackResult& Payload)
{
	LastProcessedAttackResult = Payload;
}

void USigilCombatSystemComponent::PlayPredictableMontageForTarget(USigilCombatSystemComponent* TargetCSC, FSigilPlayMontageRequest Request)
{
	if (GetOwnerRole() >= ROLE_Authority)
	{
		TargetCSC->SetReplicatedMontage(Request);
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		TargetCSC->PlayPredictedMontage(Request);
		ServerPlayPredictableMontageForTarget(TargetCSC, Request);
	}
}


void USigilCombatSystemComponent::ServerPlayPredictableMontageForTarget_Implementation(USigilCombatSystemComponent* TargetCSC, FSigilPlayMontageRequest Request)
{
	TargetCSC->SetReplicatedMontage(Request);
}


void USigilCombatSystemComponent::SetReplicatedMontage(const FSigilPlayMontageRequest& Request)
{
	TimerHandle.Invalidate();
	ReplicatedMontageInfo.AnimMontage = Request.AnimMontage;
	ReplicatedMontageInfo.PlayRate = Request.PlayRate;
	ReplicatedMontageInfo.TriggeredTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	ReplicatedMontageInfo.StartSectionName = Request.StartSectionName;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
	{
		ReplicatedMontageInfo.AnimMontage = nullptr;
		TimerHandle.Invalidate();
	}, Request.AnimMontage->GetPlayLength() * Request.PlayRate, false);


	OnRep_ReplicatedMontageInfo();
}

//server tell me to play montage.
void USigilCombatSystemComponent::OnRep_ReplicatedMontageInfo()
{
	if (ReplicatedMontageInfo.AnimMontage == nullptr)
	{
		PredictedMontageInfo.AnimMontage = nullptr;
		return;
	}
	if (USkeletalMeshComponent* MeshComponent = GetCharacterMeshComponent())
	{
		UAnimMontage* MontageToPlay = ReplicatedMontageInfo.AnimMontage;
		float TimeDiff = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() - ReplicatedMontageInfo.TriggeredTime;
		float StartTime = FMath::Clamp(TimeDiff, 0, ReplicatedMontageInfo.AnimMontage->GetPlayLength() * ReplicatedMontageInfo.PlayRate);

		//If local montage ahead of replicated montage
		if (PredictedMontageInfo.AnimMontage != nullptr)
		{
			//And it's the same.
			if (ReplicatedMontageInfo.AnimMontage == PredictedMontageInfo.AnimMontage)
			{
				PredictedMontageInfo.AnimMontage = nullptr;
				return;
			}
			PredictedMontageInfo.AnimMontage = nullptr;
		}

		MeshComponent->GetAnimInstance()->Montage_Play(MontageToPlay, ReplicatedMontageInfo.PlayRate, EMontagePlayReturnType::MontageLength, StartTime);
	}
}

void USigilCombatSystemComponent::PlayPredictedMontage(const FSigilPlayMontageRequest& Request)
{
	PredictedMontageInfo.AnimMontage = Request.AnimMontage;
	PredictedMontageInfo.PlayRate = Request.PlayRate;
	PredictedMontageInfo.TriggeredTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	PredictedMontageInfo.StartSectionName = Request.StartSectionName;
	if (USkeletalMeshComponent* MeshComponent = GetCharacterMeshComponent())
	{
		float Duration = MeshComponent->GetAnimInstance()->Montage_Play(Request.AnimMontage, Request.PlayRate, EMontagePlayReturnType::MontageLength, Request.StartTimeSeconds);
		if (Duration > 0)
		{
			// Start at a given Section.
			if (Request.StartSectionName != NAME_None)
			{
				MeshComponent->GetAnimInstance()->Montage_JumpToSection(Request.StartSectionName, Request.AnimMontage);
			}
		}
	}
}

USkeletalMeshComponent* USigilCombatSystemComponent::GetCharacterMeshComponent() const
{
	const USigilCombatSystemSettings* Settings = GetDefault<USigilCombatSystemSettings>();

	return Cast<USkeletalMeshComponent>(GetOwner()->FindComponentByTag(USkeletalMeshComponent::StaticClass(), Settings->CharacterMeshLookupTag));
}

void USigilCombatSystemComponent::OnGlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent)
{
	if (IsValid(CombatFlow))
	{
		FGameplayTagContainer DynamicTags;
		CombatFlow->HandlePreGameplayEffectSpecApply(Spec, AbilitySystemComponent, DynamicTags);
		if (!DynamicTags.IsEmpty())
		{
			Spec.AppendDynamicAssetTags(DynamicTags);
		}
	}
}

void USigilCombatSystemComponent::OnRep_CombatFlow()
{
	CombatFlow->Initialize(GetOwner());
	UE_LOG(LogSigilCombat, Display, TEXT("Combat flow replicated for %s"), *GetOwner()->GetName());
}
