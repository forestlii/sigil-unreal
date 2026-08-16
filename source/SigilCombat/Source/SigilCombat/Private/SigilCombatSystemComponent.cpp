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
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MontageClearTimerHandle);
	}
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
	// Attack results are server-authoritative: they replicate down through the FastArray.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogSigilCombat, Warning, TEXT("RegisterAttackResult called without authority on %s; ignored."), *GetNameSafe(GetOwner()));
		return;
	}
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

namespace
{
	/** Server-world time if a GameState exists, otherwise local world time (only reachable very early in a level's life). */
	float GetSigilServerWorldTime(const UWorld* World)
	{
		if (World)
		{
			if (const AGameStateBase* GameState = World->GetGameState())
			{
				return GameState->GetServerWorldTimeSeconds();
			}
			return World->GetTimeSeconds();
		}
		return 0.0f;
	}

	/** Montage-timeline position (seconds) a request starts from: the named section's start, else the explicit start time. */
	float GetMontageRequestStartPosition(const UAnimMontage* Montage, FName StartSectionName, float StartTimeSeconds)
	{
		if (StartSectionName != NAME_None)
		{
			const int32 SectionIndex = Montage->GetSectionIndex(StartSectionName);
			if (SectionIndex != INDEX_NONE)
			{
				return Montage->GetAnimCompositeSection(SectionIndex).GetTime();
			}
		}
		return FMath::Clamp(StartTimeSeconds, 0.0f, Montage->GetPlayLength());
	}
}

bool USigilCombatSystemComponent::IsMontageRequestValid(const USigilCombatSystemComponent* TargetCSC, const FSigilPlayMontageRequest& Request, FString* OutReason) const
{
	auto Reject = [OutReason](const TCHAR* Reason)
	{
		if (OutReason) { *OutReason = Reason; }
		return false;
	};

	if (!IsValid(TargetCSC) || !IsValid(TargetCSC->GetOwner()))
	{
		return Reject(TEXT("target combat system component is invalid"));
	}
	if (!IsValid(Request.AnimMontage))
	{
		return Reject(TEXT("montage is null"));
	}
	if (!FMath::IsFinite(Request.PlayRate) || Request.PlayRate <= KINDA_SMALL_NUMBER)
	{
		return Reject(TEXT("play rate must be a finite positive number"));
	}
	if (!FMath::IsFinite(Request.RootTranslationScale))
	{
		return Reject(TEXT("root translation scale is not finite"));
	}
	const float MontageLength = Request.AnimMontage->GetPlayLength();
	if (MontageLength <= KINDA_SMALL_NUMBER)
	{
		return Reject(TEXT("montage has zero length"));
	}
	if (!FMath::IsFinite(Request.StartTimeSeconds) || Request.StartTimeSeconds < 0.0f || Request.StartTimeSeconds >= MontageLength)
	{
		return Reject(TEXT("start time is outside the montage"));
	}
	if (Request.StartSectionName != NAME_None && Request.AnimMontage->GetSectionIndex(Request.StartSectionName) == INDEX_NONE)
	{
		return Reject(TEXT("start section does not exist in the montage"));
	}
	return true;
}

bool USigilCombatSystemComponent::CanPlayMontageOnTarget_Implementation(const USigilCombatSystemComponent* TargetCSC, const FSigilPlayMontageRequest& Request) const
{
	if (TargetCSC == this)
	{
		return true;
	}
	if (!IsValid(TargetCSC) || TargetCSC->GetWorld() != GetWorld())
	{
		return false;
	}

	const float MaxDistance = GetDefault<USigilCombatSystemSettings>()->MaxPredictableMontageTargetDistance;
	if (MaxDistance > 0.0f)
	{
		const AActor* Instigator = GetOwner();
		const AActor* Target = TargetCSC->GetOwner();
		if (!Instigator || !Target || FVector::DistSquared(Instigator->GetActorLocation(), Target->GetActorLocation()) > FMath::Square(MaxDistance))
		{
			return false;
		}
	}
	return true;
}

void USigilCombatSystemComponent::PlayPredictableMontageForTarget(USigilCombatSystemComponent* TargetCSC, FSigilPlayMontageRequest Request)
{
	FString Reason;
	if (!IsMontageRequestValid(TargetCSC, Request, &Reason))
	{
		UE_LOG(LogSigilCombat, Warning, TEXT("PlayPredictableMontageForTarget rejected on %s: %s."), *GetNameSafe(GetOwner()), *Reason);
		return;
	}

	if (GetOwnerRole() >= ROLE_Authority)
	{
		if (!CanPlayMontageOnTarget(TargetCSC, Request))
		{
			UE_LOG(LogSigilCombat, Warning, TEXT("%s is not allowed to play a montage on %s."), *GetNameSafe(GetOwner()), *GetNameSafe(TargetCSC->GetOwner()));
			return;
		}
		TargetCSC->SetReplicatedMontage(Request);
	}
	else if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		TargetCSC->PlayPredictedMontage(Request);
		ServerPlayPredictableMontageForTarget(TargetCSC, Request);
	}
}


void USigilCombatSystemComponent::ServerPlayPredictableMontageForTarget_Implementation(USigilCombatSystemComponent* TargetCSC, FSigilPlayMontageRequest Request)
{
	// Never trust the client: re-validate structure and authorization on the server. Rejections are logged (rate-limited), never asserted.
	FString Reason;
	if (!IsMontageRequestValid(TargetCSC, Request, &Reason))
	{
		static double LastLogTime = -100.0;
		const double Now = FPlatformTime::Seconds();
		if (Now - LastLogTime > 1.0)
		{
			LastLogTime = Now;
			UE_LOG(LogSigilCombat, Warning, TEXT("Server rejected montage request from %s: %s."), *GetNameSafe(GetOwner()), *Reason);
		}
		return;
	}
	if (!CanPlayMontageOnTarget(TargetCSC, Request))
	{
		static double LastLogTime = -100.0;
		const double Now = FPlatformTime::Seconds();
		if (Now - LastLogTime > 1.0)
		{
			LastLogTime = Now;
			UE_LOG(LogSigilCombat, Warning, TEXT("Server rejected montage request from %s targeting %s: not authorized."), *GetNameSafe(GetOwner()), *GetNameSafe(TargetCSC->GetOwner()));
		}
		return;
	}
	TargetCSC->SetReplicatedMontage(Request);
}


void USigilCombatSystemComponent::SetReplicatedMontage(const FSigilPlayMontageRequest& Request)
{
	UWorld* World = GetWorld();
	if (!World || !IsMontageRequestValid(this, Request))
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(MontageClearTimerHandle);
	++MontageRequestSerial;

	const float StartPosition = GetMontageRequestStartPosition(Request.AnimMontage, Request.StartSectionName, Request.StartTimeSeconds);

	ReplicatedMontageInfo.AnimMontage = Request.AnimMontage;
	ReplicatedMontageInfo.PlayRate = Request.PlayRate;
	ReplicatedMontageInfo.TriggeredTime = GetSigilServerWorldTime(World);
	ReplicatedMontageInfo.StartSectionName = Request.StartSectionName;
	ReplicatedMontageInfo.StartTimeSeconds = StartPosition;

	// Wall-clock duration of the remaining montage: (length - start) / rate. A higher rate plays faster and finishes sooner.
	const float RemainingLength = FMath::Max(Request.AnimMontage->GetPlayLength() - StartPosition, 0.0f);
	const float ClearDelay = FMath::Max(RemainingLength / Request.PlayRate, KINDA_SMALL_NUMBER);

	const uint32 Serial = MontageRequestSerial;
	TimerManager.SetTimer(MontageClearTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, Serial]()
	{
		// A newer request may have replaced this montage; only the matching serial may clear it.
		if (Serial == MontageRequestSerial)
		{
			ReplicatedMontageInfo.AnimMontage = nullptr;
		}
	}), ClearDelay, false);

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

	USkeletalMeshComponent* MeshComponent = GetCharacterMeshComponent();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return;
	}

	UAnimMontage* MontageToPlay = ReplicatedMontageInfo.AnimMontage;
	const float PlayRate = FMath::Max(ReplicatedMontageInfo.PlayRate, KINDA_SMALL_NUMBER);
	const float MontageLength = MontageToPlay->GetPlayLength();

	// Convert elapsed server time into montage-timeline time and clamp to the montage's valid range.
	const float ElapsedWorldTime = FMath::Max(GetSigilServerWorldTime(GetWorld()) - ReplicatedMontageInfo.TriggeredTime, 0.0f);
	const float MontageTime = FMath::Clamp(ReplicatedMontageInfo.StartTimeSeconds + ElapsedWorldTime * PlayRate, 0.0f, MontageLength);

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

	AnimInstance->Montage_Play(MontageToPlay, PlayRate, EMontagePlayReturnType::MontageLength, MontageTime);
}

void USigilCombatSystemComponent::PlayPredictedMontage(const FSigilPlayMontageRequest& Request)
{
	if (!IsMontageRequestValid(this, Request))
	{
		return;
	}

	PredictedMontageInfo.AnimMontage = Request.AnimMontage;
	PredictedMontageInfo.PlayRate = Request.PlayRate;
	PredictedMontageInfo.TriggeredTime = GetSigilServerWorldTime(GetWorld());
	PredictedMontageInfo.StartSectionName = Request.StartSectionName;

	USkeletalMeshComponent* MeshComponent = GetCharacterMeshComponent();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return;
	}

	const float Duration = AnimInstance->Montage_Play(Request.AnimMontage, Request.PlayRate, EMontagePlayReturnType::MontageLength, Request.StartTimeSeconds);
	if (Duration > 0.0f && Request.StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(Request.StartSectionName, Request.AnimMontage);
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
	if (!IsValid(CombatFlow))
	{
		return;
	}
	CombatFlow->Initialize(GetOwner());
	UE_LOG(LogSigilCombat, Display, TEXT("Combat flow replicated for %s"), *GetNameSafe(GetOwner()));

	// Attack results may have replicated before the CombatFlow subobject; process them now.
	AttackResultContainer.ConsumePendingEntries();
}
