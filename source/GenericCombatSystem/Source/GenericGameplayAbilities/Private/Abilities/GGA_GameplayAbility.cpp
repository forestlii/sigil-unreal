// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Abilities/GGA_GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Abilities/GGA_AbilityCost.h"
#include "Stats/Stats2.h"
#include "GGA_AbilitySystemComponent.h"
#include "GGA_GameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GGA_LogChannels.h"
#include "Misc/DataValidation.h"
#include "Utilities/GGA_GameplayEffectContainerFunctionLibrary.h"

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)                                                                              \
	{                                                                                                                                                    \
		if (!ensure(IsInstantiated()))                                                                                                                   \
		{                                                                                                                                                \
			ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName()); \
			return ReturnValue;                                                                                                                          \
		}                                                                                                                                                \
	}

UGGA_GameplayAbility::UGGA_GameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	bServerRespectsRemoteAbilityCancellation = false;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationGroup = EGGA_AbilityActivationGroup::Independent;

	bReplicateInputDirectly = false;

	bEnableTick = false;
}

void UGGA_GameplayAbility::Tick(float DeltaTime)
{
	AbilityTick(DeltaTime);
}

TStatId UGGA_GameplayAbility::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UGGA_GameplayAbility, STATGROUP_GameplayAbility);
}

bool UGGA_GameplayAbility::IsTickable() const
{
	return IsInstantiated() && bEnableTick && GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor && IsActive();
}

void UGGA_GameplayAbility::AbilityTick_Implementation(float DeltaTime)
{
}

AController* UGGA_GameplayAbility::GetControllerFromActorInfo() const
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

void UGGA_GameplayAbility::SetActivationGroup(EGGA_AbilityActivationGroup NewGroup)
{
	ActivationGroup = NewGroup;
}

void UGGA_GameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
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
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && GetAssetTags().HasTagExact(GGA_AbilityTraitTags::ActivationOnSpawn))
#else
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && AbilityTags.HasTagExact(GGA_AbilityTraitTags::ActivationOnSpawn))
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

void UGGA_GameplayAbility::HandleActivationFailed(const FGameplayTagContainer& FailedReason) const
{
	OnActivationFailed(FailedReason);
}

bool UGGA_GameplayAbility::HasEffectContainer(FGameplayTag ContainerTag)
{
	return EffectContainerMap.Contains(ContainerTag);
}

FGGA_GameplayEffectContainerSpec UGGA_GameplayAbility::MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	FGGA_GameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

	if (FoundContainer)
	{
		return UGGA_GameplayEffectContainerFunctionLibrary::MakeEffectContainerSpec(*FoundContainer, EventData, OverrideGameplayLevel, this);
	}
	return FGGA_GameplayEffectContainerSpec();
}

TArray<FActiveGameplayEffectHandle> UGGA_GameplayAbility::ApplyEffectContainer(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	FGGA_GameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

	if (FoundContainer)
	{
		const FGGA_GameplayEffectContainerSpec Spec = UGGA_GameplayEffectContainerFunctionLibrary::MakeEffectContainerSpec(*FoundContainer, EventData, OverrideGameplayLevel, this);
		return UGGA_GameplayEffectContainerFunctionLibrary::ApplyEffectContainerSpec(this, Spec);
	}
	return TArray<FActiveGameplayEffectHandle>();
}

void UGGA_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGGA_GameplayAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
	UAbilitySystemComponent* Comp = ActorInfo->AbilitySystemComponent.Get();

	for (const FGGA_GameplayTagCount& TagCount : ActivationOwnedLooseTags)
	{
		Comp->AddLooseGameplayTag(TagCount.Tag, TagCount.Count);
	}
}

void UGGA_GameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (UAbilitySystemComponent* Comp = ActorInfo->AbilitySystemComponent.Get())
		{
			for (const FGGA_GameplayTagCount& TagCount : ActivationOwnedLooseTags)
			{
				Comp->RemoveLooseGameplayTag(TagCount.Tag, TagCount.Count);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGGA_GameplayAbility::OnActivationFailed_Implementation(const FGameplayTagContainer& FailedReason) const
{
}

bool UGGA_GameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                              const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	//@TODO Possibly remove after setting up tag relationships
	UGGA_AbilitySystemComponent* GASC = CastChecked<UGGA_AbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (GASC->IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(GGA_AbilityActivateFailTags::ActivationGroup);
		}
		return false;
	}

	return true;
}

void UGGA_GameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	// The ability can not block canceling if it's replaceable.
	if (!bCanBeCanceled && (ActivationGroup == EGGA_AbilityActivationGroup::Exclusive_Replaceable))
	{
		UE_LOG(LogGGA_Ability, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UGGA_GameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnGiveAbility();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UGGA_GameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnRemoveAbility();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UGGA_GameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	K2_OnAvatarSet();
}

bool UGGA_GameplayAbility::ShouldActivateAbility(ENetRole Role) const
{
	return K2_ShouldActivateAbility(Role) && Super::ShouldActivateAbility(Role);
	// Don't violate security policy if we're not the server
}

bool UGGA_GameplayAbility::K2_ShouldActivateAbility_Implementation(ENetRole Role) const
{
	return true;
}

void UGGA_GameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	K2_OnInputPressed(Handle, *ActorInfo, ActivationInfo);
}

void UGGA_GameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	K2_OnInputReleased(Handle, *ActorInfo, ActivationInfo);
}

bool UGGA_GameplayAbility::IsInputPressed() const
{
	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	return Spec && Spec->InputPressed;
}

bool UGGA_GameplayAbility::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately)
{
	UGGA_AbilitySystemComponent* ASC = Cast<UGGA_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (ASC)
	{
		return ASC->BatchRPCTryActivateAbility(InAbilityHandle, EndAbilityImmediately);
	}
	return false;
}

void UGGA_GameplayAbility::ExternalEndAbility()
{
	check(CurrentActorInfo);

	bool bReplicateEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGGA_GameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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
	for (TObjectPtr<UGGA_AbilityCost> AdditionalCost : AdditionalCosts)
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

bool UGGA_GameplayAbility::K2_OnCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo& ActorInfo) const
{
	return true;
}

void UGGA_GameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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
			if (UGGA_AbilitySystemComponent* ASC = Cast<UGGA_AbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
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
	for (TObjectPtr<UGGA_AbilityCost> AdditionalCost : AdditionalCosts)
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

UGameplayEffect* UGGA_GameplayAbility::GetCostGameplayEffect() const
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

TSubclassOf<UGameplayEffect> UGGA_GameplayAbility::K2_GetCostGameplayEffect_Implementation() const
{
	return CostGameplayEffectClass;
}

void UGGA_GameplayAbility::K2_OnApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo& ActorInfo,
                                                         const FGameplayAbilityActivationInfo ActivationInfo) const
{
}

void UGGA_GameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);
}

bool UGGA_GameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
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

	const UGGA_AbilitySystemComponent* GASC = Cast<UGGA_AbilitySystemComponent>(&AbilitySystemComponent);
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

void UGGA_GameplayAbility::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetData)
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

#if WITH_EDITOR
EDataValidationResult UGGA_GameplayAbility::IsDataValid(FDataValidationContext& Context) const
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

void UGGA_GameplayAbility::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}
#endif
