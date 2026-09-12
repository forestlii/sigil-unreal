// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Abilities/SigilGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Abilities/SigilAbilityCost.h"
#include "Abilities/SigilAbilitySourceInterface.h"
#include "Stats/Stats2.h"
#include "SigilAbilitySystemComponent.h"
#include "SigilGasTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "SigilGasLogChannels.h"
#include "Misc/DataValidation.h"
#include "Utilities/SigilGameplayEffectContainerFunctionLibrary.h"

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)                                                                              \
	{                                                                                                                                                    \
		if (!ensure(IsInstantiated()))                                                                                                                   \
		{                                                                                                                                                \
			ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName()); \
			return ReturnValue;                                                                                                                          \
		}                                                                                                                                                \
	}

USigilGameplayAbility::USigilGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	bServerRespectsRemoteAbilityCancellation = false;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationGroup = ESigilAbilityActivationGroup::Independent;

	bReplicateInputDirectly = false;

	bEnableTick = false;
	bRequireSourceObjectActive = false;
	CooldownDurationSetByCallerTag = SigilSetByCallerTags::CooldownDuration;
}

void USigilGameplayAbility::Tick(float DeltaTime)
{
	AbilityTick(DeltaTime);
}

TStatId USigilGameplayAbility::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USigilGameplayAbility, STATGROUP_GameplayAbility);
}

bool USigilGameplayAbility::IsTickable() const
{
	return IsInstantiated() && bEnableTick && GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor && IsActive();
}

void USigilGameplayAbility::AbilityTick_Implementation(float DeltaTime)
{
}

AController* USigilGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

void USigilGameplayAbility::SetActivationGroup(ESigilAbilityActivationGroup NewGroup)
{
	ActivationGroup = NewGroup;
}

void USigilGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS

#if ENGINE_MINOR_VERSION > 4
	// Fixing this up to use the instance activation, but this function should be deprecated as it cannot work with InstancedPerExecution
	UE_CLOG(Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution, LogAbilitySystem, Warning,
	        TEXT("%hs: %s is InstancedPerExecution. This is unreliable for Input as you may only interact with the latest spawned Instance"), __func__, *GetNameSafe(Spec.Ability));
	TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
	const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? Spec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
	const bool bIsPredicting = (ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);
#else
	const bool bIsPredicting = (Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);
#endif
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	// Try to activate if activation policy is on spawn.
#if ENGINE_MINOR_VERSION > 4
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && GetAssetTags().HasTagExact(SigilAbilityTraitTags::ActivationOnSpawn))
#else
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && AbilityTags.HasTagExact(SigilAbilityTraitTags::ActivationOnSpawn))
#endif
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void USigilGameplayAbility::HandleActivationFailed(const FGameplayTagContainer& FailedReason) const
{
	OnActivationFailed(FailedReason);
}

bool USigilGameplayAbility::HasEffectContainer(FGameplayTag ContainerTag)
{
	return EffectContainerMap.Contains(ContainerTag);
}

FSigilGameplayEffectContainerSpec USigilGameplayAbility::MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	FSigilGameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

	if (FoundContainer)
	{
		return USigilGameplayEffectContainerFunctionLibrary::MakeEffectContainerSpec(*FoundContainer, EventData, OverrideGameplayLevel, this);
	}
	return FSigilGameplayEffectContainerSpec();
}

TArray<FActiveGameplayEffectHandle> USigilGameplayAbility::ApplyEffectContainer(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	FSigilGameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

	if (FoundContainer)
	{
		const FSigilGameplayEffectContainerSpec Spec = USigilGameplayEffectContainerFunctionLibrary::MakeEffectContainerSpec(*FoundContainer, EventData, OverrideGameplayLevel, this);
		return USigilGameplayEffectContainerFunctionLibrary::ApplyEffectContainerSpec(this, Spec);
	}
	return TArray<FActiveGameplayEffectHandle>();
}

void USigilGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void USigilGameplayAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
	UAbilitySystemComponent* Comp = ActorInfo->AbilitySystemComponent.Get();

	for (const FSigilGameplayTagCount& TagCount : ActivationOwnedLooseTags)
	{
		Comp->AddLooseGameplayTag(TagCount.Tag, TagCount.Count);
	}
}

void USigilGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (UAbilitySystemComponent* Comp = ActorInfo->AbilitySystemComponent.Get())
		{
			for (const FSigilGameplayTagCount& TagCount : ActivationOwnedLooseTags)
			{
				Comp->RemoveLooseGameplayTag(TagCount.Tag, TagCount.Count);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USigilGameplayAbility::OnActivationFailed_Implementation(const FGameplayTagContainer& FailedReason) const
{
}

bool USigilGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                              const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	// Source gate (GASShooter's bSourceObjectMustEqualCurrentWeaponToActivate, resolved through an interface).
	if (bRequireSourceObjectActive && !IsAbilitySourceActive(GetSourceObject(Handle, ActorInfo)))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(SigilAbilityActivateFailTags::SourceObjectInactive);
		}
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	//@TODO Possibly remove after setting up tag relationships
	USigilAbilitySystemComponent* GASC = CastChecked<USigilAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (GASC->IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(SigilAbilityActivateFailTags::ActivationGroup);
		}
		return false;
	}

	return true;
}

const FGameplayTagContainer* USigilGameplayAbility::GetCooldownTags() const
{
	// GASDocumentation 4.5.15 (Copyright 2020 Dan Kestranek, MIT): union of the cooldown GE's tags and the ability's own cooldown tags.
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (CooldownTags.IsEmpty())
	{
		return ParentTags;
	}

	// TempCooldownTags lives on the CDO for non-instanced calls, so rebuild it every time in case CooldownTags changed.
	TempCooldownTags.Reset();
	if (ParentTags)
	{
		TempCooldownTags.AppendTags(*ParentTags);
		// The shared GE grants Sigil.Cooldown.Shared only to satisfy IsDataValid; matching on it would make every ability
		// that shares the effect block every other one (PR #4 review P1-E).
		TempCooldownTags.RemoveTag(SigilCooldownTags::SharedMarker);
	}
	TempCooldownTags.AppendTags(CooldownTags);
	return &TempCooldownTags;
}

void USigilGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CooldownGE)
	{
		return;
	}

	const float Level = GetAbilityLevel(Handle, ActorInfo);
	const float Duration = CooldownDuration.GetValueAtLevel(Level);
	const bool bWriteDuration = Duration > 0.f && CooldownDurationSetByCallerTag.IsValid();
	if (CooldownTags.IsEmpty() && !bWriteDuration)
	{
		// Nothing to inject: keep the engine behaviour unchanged.
		Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		return;
	}

	// GASDocumentation 4.5.15 technique 1 (Copyright 2020 Dan Kestranek, MIT): shared cooldown GE + SetByCaller duration.
	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CooldownGE->GetClass(), Level);
	if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
	{
		Spec->DynamicGrantedTags.AppendTags(CooldownTags);
		if (bWriteDuration)
		{
			Spec->SetSetByCallerMagnitude(CooldownDurationSetByCallerTag, Duration);
		}
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}

bool USigilGameplayAbility::IsAbilitySourceActive(const UObject* SourceObject)
{
	if (!IsValid(SourceObject) || !SourceObject->Implements<USigilAbilitySourceInterface>())
	{
		return false;
	}

	return ISigilAbilitySourceInterface::Execute_IsAbilitySourceActive(SourceObject);
}

void USigilGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	// The ability can not block canceling if it's replaceable.
	if (!bCanBeCanceled && (ActivationGroup == ESigilAbilityActivationGroup::Exclusive_Replaceable))
	{
		UE_LOG(LogSigilAbility, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void USigilGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnGiveAbility();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void USigilGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnRemoveAbility();

	// The ASC drops its side of the per-mesh bookkeeping when the ability ends; this drops ours when the spec goes away.
	CurrentAbilityMeshMontages.Reset();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void USigilGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// Meshes tracked here belonged to the previous avatar (the ASC resets its LocalMeshMontages on avatar change as well).
	CurrentAbilityMeshMontages.Reset();

	Super::OnAvatarSet(ActorInfo, Spec);
	K2_OnAvatarSet();

	// Passive abilities granted before the avatar existed get another chance here (GASShooter GSGameplayAbility::OnAvatarSet,
	// Copyright 2020 Dan Kestranek, MIT). TryActivateAbilityOnSpawn skips specs that are already active, so this cannot
	// double-activate when the ASC also retries from InitAbilityActorInfo.
	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

bool USigilGameplayAbility::ShouldActivateAbility(ENetRole Role) const
{
	return K2_ShouldActivateAbility(Role) && Super::ShouldActivateAbility(Role);
	// Don't violate security policy if we're not the server
}

bool USigilGameplayAbility::K2_ShouldActivateAbility_Implementation(ENetRole Role) const
{
	return true;
}

void USigilGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	K2_OnInputPressed(Handle, *ActorInfo, ActivationInfo);
}

void USigilGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	K2_OnInputReleased(Handle, *ActorInfo, ActivationInfo);
}

bool USigilGameplayAbility::IsInputPressed() const
{
	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	return Spec && Spec->InputPressed;
}

bool USigilGameplayAbility::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately)
{
	USigilAbilitySystemComponent* ASC = Cast<USigilAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (ASC)
	{
		return ASC->BatchRPCTryActivateAbility(InAbilityHandle, EndAbilityImmediately);
	}
	return false;
}

void USigilGameplayAbility::ExternalEndAbility()
{
	check(CurrentActorInfo);

	bool bReplicateEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool USigilGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                     OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	if (!K2_OnCheckCost(Handle, *ActorInfo))
	{
		return false;
	}
	for (TObjectPtr<USigilAbilityCost> AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, OptionalRelevantTags))
			{
				return false;
			}
		}
	}
	return true;
}

bool USigilGameplayAbility::K2_OnCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo& ActorInfo) const
{
	return true;
}

void USigilGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	check(ActorInfo);

	K2_OnApplyCost(Handle, *ActorInfo, ActivationInfo);

	// Used to determine if the ability actually hit a target (as some costs are only spent on successful attempts)
	auto DetermineIfAbilityHitTarget = [&]()
	{
		if (ActorInfo->IsNetAuthority())
		{
			if (USigilAbilitySystemComponent* ASC = Cast<USigilAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
			{
				FGameplayAbilityTargetDataHandle TargetData;
				ASC->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
				for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
				{
					if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
					{
						return true;
					}
				}
			}
		}

		return false;
	};

	// Pay any additional costs
	bool bAbilityHitTarget = false;
	bool bHasDeterminedIfAbilityHitTarget = false;
	for (TObjectPtr<USigilAbilityCost> AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (AdditionalCost->ShouldOnlyApplyCostOnHit())
			{
				if (!bHasDeterminedIfAbilityHitTarget)
				{
					bAbilityHitTarget = DetermineIfAbilityHitTarget();
					bHasDeterminedIfAbilityHitTarget = true;
				}

				if (!bAbilityHitTarget)
				{
					continue;
				}
			}

			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}

UGameplayEffect* USigilGameplayAbility::GetCostGameplayEffect() const
{
	if (TSubclassOf<UGameplayEffect> GE = K2_GetCostGameplayEffect())
	{
		if (GE)
		{
			return GE->GetDefaultObject<UGameplayEffect>();
		}
		return nullptr;
	}
	return nullptr;
}

TSubclassOf<UGameplayEffect> USigilGameplayAbility::K2_GetCostGameplayEffect_Implementation() const
{
	return CostGameplayEffectClass;
}

void USigilGameplayAbility::K2_OnApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo& ActorInfo,
                                                         const FGameplayAbilityActivationInfo ActivationInfo) const
{
}

void USigilGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);
}

bool USigilGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                                             FGameplayTagContainer* OptionalRelevantTags) const
{
	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	FGameplayTagContainer TempAbilityTags = FGameplayTagContainer::EmptyContainer;

#if ENGINE_MINOR_VERSION > 4
	TempAbilityTags = GetAssetTags();
#else
	TempAbilityTags = AbilityTags;
#endif

	// Check if any of this ability's tags are currently blocked
	if (AbilitySystemComponent.AreAbilityTagsBlocked(TempAbilityTags))
	{
		bBlocked = true;
	}

	const USigilAbilitySystemComponent* GASC = Cast<USigilAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// Expand our ability tags to add additional required/blocked tags
	if (GASC)
	{
		GASC->GetAdditionalActivationTagRequirements(TempAbilityTags, AllRequiredTags, AllBlockedTags);
	}

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

void USigilGameplayAbility::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (IsPredictingClient())
	{
		UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
		check(ASC);

		// Create new prediction window for next operation. 为接下来的操作新增一个pk
		FScopedPredictionWindow(ASC, true);

		FGameplayTag ApplicationTag;
		// tell server about it.   告诉服务器设置TargetData,传入技能的uid和激活id,Data和本次操作的id
		CurrentActorInfo->AbilitySystemComponent->CallServerSetReplicatedTargetData(
			CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(),
			TargetData, ApplicationTag, ASC->ScopedPredictionKey);
	}
}

#pragma region MeshMontage

bool USigilGameplayAbility::IsAvatarMainMesh(const USkeletalMeshComponent* InMesh) const
{
	return InMesh && CurrentActorInfo && CurrentActorInfo->SkeletalMeshComponent.Get() == InMesh;
}

UAnimMontage* USigilGameplayAbility::GetCurrentMontageForMesh(const USkeletalMeshComponent* InMesh) const
{
	if (!InMesh)
	{
		return nullptr;
	}

	if (IsAvatarMainMesh(InMesh))
	{
		return GetCurrentMontage();
	}

	const FSigilAbilityMeshMontage* Entry = CurrentAbilityMeshMontages.FindByPredicate([InMesh](const FSigilAbilityMeshMontage& Candidate) { return Candidate.Mesh == InMesh; });
	return Entry ? Entry->Montage.Get() : nullptr;
}

void USigilGameplayAbility::SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* InCurrentMontage)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(SetCurrentMontageForMesh, );

	if (!InMesh)
	{
		return;
	}

	if (IsAvatarMainMesh(InMesh))
	{
		SetCurrentMontage(InCurrentMontage);
		return;
	}

	// Null clears: drop the entry instead of keeping a (Mesh, null) pair around (PR #4 review P1-C).
	if (!InCurrentMontage)
	{
		CurrentAbilityMeshMontages.RemoveAll([InMesh](const FSigilAbilityMeshMontage& Candidate) { return Candidate.Mesh == InMesh || !IsValid(Candidate.Mesh); });
		return;
	}

	// GASShooter looked the entry up by value and mutated a copy, so a second Set for the same mesh never took effect.
	if (FSigilAbilityMeshMontage* Entry = CurrentAbilityMeshMontages.FindByPredicate([InMesh](const FSigilAbilityMeshMontage& Candidate) { return Candidate.Mesh == InMesh; }))
	{
		Entry->Montage = InCurrentMontage;
	}
	else
	{
		FSigilAbilityMeshMontage& NewEntry = CurrentAbilityMeshMontages.AddDefaulted_GetRef();
		NewEntry.Mesh = InMesh;
		NewEntry.Montage = InCurrentMontage;
	}
}

void USigilGameplayAbility::MontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName)
{
	check(CurrentActorInfo);

	USigilAbilitySystemComponent* const ASC = Cast<USigilAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
	if (ASC && ASC->GetAnimatingAbilityForMesh(InMesh) == this)
	{
		ASC->CurrentMontageJumpToSectionForMesh(InMesh, SectionName);
	}
}

void USigilGameplayAbility::MontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName)
{
	check(CurrentActorInfo);

	USigilAbilitySystemComponent* const ASC = Cast<USigilAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
	if (ASC && ASC->GetAnimatingAbilityForMesh(InMesh) == this)
	{
		ASC->CurrentMontageSetNextSectionNameForMesh(InMesh, FromSectionName, ToSectionName);
	}
}

void USigilGameplayAbility::MontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
	check(CurrentActorInfo);

	USigilAbilitySystemComponent* const ASC = Cast<USigilAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
	if (ASC && ASC->GetAnimatingAbilityForMesh(InMesh) == this)
	{
		ASC->CurrentMontageStopForMesh(InMesh, OverrideBlendOutTime);
	}
}

void USigilGameplayAbility::MontageStopForAllMeshes(float OverrideBlendOutTime)
{
	check(CurrentActorInfo);

	USigilAbilitySystemComponent* const ASC = Cast<USigilAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
	if (!ASC)
	{
		return;
	}

	if (ASC->GetAnimatingAbility() == this)
	{
		ASC->CurrentMontageStop(OverrideBlendOutTime);
	}

	// Copy: stopping can re-enter SetCurrentMontageForMesh and mutate the array.
	TArray<TObjectPtr<USkeletalMeshComponent>> Meshes;
	for (const FSigilAbilityMeshMontage& Entry : CurrentAbilityMeshMontages)
	{
		Meshes.Add(Entry.Mesh);
	}
	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		if (ASC->GetAnimatingAbilityForMesh(Mesh) == this)
		{
			ASC->CurrentMontageStopForMesh(Mesh, OverrideBlendOutTime);
		}
	}
}

#pragma endregion

#if WITH_EDITOR
EDataValidationResult USigilGameplayAbility::IsDataValid(FDataValidationContext& Context) const
{
	if (bReplicateInputDirectly == true)
	{
		Context.AddError(FText::FromString(TEXT("bReplicateInputDirectly is not allow to be true")));
		return EDataValidationResult::Invalid;
	}
	if (bServerRespectsRemoteAbilityCancellation == true)
	{
		Context.AddError(FText::FromString(TEXT("bServerRespectsRemoteAbilityCancellation is not allow to be true.")));
		return EDataValidationResult::Invalid;
	}

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (InstancingPolicy == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		Context.AddError(FText::FromString(TEXT("NonInstanced ability is deprecated since UE5.5, Use InstancedPerActor as the default to avoid confusing corner cases")));
		return EDataValidationResult::Invalid;
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	// if (ReplicationPolicy == EGameplayAbilityReplicationPolicy::Type::ReplicateYes)
	// {
	// 	Context.AddError(FText::FromString(TEXT("ReplicationPolicy->ReplicateYes is not acceptable, Pelease use other option!")));
	// 	return EDataValidationResult::Invalid;
	// }

	// if (!AbilityTriggers.IsEmpty() && NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::Type::ServerInitiated)
	// {
	// 	ValidationErrors.Add(FText::FromString(TEXT("Ability with triggers doesn't work with ServerInitiated Net Execution Policy!")));
	// 	return EDataValidationResult::Invalid;
	// }

	// if (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::Type::ServerInitiated)
	// {
	// 	ValidationErrors.Add(FText::FromString(TEXT("NetExecutionPolicy->ServerInitiated is not acceptable, Pelease use other option!")));
	// 	return EDataValidationResult::Invalid;
	// }

	if (bHasBlueprintActivateFromEvent && bHasBlueprintActivate && !AbilityTriggers.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("ActivateAbilityFromEvent will not run! Please remove ActivateAbility node!")));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}

#include "UObject/ObjectSaveContext.h"

void USigilGameplayAbility::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}
#endif
