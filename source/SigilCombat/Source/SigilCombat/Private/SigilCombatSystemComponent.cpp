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
#include "GameFramework/Character.h"
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

bool USigilCombatSystemComponent::IsCombatFlowReady() const
{
	return IsValid(CombatFlow) && CombatFlow->IsInitialized();
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
	const USigilCombatSystemSettings* Settings = GetDefault<USigilCombatSystemSettings>();
	if (!FMath::IsFinite(Request.PlayRate) || Request.PlayRate < Settings->MinPredictableMontagePlayRate || Request.PlayRate > Settings->MaxPredictableMontagePlayRate)
	{
		return Reject(TEXT("play rate is not finite or outside the configured min/max"));
	}
	if (!FMath::IsFinite(Request.RootTranslationScale) || Request.RootTranslationScale < 0.0f || Request.RootTranslationScale > Settings->MaxPredictableMontageRootTranslationScale)
	{
		return Reject(TEXT("root translation scale is not finite or outside [0, max]"));
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
	if (!IsMontageLinear(Request.AnimMontage))
	{
		return Reject(TEXT("montage has looping or non-linear sections; predictable playback supports linear montages only"));
	}
	return true;
}

bool USigilCombatSystemComponent::IsMontageLinear(const UAnimMontage* Montage)
{
	if (!Montage)
	{
		return false;
	}
	const int32 NumSections = Montage->CompositeSections.Num();
	for (int32 Index = 0; Index < NumSections; ++Index)
	{
		const FName Next = Montage->CompositeSections[Index].NextSectionName;
		if (Next == NAME_None)
		{
			continue; // section ends the montage
		}
		const int32 NextIndex = Montage->GetSectionIndex(Next);
		if (NextIndex != Index + 1)
		{
			return false; // loops back, skips ahead, or references a missing section
		}
	}
	return true;
}

bool USigilCombatSystemComponent::CanPlayMontageOnTarget_Implementation(const USigilCombatSystemComponent* TargetCSC, const FSigilPlayMontageRequest& Request) const
{
	if (TargetCSC == this)
	{
		return true;
	}

	// Cross-target playback is opt-in: without a configured distance limit every other target is rejected.
	const float MaxDistance = GetDefault<USigilCombatSystemSettings>()->MaxPredictableMontageTargetDistance;
	if (MaxDistance <= 0.0f)
	{
		return false;
	}
	if (!IsValid(TargetCSC) || TargetCSC->GetWorld() != GetWorld())
	{
		return false;
	}
	const AActor* Instigator = GetOwner();
	const AActor* Target = TargetCSC->GetOwner();
	return Instigator && Target && FVector::DistSquared(Instigator->GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(MaxDistance);
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
		Request.RequestId = NextMontageRequestId++;
		TargetCSC->SetReplicatedMontage(Request);
	}
	else if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Request.RequestId = NextMontageRequestId++;

		// Prune stale book-keeping (predictions the server never answered explicitly, e.g. accepted ones).
		const double Now = FPlatformTime::Seconds();
		for (auto It = PendingPredictions.CreateIterator(); It; ++It)
		{
			if (Now - It->Value.IssuedTime > 10.0 || !It->Value.Target.IsValid())
			{
				It.RemoveCurrent();
			}
		}

		if (TargetCSC->PlayPredictedMontage(Request))
		{
			PendingPredictions.Add(Request.RequestId, FPendingPrediction{TargetCSC, Now});
		}
		ServerPlayPredictableMontageForTarget(TargetCSC, Request);
	}
}


void USigilCombatSystemComponent::ServerPlayPredictableMontageForTarget_Implementation(USigilCombatSystemComponent* TargetCSC, FSigilPlayMontageRequest Request)
{
	// Never trust the client: re-validate structure, rate and authorization on the server.
	// Rejections are logged (rate-limited) and echoed back so the client can roll back its prediction; never asserted.
	auto Reject = [this, &Request](const FString& Why)
	{
		static double LastLogTime = -100.0;
		const double Now = FPlatformTime::Seconds();
		if (Now - LastLogTime > 1.0)
		{
			LastLogTime = Now;
			UE_LOG(LogSigilCombat, Warning, TEXT("Server rejected montage request %d from %s: %s."), Request.RequestId, *GetNameSafe(GetOwner()), *Why);
		}
		ClientMontageRequestRejected(Request.RequestId);
	};

	// Per-component request rate limit.
	const int32 MaxPerSecond = GetDefault<USigilCombatSystemSettings>()->MaxPredictableMontageRequestsPerSecond;
	if (MaxPerSecond > 0)
	{
		const double Now = FPlatformTime::Seconds();
		if (Now - MontageRequestWindowStart >= 1.0)
		{
			MontageRequestWindowStart = Now;
			MontageRequestsInWindow = 0;
		}
		if (++MontageRequestsInWindow > MaxPerSecond)
		{
			Reject(TEXT("rate limit exceeded"));
			return;
		}
	}

	FString Reason;
	if (!IsMontageRequestValid(TargetCSC, Request, &Reason))
	{
		Reject(Reason);
		return;
	}
	if (!CanPlayMontageOnTarget(TargetCSC, Request))
	{
		Reject(FString::Printf(TEXT("not authorized to target %s"), *GetNameSafe(TargetCSC->GetOwner())));
		return;
	}
	TargetCSC->SetReplicatedMontage(Request);
}

void USigilCombatSystemComponent::ClientMontageRequestRejected_Implementation(int32 RequestId)
{
	FPendingPrediction Pending;
	if (PendingPredictions.RemoveAndCopyValue(RequestId, Pending))
	{
		if (USigilCombatSystemComponent* Target = Pending.Target.Get())
		{
			Target->CancelPredictedMontage(RequestId);
		}
	}
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
	ReplicatedMontageInfo.RequestId = Request.RequestId;
	ReplicatedMontageInfo.RootTranslationScale = Request.RootTranslationScale;

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
		PredictedMontageInfo = FSigilPredictedMontageInfo();
		return;
	}

	// Reconcile with a local prediction by request id (never by asset pointer: the same asset may be
	// requested with different sections / rates / start times).
	if (PredictedMontageInfo.RequestId != 0)
	{
		if (PredictedMontageInfo.RequestId == ReplicatedMontageInfo.RequestId)
		{
			// Server accepted exactly what we predicted; the montage is already playing locally.
			PredictedMontageInfo = FSigilPredictedMontageInfo();
			ApplyRootTranslationScale(ReplicatedMontageInfo.RootTranslationScale);
			return;
		}
		// A different authoritative montage arrived: drop the prediction and follow the server.
		PredictedMontageInfo = FSigilPredictedMontageInfo();
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

	ApplyRootTranslationScale(ReplicatedMontageInfo.RootTranslationScale);
	AnimInstance->Montage_Play(MontageToPlay, PlayRate, EMontagePlayReturnType::MontageLength, MontageTime);
}

bool USigilCombatSystemComponent::PlayPredictedMontage(const FSigilPlayMontageRequest& Request)
{
	if (!IsMontageRequestValid(this, Request))
	{
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetCharacterMeshComponent();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return false;
	}

	ApplyRootTranslationScale(Request.RootTranslationScale);
	const float Duration = AnimInstance->Montage_Play(Request.AnimMontage, Request.PlayRate, EMontagePlayReturnType::MontageLength, Request.StartTimeSeconds);
	if (Duration <= 0.0f)
	{
		// Nothing is playing: do not record a prediction, or a later authoritative replication of the same asset would be swallowed.
		return false;
	}
	if (Request.StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(Request.StartSectionName, Request.AnimMontage);
	}

	PredictedMontageInfo.AnimMontage = Request.AnimMontage;
	PredictedMontageInfo.PlayRate = Request.PlayRate;
	PredictedMontageInfo.TriggeredTime = GetSigilServerWorldTime(GetWorld());
	PredictedMontageInfo.StartSectionName = Request.StartSectionName;
	PredictedMontageInfo.RequestId = Request.RequestId;
	return true;
}

void USigilCombatSystemComponent::CancelPredictedMontage(int32 RequestId)
{
	if (RequestId == 0 || PredictedMontageInfo.RequestId != RequestId)
	{
		return;
	}
	if (USkeletalMeshComponent* MeshComponent = GetCharacterMeshComponent())
	{
		if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
		{
			if (PredictedMontageInfo.AnimMontage && AnimInstance->Montage_IsPlaying(PredictedMontageInfo.AnimMontage))
			{
				AnimInstance->Montage_Stop(PredictedMontageInfo.AnimMontage->BlendOut.GetBlendTime(), PredictedMontageInfo.AnimMontage);
			}
		}
	}
	PredictedMontageInfo = FSigilPredictedMontageInfo();
}

void USigilCombatSystemComponent::ApplyRootTranslationScale(float Scale) const
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->SetAnimRootMotionTranslationScale(FMath::Max(Scale, 0.0f));
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
