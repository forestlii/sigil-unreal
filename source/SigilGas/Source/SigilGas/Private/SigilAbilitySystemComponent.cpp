// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayCueManager.h"
#include "SigilAbilitySystemGlobals.h"
#include "SigilAbilityTagRelationshipMapping.h"
#include "SigilGlobalAbilitySystem.h"
#include "SigilGasLogChannels.h"
#include "Abilities/SigilGameplayAbility.h"
#include "Abilities/SigilGameplayAbilityInterface.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Runtime/Launch/Resources/Version.h"


#pragma region Initialization

USigilAbilitySystemComponent::USigilAbilitySystemComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	AbilitySystemReplicationMode = EGameplayEffectReplicationMode::Mixed;

	bReplicateUsingRegisteredSubObjectList = true;

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void USigilAbilitySystemComponent::InitializeAbilitySystem(AActor* InOwnerActor, AActor* InAvatarActor)
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

void USigilAbilitySystemComponent::UninitializeAbilitySystem()
{
	if (bAbilitySystemInitialized)
	{
		bAbilitySystemInitialized = false;
		OnAbilitySystemUninitialized.Broadcast();
	}
}


void USigilAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
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
		// Secondary-mesh montage bookkeeping belongs to the previous avatar's meshes.
		LocalMeshMontages.Reset();

		RegisterToGlobalAbilitySystem();

		ABILITYLIST_SCOPE_LOCK();
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (ISigilGameplayAbilityInterface* AbilityCDO = Cast<ISigilGameplayAbilityInterface>(AbilitySpec.Ability))
			{
				AbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
			}
		}
	}
}

void USigilAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterToGlobalAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void USigilAbilitySystemComponent::InitializeComponent()
{
	SetReplicationMode(AbilitySystemReplicationMode);
	Super::InitializeComponent();
}

void USigilAbilitySystemComponent::InitializeAbilitySets(AActor* InOwnerActor, AActor* InAvatarActor)
{
	if (GetNetMode() != NM_Client)
	{
		for (int32 i = DefaultAbilitySet_GrantedHandles.Num() - 1; i >= 0; i--)
		{
			DefaultAbilitySet_GrantedHandles[i].TakeFromAbilitySystem(this);
		}
		DefaultAbilitySet_GrantedHandles.Empty();

		for (TObjectPtr<const USigilAbilitySet> AbilitySet : DefaultAbilitySets)
		{
			if (!AbilitySet)
				continue;
			AbilitySet->GiveToAbilitySystem(this, /*inout*/ &DefaultAbilitySet_GrantedHandles.AddDefaulted_GetRef(), this);
		}
	}
}

void USigilAbilitySystemComponent::InitializeAttributes(FSigilAttributeGroupName GroupName, int32 Level, bool bInitialInit)
{
	if (const USigilAbilitySystemGlobals* Globals = Cast<USigilAbilitySystemGlobals>(USigilAbilitySystemGlobals::GetAbilitySystemGlobals()))
	{
		Globals->InitAttributeSetDefaults(this, GroupName, Level, bInitialInit);
	}else
	{
		UE_LOG(LogSigilAbilitySystem,Warning,TEXT("Failed to InitializeAttributes as your project is not configured to use SigilAbilitySystemGlobals(or derived class)."));
	}
}

void USigilAbilitySystemComponent::SendGameplayEventToActor_Replicated(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
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

void USigilAbilitySystemComponent::ServerSendGameplayEventToActor_Implementation(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	MulticastSendGameplayEventToActor(Actor, EventTag, Payload);
}

bool USigilAbilitySystemComponent::ServerSendGameplayEventToActor_Validate(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	return true;
}

void USigilAbilitySystemComponent::MulticastSendGameplayEventToActor_Implementation(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Actor, EventTag, Payload);
}

void USigilAbilitySystemComponent::PostInitProperties()
{
	Super::PostInitProperties();
	ReplicationMode = AbilitySystemReplicationMode;
}

void USigilAbilitySystemComponent::RegisterToGlobalAbilitySystem()
{
	if (bRegisteredToGlobalAbilitySystem)
		return;
	// Register with the global system once we actually have a pawn avatar. We wait until this time since some globally-applied effects may require an avatar.
	if (USigilGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<USigilGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->RegisterASC(this);
		bRegisteredToGlobalAbilitySystem = true;
	}
}

void USigilAbilitySystemComponent::UnregisterToGlobalAbilitySystem()
{
	if (!bRegisteredToGlobalAbilitySystem)
		return;
	if (USigilGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<USigilGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->UnregisterASC(this);
		bRegisteredToGlobalAbilitySystem = false;
	}
}

#pragma endregion

#pragma region AbilitiesActivation

bool USigilAbilitySystemComponent::IsActivationGroupBlocked(ESigilAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case ESigilAbilityActivationGroup::Independent:
		// Independent abilities are never blocked.
		bBlocked = false;
		break;

	case ESigilAbilityActivationGroup::Exclusive_Replaceable:
	case ESigilAbilityActivationGroup::Exclusive_Blocking:
		// Exclusive abilities can activate if nothing is blocking.
		bBlocked = (ActivationGroupCounts[(uint8)ESigilAbilityActivationGroup::Exclusive_Blocking] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

void USigilAbilitySystemComponent::AddAbilityToActivationGroup(ESigilAbilityActivationGroup Group, UGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case ESigilAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case ESigilAbilityActivationGroup::Exclusive_Replaceable:
	case ESigilAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(ESigilAbilityActivationGroup::Exclusive_Replaceable, Ability, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: In valid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)ESigilAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)ESigilAbilityActivationGroup::Exclusive_Blocking];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogSigilAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void USigilAbilitySystemComponent::RemoveAbilityFromActivationGroup(ESigilAbilityActivationGroup Group, UGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

bool USigilAbilitySystemComponent::CanChangeActivationGroup(ESigilAbilityActivationGroup NewGroup, UGameplayAbility* Ability) const
{
	if (Ability == nullptr || !Ability->IsInstantiated() || !Ability->IsActive())
	{
		return false;
	}

	ISigilGameplayAbilityInterface* AbilityInterface = Cast<ISigilGameplayAbilityInterface>(Ability);
	if (AbilityInterface == nullptr)
	{
		return false;
	}

	if (AbilityInterface->GetActivationGroup() == NewGroup)
	{
		return true;
	}


	if ((AbilityInterface->GetActivationGroup() != ESigilAbilityActivationGroup::Exclusive_Blocking) && IsActivationGroupBlocked(NewGroup))
	{
		// This ability can't change groups if it's blocked (unless it is the one doing the blocking).
		return false;
	}

	if ((NewGroup == ESigilAbilityActivationGroup::Exclusive_Replaceable) && !Ability->CanBeCanceled())
	{
		// This ability can't become replaceable if it can't be canceled.
		return false;
	}

	return true;
}

bool USigilAbilitySystemComponent::ChangeActivationGroup(ESigilAbilityActivationGroup NewGroup, UGameplayAbility* Ability)
{
	if (!CanChangeActivationGroup(NewGroup, Ability))
	{
		return false;
	}

	ISigilGameplayAbilityInterface* AbilityInterface = Cast<ISigilGameplayAbilityInterface>(Ability);
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

void USigilAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (ISigilGameplayAbilityInterface* AbilityInterface = Cast<ISigilGameplayAbilityInterface>(Ability))
	{
		AddAbilityToActivationGroup(AbilityInterface->GetActivationGroup(), Ability);
	}

	OnAbilityActivated.Broadcast(Handle, Ability);
}

void USigilAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
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

void USigilAbilitySystemComponent::ClientNotifyAbilityActivationFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityActivationFailed(Ability, FailureReason);
}

void USigilAbilitySystemComponent::HandleAbilityActivationFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	OnAbilityActivationFailed.Broadcast(Ability, FailureReason);

	if (const ISigilGameplayAbilityInterface* AbilityInterface = Cast<const ISigilGameplayAbilityInterface>(Ability))
	{
		AbilityInterface->HandleActivationFailed(FailureReason);
	}
}

#pragma endregion

#pragma region AbilityCancellation

void USigilAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
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
						UE_LOG(LogSigilAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *AbilityInstance->GetName());
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

void USigilAbilitySystemComponent::CancelActivationGroupAbilities(ESigilAbilityActivationGroup Group, UGameplayAbility* IgnoreAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
	{
		bool SameGroup = false;
		if (const ISigilGameplayAbilityInterface* AbilityInterface = Cast<ISigilGameplayAbilityInterface>(Ability))
		{
			SameGroup = AbilityInterface->GetActivationGroup() == Group;
		}
		return (SameGroup && (Ability != IgnoreAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void USigilAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (const ISigilGameplayAbilityInterface* AbilityInterface = Cast<ISigilGameplayAbilityInterface>(Ability))
	{
		RemoveAbilityFromActivationGroup(AbilityInterface->GetActivationGroup(), Ability);
	}

	// Secondary meshes only: the engine keeps its own main-mesh bookkeeping until the montage blends out.
	for (FSigilLocalMeshMontage& Entry : LocalMeshMontages)
	{
		if (Entry.AnimatingAbility.Get() == Ability)
		{
			NotifyAbilityMeshMontage(Ability, Entry.Mesh, nullptr);
			Entry.AnimatingAbility = nullptr;
		}
	}

	AbilityEndedEvent.Broadcast(Handle, Ability, bWasCancelled);
}

void USigilAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: Apply any special logic like blocking input or movement
}

#pragma endregion

#pragma region Abilities

bool USigilAbilitySystemComponent::GetCooldownRemainingForTags(FGameplayTagContainer CooldownTags, float& TimeRemaining, float& CooldownDuration)
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

bool USigilAbilitySystemComponent::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle,
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
				if (ISigilGameplayAbilityInterface* AbilityInterface = Cast<ISigilGameplayAbilityInterface>(AbilitySpec->GetPrimaryInstance()))
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
void USigilAbilitySystemComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset(); // Fix for Version under 5.2
	TagContainer.AppendTags(GameplayTagCountContainer.GetExplicitGameplayTags());
}

FString USigilAbilitySystemComponent::GetOwnedGameplayTagsString()
{
	FString BlockedTagsStrings;
	for (auto Tag : GameplayTagCountContainer.GetExplicitGameplayTags())
	{
		BlockedTagsStrings.Append(FString::Printf(TEXT("%s (%d),\n"), *Tag.ToString(), GameplayTagCountContainer.GetTagCount(Tag)));
	}
	return BlockedTagsStrings;
}

void USigilAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired,
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

void USigilAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
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

void USigilAbilitySystemComponent::SetTagRelationshipMapping(USigilAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

#pragma endregion

#pragma region Attributes

FString USigilAbilitySystemComponent::GetOwnedGameplayAttributeSetString()
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
void USigilAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo,
                                                       FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}
#pragma endregion
#pragma region MeshMontage

bool USigilAbilitySystemComponent::IsAvatarMainMesh(const USkeletalMeshComponent* InMesh) const
{
	return InMesh && AbilityActorInfo.IsValid() && AbilityActorInfo->SkeletalMeshComponent.Get() == InMesh;
}

FSigilLocalMeshMontage* USigilAbilitySystemComponent::FindLocalMeshMontage(const USkeletalMeshComponent* InMesh)
{
	return LocalMeshMontages.FindByPredicate([InMesh](const FSigilLocalMeshMontage& Entry) { return Entry.Mesh == InMesh; });
}

const FSigilLocalMeshMontage* USigilAbilitySystemComponent::FindLocalMeshMontage(const USkeletalMeshComponent* InMesh) const
{
	return LocalMeshMontages.FindByPredicate([InMesh](const FSigilLocalMeshMontage& Entry) { return Entry.Mesh == InMesh; });
}

FSigilLocalMeshMontage& USigilAbilitySystemComponent::FindOrAddLocalMeshMontage(USkeletalMeshComponent* InMesh)
{
	if (FSigilLocalMeshMontage* Existing = FindLocalMeshMontage(InMesh))
	{
		return *Existing;
	}

	FSigilLocalMeshMontage& Entry = LocalMeshMontages.AddDefaulted_GetRef();
	Entry.Mesh = InMesh;
	return Entry;
}

UAnimInstance* USigilAbilitySystemComponent::GetSecondaryMeshAnimInstance(const USkeletalMeshComponent* InMesh) const
{
	if (!IsValid(InMesh) || !AbilityActorInfo.IsValid())
	{
		return nullptr;
	}

	// The mesh must belong to the avatar, directly or through an owned actor (weapon / equipment actor).
	const AActor* Avatar = AbilityActorInfo->AvatarActor.Get();
	bool bOwnedByAvatar = false;
	constexpr int32 MaxDepth = 8;
	const AActor* Current = InMesh->GetOwner();
	for (int32 Depth = 0; Current && Depth < MaxDepth; ++Depth, Current = Current->GetOwner())
	{
		if (Current == Avatar)
		{
			bOwnedByAvatar = true;
			break;
		}
	}

	return bOwnedByAvatar ? InMesh->GetAnimInstance() : nullptr;
}

void USigilAbilitySystemComponent::NotifyAbilityMeshMontage(UGameplayAbility* Ability, USkeletalMeshComponent* InMesh, UAnimMontage* Montage) const
{
	if (USigilGameplayAbility* SigilAbility = Cast<USigilGameplayAbility>(Ability))
	{
		SigilAbility->SetCurrentMontageForMesh(InMesh, Montage);
	}
}

float USigilAbilitySystemComponent::PlayMontageForMesh(UGameplayAbility* InAnimatingAbility, USkeletalMeshComponent* InMesh, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* NewAnimMontage,
                                                       float InPlayRate, FName StartSectionName, float StartTimeSeconds)
{
	if (!IsValid(InMesh) || !NewAnimMontage)
	{
		return -1.f;
	}

	if (IsAvatarMainMesh(InMesh))
	{
		// Engine path: replication, prediction and the ability's CurrentMontage are handled there.
		return PlayMontage(InAnimatingAbility, ActivationInfo, NewAnimMontage, InPlayRate, StartSectionName, StartTimeSeconds);
	}

	// Secondary meshes (first person body, arms, weapon) are cosmetic for the local viewer only.
	if (!AbilityActorInfo.IsValid() || !AbilityActorInfo->IsLocallyControlled())
	{
		return -1.f;
	}

	UAnimInstance* AnimInstance = GetSecondaryMeshAnimInstance(InMesh);
	if (!AnimInstance)
	{
		UE_LOG(LogSigilAbilitySystem, Verbose, TEXT("PlayMontageForMesh: mesh [%s] has no anim instance or is not owned by avatar [%s]."), *GetNameSafe(InMesh),
		       *GetNameSafe(AbilityActorInfo->AvatarActor.Get()));
		return -1.f;
	}

	const float Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate, EMontagePlayReturnType::MontageLength, StartTimeSeconds);
	if (Duration <= 0.f)
	{
		return Duration;
	}

	FSigilLocalMeshMontage& Entry = FindOrAddLocalMeshMontage(InMesh);
	if (UGameplayAbility* Previous = Entry.AnimatingAbility.Get())
	{
		if (Previous != InAnimatingAbility)
		{
			// The previous ability already received the montage's interrupted callback; it is expected to end itself.
			NotifyAbilityMeshMontage(Previous, InMesh, nullptr);
		}
	}

	Entry.AnimMontage = NewAnimMontage;
	Entry.AnimatingAbility = InAnimatingAbility;
	NotifyAbilityMeshMontage(InAnimatingAbility, InMesh, NewAnimMontage);

	if (StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(StartSectionName, NewAnimMontage);
	}

	return Duration;
}

void USigilAbilitySystemComponent::CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
	if (IsAvatarMainMesh(InMesh))
	{
		CurrentMontageStop(OverrideBlendOutTime);
		return;
	}

	const FSigilLocalMeshMontage* Entry = FindLocalMeshMontage(InMesh);
	UAnimInstance* AnimInstance = GetSecondaryMeshAnimInstance(InMesh);
	UAnimMontage* MontageToStop = Entry ? Entry->AnimMontage.Get() : nullptr;
	if (AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop))
	{
		const float BlendOutTime = (OverrideBlendOutTime >= 0.0f ? OverrideBlendOutTime : MontageToStop->BlendOut.GetBlendTime());
		AnimInstance->Montage_Stop(BlendOutTime, MontageToStop);
	}
}

void USigilAbilitySystemComponent::StopAllCurrentMontages(float OverrideBlendOutTime)
{
	CurrentMontageStop(OverrideBlendOutTime);

	// Copy the mesh list: stopping can trigger callbacks that mutate the bookkeeping.
	TArray<TObjectPtr<USkeletalMeshComponent>> Meshes;
	for (const FSigilLocalMeshMontage& Entry : LocalMeshMontages)
	{
		Meshes.Add(Entry.Mesh);
	}
	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		CurrentMontageStopForMesh(Mesh, OverrideBlendOutTime);
	}
}

void USigilAbilitySystemComponent::CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName)
{
	if (IsAvatarMainMesh(InMesh))
	{
		CurrentMontageJumpToSection(SectionName);
		return;
	}

	UAnimInstance* AnimInstance = GetSecondaryMeshAnimInstance(InMesh);
	if (UAnimMontage* CurrentMontage = GetCurrentMontageForMesh(InMesh); CurrentMontage && AnimInstance && SectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(SectionName, CurrentMontage);
	}
}

void USigilAbilitySystemComponent::CurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName)
{
	if (IsAvatarMainMesh(InMesh))
	{
		CurrentMontageSetNextSectionName(FromSectionName, ToSectionName);
		return;
	}

	UAnimInstance* AnimInstance = GetSecondaryMeshAnimInstance(InMesh);
	if (UAnimMontage* CurrentMontage = GetCurrentMontageForMesh(InMesh); CurrentMontage && AnimInstance)
	{
		AnimInstance->Montage_SetNextSection(FromSectionName, ToSectionName, CurrentMontage);
	}
}

void USigilAbilitySystemComponent::CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, float InPlayRate)
{
	if (IsAvatarMainMesh(InMesh))
	{
		CurrentMontageSetPlayRate(InPlayRate);
		return;
	}

	UAnimInstance* AnimInstance = GetSecondaryMeshAnimInstance(InMesh);
	if (UAnimMontage* CurrentMontage = GetCurrentMontageForMesh(InMesh); CurrentMontage && AnimInstance)
	{
		AnimInstance->Montage_SetPlayRate(CurrentMontage, InPlayRate);
	}
}

UAnimMontage* USigilAbilitySystemComponent::GetCurrentMontageForMesh(const USkeletalMeshComponent* InMesh) const
{
	if (IsAvatarMainMesh(InMesh))
	{
		return GetCurrentMontage();
	}

	const FSigilLocalMeshMontage* Entry = FindLocalMeshMontage(InMesh);
	const UAnimInstance* AnimInstance = GetSecondaryMeshAnimInstance(InMesh);
	if (Entry && Entry->AnimMontage && AnimInstance && AnimInstance->Montage_IsActive(Entry->AnimMontage))
	{
		return Entry->AnimMontage;
	}

	return nullptr;
}

UGameplayAbility* USigilAbilitySystemComponent::GetAnimatingAbilityForMesh(const USkeletalMeshComponent* InMesh) const
{
	if (IsAvatarMainMesh(InMesh))
	{
		return GetAnimatingAbility();
	}

	const FSigilLocalMeshMontage* Entry = FindLocalMeshMontage(InMesh);
	return Entry ? Entry->AnimatingAbility.Get() : nullptr;
}

bool USigilAbilitySystemComponent::IsAnimatingAbilityForAnyMesh(const UGameplayAbility* Ability) const
{
	if (!Ability)
	{
		return false;
	}

	if (GetAnimatingAbility() == Ability)
	{
		return true;
	}

	for (const FSigilLocalMeshMontage& Entry : LocalMeshMontages)
	{
		if (Entry.AnimatingAbility.Get() == Ability)
		{
			return true;
		}
	}

	return false;
}

void USigilAbilitySystemComponent::ClearAnimatingAbilityForMesh(USkeletalMeshComponent* InMesh, UGameplayAbility* Ability)
{
	if (!Ability)
	{
		return;
	}

	if (IsAvatarMainMesh(InMesh))
	{
		ClearAnimatingAbility(Ability);
		return;
	}

	if (FSigilLocalMeshMontage* Entry = FindLocalMeshMontage(InMesh); Entry && Entry->AnimatingAbility.Get() == Ability)
	{
		NotifyAbilityMeshMontage(Ability, InMesh, nullptr);
		Entry->AnimatingAbility = nullptr;
	}
}

void USigilAbilitySystemComponent::ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability)
{
	if (!Ability)
	{
		return;
	}

	ClearAnimatingAbility(Ability);

	for (FSigilLocalMeshMontage& Entry : LocalMeshMontages)
	{
		if (Entry.AnimatingAbility.Get() == Ability)
		{
			NotifyAbilityMeshMontage(Ability, Entry.Mesh, nullptr);
			Entry.AnimatingAbility = nullptr;
		}
	}
}

#pragma endregion
