// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GGA_AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayCueManager.h"
#include "GGA_AbilitySystemGlobals.h"
#include "GGA_AbilityTagRelationshipMapping.h"
#include "GGA_GlobalAbilitySystem.h"
#include "GGA_LogChannels.h"
#include "Abilities/GGA_GameplayAbilityInterface.h"
#include "GameFramework/Pawn.h"
#include "Runtime/Launch/Resources/Version.h"


#pragma region Initialization

UGGA_AbilitySystemComponent::UGGA_AbilitySystemComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	AbilitySystemReplicationMode = EGameplayEffectReplicationMode::Mixed;

	bReplicateUsingRegisteredSubObjectList = true;

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UGGA_AbilitySystemComponent::InitializeAbilitySystem(AActor* InOwnerActor, AActor* InAvatarActor)
{
	check(InOwnerActor);
	check(InAvatarActor);

	if (!bAbilitySystemInitialized)
	{
		FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
		const bool AvatarChanged = InAvatarActor && (InAvatarActor != ActorInfo->AvatarActor);
		InitAbilityActorInfo(InOwnerActor, InAvatarActor);
		InitializeAbilitySets(InOwnerActor, InAvatarActor);
		if (AttributeSetInitializeGroupName.IsValid())
		{
			InitializeAttributes(AttributeSetInitializeGroupName, AttributeSetInitializeLevel, true);
		}
		bAbilitySystemInitialized = true;
		OnAbilitySystemInitialized.Broadcast();
	}
}

void UGGA_AbilitySystemComponent::UninitializeAbilitySystem()
{
	if (bAbilitySystemInitialized)
	{
		bAbilitySystemInitialized = false;
		OnAbilitySystemUninitialized.Broadcast();
	}
}


void UGGA_AbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool AvatarChanged = InAvatarActor && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		return;
	}

	if (AvatarChanged)
	{
		RegisterToGlobalAbilitySystem();

		ABILITYLIST_SCOPE_LOCK();
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (IGGA_GameplayAbilityInterface* AbilityCDO = Cast<IGGA_GameplayAbilityInterface>(AbilitySpec.Ability))
			{
				AbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
			}
		}
	}
}

void UGGA_AbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterToGlobalAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void UGGA_AbilitySystemComponent::InitializeComponent()
{
	SetReplicationMode(AbilitySystemReplicationMode);
	Super::InitializeComponent();
}

void UGGA_AbilitySystemComponent::InitializeAbilitySets(AActor* InOwnerActor, AActor* InAvatarActor)
{
	if (GetNetMode() != NM_Client)
	{
		for (int32 i = DefaultAbilitySet_GrantedHandles.Num() - 1; i >= 0; i--)
		{
			DefaultAbilitySet_GrantedHandles[i].TakeFromAbilitySystem(this);
		}
		DefaultAbilitySet_GrantedHandles.Empty();

		for (TObjectPtr<const UGGA_AbilitySet> AbilitySet : DefaultAbilitySets)
		{
			if (!AbilitySet)
				continue;
			AbilitySet->GiveToAbilitySystem(this, /*inout*/ &DefaultAbilitySet_GrantedHandles.AddDefaulted_GetRef(), this);
		}
	}
}

void UGGA_AbilitySystemComponent::InitializeAttributes(FGGA_AttributeGroupName GroupName, int32 Level, bool bInitialInit)
{
	if (const UGGA_AbilitySystemGlobals* Globals = Cast<UGGA_AbilitySystemGlobals>(UGGA_AbilitySystemGlobals::GetAbilitySystemGlobals()))
	{
		Globals->InitAttributeSetDefaults(this, GroupName, Level, bInitialInit);
	}else
	{
		UE_LOG(LogGGA_AbilitySystem,Warning,TEXT("Failed to InitializeAttributes as your project is not configured to use GGA_AbilitySystemGlobals(or derived class)."));
	}
}

void UGGA_AbilitySystemComponent::SendGameplayEventToActor_Replicated(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	if (IsValid(Actor) && EventTag.IsValid())
	{
		if (Actor->HasAuthority())
		{
			MulticastSendGameplayEventToActor(Actor, EventTag, Payload);
		}
		else
		{
			ServerSendGameplayEventToActor(Actor, EventTag, Payload);
		}
	}
}

void UGGA_AbilitySystemComponent::ServerSendGameplayEventToActor_Implementation(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	MulticastSendGameplayEventToActor(Actor, EventTag, Payload);
}

bool UGGA_AbilitySystemComponent::ServerSendGameplayEventToActor_Validate(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	return true;
}

void UGGA_AbilitySystemComponent::MulticastSendGameplayEventToActor_Implementation(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Actor, EventTag, Payload);
}

void UGGA_AbilitySystemComponent::PostInitProperties()
{
	Super::PostInitProperties();
	ReplicationMode = AbilitySystemReplicationMode;
}

void UGGA_AbilitySystemComponent::RegisterToGlobalAbilitySystem()
{
	if (bRegisteredToGlobalAbilitySystem)
		return;
	// Register with the global system once we actually have a pawn avatar. We wait until this time since some globally-applied effects may require an avatar.
	if (UGGA_GlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UGGA_GlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->RegisterASC(this);
		bRegisteredToGlobalAbilitySystem = true;
	}
}

void UGGA_AbilitySystemComponent::UnregisterToGlobalAbilitySystem()
{
	if (!bRegisteredToGlobalAbilitySystem)
		return;
	if (UGGA_GlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UGGA_GlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->UnregisterASC(this);
		bRegisteredToGlobalAbilitySystem = false;
	}
}

#pragma endregion

#pragma region AbilitiesActivation

bool UGGA_AbilitySystemComponent::IsActivationGroupBlocked(EGGA_AbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EGGA_AbilityActivationGroup::Independent:
		// Independent abilities are never blocked.
		bBlocked = false;
		break;

	case EGGA_AbilityActivationGroup::Exclusive_Replaceable:
	case EGGA_AbilityActivationGroup::Exclusive_Blocking:
		// Exclusive abilities can activate if nothing is blocking.
		bBlocked = (ActivationGroupCounts[(uint8)EGGA_AbilityActivationGroup::Exclusive_Blocking] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

void UGGA_AbilitySystemComponent::AddAbilityToActivationGroup(EGGA_AbilityActivationGroup Group, UGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case EGGA_AbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case EGGA_AbilityActivationGroup::Exclusive_Replaceable:
	case EGGA_AbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(EGGA_AbilityActivationGroup::Exclusive_Replaceable, Ability, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: In valid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EGGA_AbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)EGGA_AbilityActivationGroup::Exclusive_Blocking];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogGGA_AbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UGGA_AbilitySystemComponent::RemoveAbilityFromActivationGroup(EGGA_AbilityActivationGroup Group, UGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

bool UGGA_AbilitySystemComponent::CanChangeActivationGroup(EGGA_AbilityActivationGroup NewGroup, UGameplayAbility* Ability) const
{
	if (Ability == nullptr || !Ability->IsInstantiated() || !Ability->IsActive())
	{
		return false;
	}

	IGGA_GameplayAbilityInterface* AbilityInterface = Cast<IGGA_GameplayAbilityInterface>(Ability);
	if (AbilityInterface == nullptr)
	{
		return false;
	}

	if (AbilityInterface->GetActivationGroup() == NewGroup)
	{
		return true;
	}


	if ((AbilityInterface->GetActivationGroup() != EGGA_AbilityActivationGroup::Exclusive_Blocking) && IsActivationGroupBlocked(NewGroup))
	{
		// This ability can't change groups if it's blocked (unless it is the one doing the blocking).
		return false;
	}

	if ((NewGroup == EGGA_AbilityActivationGroup::Exclusive_Replaceable) && !Ability->CanBeCanceled())
	{
		// This ability can't become replaceable if it can't be canceled.
		return false;
	}

	return true;
}

bool UGGA_AbilitySystemComponent::ChangeActivationGroup(EGGA_AbilityActivationGroup NewGroup, UGameplayAbility* Ability)
{
	if (!CanChangeActivationGroup(NewGroup, Ability))
	{
		return false;
	}

	IGGA_GameplayAbilityInterface* AbilityInterface = Cast<IGGA_GameplayAbilityInterface>(Ability);
	if (AbilityInterface == nullptr)
	{
		return false;
	}

	if (AbilityInterface->GetActivationGroup() != NewGroup)
	{
		RemoveAbilityFromActivationGroup(AbilityInterface->GetActivationGroup(), Ability);
		AddAbilityToActivationGroup(NewGroup, Ability);
		AbilityInterface->SetActivationGroup(NewGroup);
	}

	return true;
}

void UGGA_AbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (IGGA_GameplayAbilityInterface* AbilityInterface = Cast<IGGA_GameplayAbilityInterface>(Ability))
	{
		AddAbilityToActivationGroup(AbilityInterface->GetActivationGroup(), Ability);
	}

	OnAbilityActivated.Broadcast(Handle, Ability);
}

void UGGA_AbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityActivationFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityActivationFailed(Ability, FailureReason);
}

void UGGA_AbilitySystemComponent::ClientNotifyAbilityActivationFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityActivationFailed(Ability, FailureReason);
}

void UGGA_AbilitySystemComponent::HandleAbilityActivationFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	OnAbilityActivationFailed.Broadcast(Ability, FailureReason);

	if (const IGGA_GameplayAbilityInterface* AbilityInterface = Cast<const IGGA_GameplayAbilityInterface>(Ability))
	{
		AbilityInterface->HandleActivationFailed(FailureReason);
	}
}

#pragma endregion

#pragma region AbilityCancellation

void UGGA_AbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability == nullptr || !AbilitySpec.IsActive())
		{
			continue;
		}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION <5
		if (AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
#endif
		{
			// Cancel all the spawned instances, not the CDO.
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				if (ShouldCancelFunc(AbilityInstance, AbilitySpec.Handle))
				{
					if (AbilityInstance->CanBeCanceled())
					{
						AbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), AbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						UE_LOG(LogGGA_AbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *AbilityInstance->GetName());
					}
				}
			}
		}
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION <5
		else
		{
			// Cancel the non-instanced ability CDO.
			if (ShouldCancelFunc(AbilitySpec.Ability, AbilitySpec.Handle))
			{
				// Non-instanced abilities can always be canceled.
				check(AbilitySpec.Ability->CanBeCanceled());
				AbilitySpec.Ability->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
		}
#endif
	}
}

void UGGA_AbilitySystemComponent::CancelActivationGroupAbilities(EGGA_AbilityActivationGroup Group, UGameplayAbility* IgnoreAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
	{
		bool SameGroup = false;
		if (const IGGA_GameplayAbilityInterface* AbilityInterface = Cast<IGGA_GameplayAbilityInterface>(Ability))
		{
			SameGroup = AbilityInterface->GetActivationGroup() == Group;
		}
		return (SameGroup && (Ability != IgnoreAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UGGA_AbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (const IGGA_GameplayAbilityInterface* AbilityInterface = Cast<IGGA_GameplayAbilityInterface>(Ability))
	{
		RemoveAbilityFromActivationGroup(AbilityInterface->GetActivationGroup(), Ability);
	}

	AbilityEndedEvent.Broadcast(Handle, Ability, bWasCancelled);
}

void UGGA_AbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: Apply any special logic like blocking input or movement
}

#pragma endregion

#pragma region Abilities

bool UGGA_AbilitySystemComponent::GetCooldownRemainingForTags(FGameplayTagContainer CooldownTags, float& TimeRemaining, float& CooldownDuration)
{
	if (CooldownTags.Num() > 0)
	{
		TimeRemaining = 0.f;
		CooldownDuration = 0.f;

		FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
		TArray<TPair<float, float>> DurationAndTimeRemaining = GetActiveEffectsTimeRemainingAndDuration(Query);
		if (DurationAndTimeRemaining.Num() > 0)
		{
			int32 BestIdx = 0;
			float LongestTime = DurationAndTimeRemaining[0].Key;
			for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
			{
				if (DurationAndTimeRemaining[Idx].Key > LongestTime)
				{
					LongestTime = DurationAndTimeRemaining[Idx].Key;
					BestIdx = Idx;
				}
			}

			TimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
			CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;

			return true;
		}
	}
	return false;
}

bool UGGA_AbilitySystemComponent::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle,
                                                             bool EndAbilityImmediately)
{
	bool AbilityActivated = false;
	if (InAbilityHandle.IsValid())
	{
		FScopedServerAbilityRPCBatcher AbilityRpcBatching(this, InAbilityHandle);
		AbilityActivated = TryActivateAbility(InAbilityHandle, true);

		if (EndAbilityImmediately)
		{
			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(InAbilityHandle);
			if (AbilitySpec)
			{
				if (IGGA_GameplayAbilityInterface* AbilityInterface = Cast<IGGA_GameplayAbilityInterface>(AbilitySpec->GetPrimaryInstance()))
				{
					AbilityInterface->ExternalEndAbility();
				}
			}
		}

		return AbilityActivated;
	}

	return AbilityActivated;
}
#pragma endregion


#pragma region GameplayTags
void UGGA_AbilitySystemComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset(); // Fix for Version under 5.2
	TagContainer.AppendTags(GameplayTagCountContainer.GetExplicitGameplayTags());
}

FString UGGA_AbilitySystemComponent::GetOwnedGameplayTagsString()
{
	FString BlockedTagsStrings;
	for (auto Tag : GameplayTagCountContainer.GetExplicitGameplayTags())
	{
		BlockedTagsStrings.Append(FString::Printf(TEXT("%s (%d),\n"), *Tag.ToString(), GameplayTagCountContainer.GetTagCount(Tag)));
	}
	return BlockedTagsStrings;
}

void UGGA_AbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired,
                                                                         FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		FGameplayTagContainer ActorTags;
		GetOwnedGameplayTags(ActorTags);
		TagRelationshipMapping->GetRequiredAndBlockedActivationTagsV2(ActorTags, AbilityTags, &OutActivationRequired, &OutActivationBlocked);
		// TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

void UGGA_AbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
                                                                 const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		FGameplayTagContainer ActorTags;
		GetOwnedGameplayTags(ActorTags);
		// Use the mapping to expand the ability tags into block and cancel tag
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancelV2(ActorTags, AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
		// TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

	//@TODO: Apply any special logic like blocking input or movement
}

void UGGA_AbilitySystemComponent::SetTagRelationshipMapping(UGGA_AbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

#pragma endregion

#pragma region Attributes

FString UGGA_AbilitySystemComponent::GetOwnedGameplayAttributeSetString()
{
	FString AttributeSetString;
	TArray<FGameplayAttribute> Attributes;
	GetAllAttributes(Attributes);
	for (const auto& Attribute : Attributes)
	{
		AttributeSetString.Append(
			FString::Printf(TEXT("%s : %.2f \n"), *Attribute.GetName(), GetNumericAttribute(Attribute)));
	}
	return AttributeSetString;
}

#pragma endregion

#pragma region TargetData
void UGGA_AbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo,
                                                       FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}
#pragma endregion
