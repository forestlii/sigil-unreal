// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GCS_CombatSystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "TimerManager.h"
#include "GCS_LogChannels.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "CombatFlow/GCS_AttackRequest.h"
#include "CombatFlow/GCS_CombatFlow.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Utility/GCS_CombatFunctionLibrary.h"
#include "GCS_CombatSystemSettings.h"


UGCS_CombatSystemComponent::UGCS_CombatSystemComponent(): AttackResultContainer(this, 10)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
	bReplicateUsingRegisteredSubObjectList = true;
}

void UGCS_CombatSystemComponent::InitializeComponent()
{
	AttackResultContainer.SetOwningCombatSystem(this);
	Super::InitializeComponent();
}

void UGCS_CombatSystemComponent::BeginPlay()
{
	if (GetWorld()->IsGameWorld())
	{
		if (GetOwner()->HasAuthority() && CombatFlowClass)
		{
			UGCS_CombatFlow* LocalNewProperty = NewObject<UGCS_CombatFlow>(GetOwner(), CombatFlowClass);
			LocalNewProperty->Initialize(GetOwner());
			CombatFlow = LocalNewProperty;
			AddReplicatedSubObject(CombatFlow);
		}
		UGGA_AbilitySystemGlobals::RegisterEventReceiver(this);
	}
	Super::BeginPlay();
}

void UGCS_CombatSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UGGA_AbilitySystemGlobals::UnregisterEventReceiver(this);
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (CombatFlow)
		{
			RemoveReplicatedSubObject(CombatFlow);
		}
	}
}

void UGCS_CombatSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CombatFlow);
	DOREPLIFETIME(ThisClass, AttackResultContainer);
	DOREPLIFETIME(ThisClass, ReplicatedMontageInfo);
}

UGCS_CombatSystemComponent* UGCS_CombatSystemComponent::GetCombatSystemComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UGCS_CombatSystemComponent>() : nullptr;
}

bool UGCS_CombatSystemComponent::FindCombatSystemComponent(const AActor* Actor, UGCS_CombatSystemComponent*& CombatComponent)
{
	CombatComponent = Actor ? Actor->FindComponentByClass<UGCS_CombatSystemComponent>() : nullptr;
	return IsValid(CombatComponent);
}

bool UGCS_CombatSystemComponent::FindTypedCombatSystemComponent(AActor* Actor, TSubclassOf<UGCS_CombatSystemComponent> DesiredClass, UGCS_CombatSystemComponent*& Component)
{
	if (UGCS_CombatSystemComponent* Instance = GetCombatSystemComponent(Actor))
	{
		if (Instance->GetClass()->IsChildOf(DesiredClass))
		{
			Component = Instance;
			return true;
		}
	}
	return false;
}

UGCS_CombatFlow* UGCS_CombatSystemComponent::GetCombatFlow() const
{
	return CombatFlow;
}

void UGCS_CombatSystemComponent::RegisterAttackResult(FGCS_AttackResult& Payload)
{
	//TODO Should make this server only?
	AttackResultContainer.AddEntry(Payload);
}

FGCS_AttackResult UGCS_CombatSystemComponent::GetLastProcessedAttackResult() const
{
	return LastProcessedAttackResult;
}

void UGCS_CombatSystemComponent::SetLastProcessedAttackResult(const FGCS_AttackResult& Payload)
{
	LastProcessedAttackResult = Payload;
}

void UGCS_CombatSystemComponent::PlayPredictableMontageForTarget(UGCS_CombatSystemComponent* TargetCSC, FGCS_PlayMontageRequest Request)
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


void UGCS_CombatSystemComponent::ServerPlayPredictableMontageForTarget_Implementation(UGCS_CombatSystemComponent* TargetCSC, FGCS_PlayMontageRequest Request)
{
	TargetCSC->SetReplicatedMontage(Request);
}


void UGCS_CombatSystemComponent::SetReplicatedMontage(const FGCS_PlayMontageRequest& Request)
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
void UGCS_CombatSystemComponent::OnRep_ReplicatedMontageInfo()
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

void UGCS_CombatSystemComponent::PlayPredictedMontage(const FGCS_PlayMontageRequest& Request)
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

USkeletalMeshComponent* UGCS_CombatSystemComponent::GetCharacterMeshComponent() const
{
	const UGCS_CombatSystemSettings* Settings = GetDefault<UGCS_CombatSystemSettings>();

	return Cast<USkeletalMeshComponent>(GetOwner()->FindComponentByTag(USkeletalMeshComponent::StaticClass(), Settings->CharacterMeshLookupTag));
}

void UGCS_CombatSystemComponent::OnGlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent)
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

void UGCS_CombatSystemComponent::OnRep_CombatFlow()
{
	CombatFlow->Initialize(GetOwner());
	UE_LOG(LogGCS, Display, TEXT("Combat flow replicated for %s"), *GetOwner()->GetName());
}
