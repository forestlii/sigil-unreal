// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayCueManager.h"
#include "SigilAbilitySystemGlobals.h"
#include "SigilAbilityTagRelationshipMapping.h"
#include "SigilGlobalAbilitySystem.h"
#include "SigilGasLogChannels.h"
#include "Abilities/SigilGameplayAbilityInterface.h"
#include "Utilities/SigilAbilitySystemFunctionLibrary.h"
#include "GameFramework/Pawn.h"
#include "Misc/SecureHash.h"
#include "Runtime/Launch/Resources/Version.h"

namespace
{
	void AppendEntitlementCanonicalField(FString& Target, const FString& Field)
	{
		Target.Appendf(TEXT("%d:"), Field.Len());
		Target.Append(Field);
		Target.AppendChar(TEXT(';'));
	}

	FString HashEntitlementCanonicalString(const FString& Canonical)
	{
		const FTCHARToUTF8 Utf8(*Canonical);
		return FSHA1::HashBuffer(Utf8.Get(), static_cast<uint64>(Utf8.Length())).ToString();
	}

	FString BuildAbilitySpecCanonicalEntry(const FGameplayAbilitySpec& Spec)
	{
		if (!Spec.Ability)
		{
			return FString();
		}

		TArray<FString> DynamicTagStrings;
		for (const FGameplayTag& DynamicTag : Spec.GetDynamicSpecSourceTags())
		{
			DynamicTagStrings.Add(DynamicTag.ToString());
		}
		DynamicTagStrings.Sort();

		FString CanonicalEntry;
		AppendEntitlementCanonicalField(CanonicalEntry, Spec.Ability->GetClass()->GetPathName());
		AppendEntitlementCanonicalField(CanonicalEntry, LexToString(Spec.Level));
		AppendEntitlementCanonicalField(CanonicalEntry, LexToString(Spec.InputID > 0 ? Spec.InputID : INDEX_NONE));
		for (const FString& DynamicTagString : DynamicTagStrings)
		{
			AppendEntitlementCanonicalField(CanonicalEntry, DynamicTagString);
		}
		return CanonicalEntry;
	}
}

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
	if (bAbilityEntitlementReconcileInProgress || bAbilityEntitlementResetInProgress || bAbilityActivationGateMutationInProgress)
	{
		UE_LOG(LogSigilAbilitySystem, Warning, TEXT("InitializeAbilitySystem was rejected during an entitlement projection mutation."));
		return;
	}

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
	if (bAbilityEntitlementReconcileInProgress || bAbilityEntitlementResetInProgress || bAbilityActivationGateMutationInProgress)
	{
		UE_LOG(LogSigilAbilitySystem, Warning, TEXT("UninitializeAbilitySystem was rejected during an entitlement projection mutation."));
		return;
	}

	ResetAbilityEntitlementProjection();
	if (bHasAcceptedAbilityEntitlementSnapshot || !AbilityEntitlementRuntimeGrants.IsEmpty() || !AbilityActivationGateSources.IsEmpty() || bAbilityEntitlementProjectionNeedsReset)
	{
		UE_LOG(LogSigilAbilitySystem, Warning, TEXT("UninitializeAbilitySystem retained initialization because the entitlement projection could not be reset safely."));
		return;
	}

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
	ResetAbilityEntitlementProjection();
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

#pragma region AbilityEntitlements

FSigilAbilityReconcileResult USigilAbilitySystemComponent::ReconcileAbilityEntitlements(const FSigilAbilityEntitlementSnapshot& Desired)
{
	auto MakeResult = [this](ESigilAbilityReconcileStatus Status, FString Error = FString())
	{
		FSigilAbilityReconcileResult Result;
		Result.Status = Status;
		Result.AcceptedRevision = bHasAcceptedAbilityEntitlementSnapshot ? LastAcceptedAbilityEntitlementRevision : INDEX_NONE;
		Result.CanonicalDigest = bHasAcceptedAbilityEntitlementSnapshot ? LastAcceptedAbilityEntitlementDigest : FString();
		Result.Error = MoveTemp(Error);
		return Result;
	};

	if (!IsInGameThread())
	{
		return MakeResult(ESigilAbilityReconcileStatus::InvalidSnapshot, TEXT("Entitlement reconciliation must run on the game thread."));
	}
	if (bAbilityEntitlementReconcileInProgress || bAbilityEntitlementResetInProgress || bAbilityActivationGateMutationInProgress || AbilityScopeLockCount > 0)
	{
		return MakeResult(ESigilAbilityReconcileStatus::ReentrantCall, TEXT("Entitlement reconciliation cannot run during another projection mutation or AbilityList lock."));
	}
	if (bAbilityEntitlementProjectionNeedsReset)
	{
		return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("The entitlement projection is fail-closed after a lifecycle callback changed runtime state; call ResetAbilityEntitlementProjection before reconciling again."));
	}
	TGuardValue<bool> ReconcileGuard(bAbilityEntitlementReconcileInProgress, true);

	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid() || !ActorInfo->AvatarActor.IsValid())
	{
		return MakeResult(ESigilAbilityReconcileStatus::ActorInfoNotReady, TEXT("Ability ActorInfo is not ready."));
	}
	if (!IsOwnerActorAuthoritative())
	{
		return MakeResult(ESigilAbilityReconcileStatus::NotAuthority, TEXT("Only the authoritative ASC may reconcile entitlements."));
	}
	const FGameplayAbilityActorInfo* ExpectedActorInfo = ActorInfo;
	const TWeakObjectPtr<AActor> ExpectedOwnerActor = ActorInfo->OwnerActor;
	const TWeakObjectPtr<AActor> ExpectedAvatarActor = ActorInfo->AvatarActor;
	auto IsExpectedContextCurrent = [this, ExpectedActorInfo, ExpectedOwnerActor, ExpectedAvatarActor]()
	{
		const AActor* OwnerActor = ExpectedOwnerActor.Get();
		const AActor* AvatarActor = ExpectedAvatarActor.Get();
		return AbilityActorInfo.Get() == ExpectedActorInfo
			&& ExpectedActorInfo->OwnerActor == ExpectedOwnerActor
			&& ExpectedActorInfo->AvatarActor == ExpectedAvatarActor
			&& IsValid(OwnerActor)
			&& IsValid(AvatarActor)
			&& !OwnerActor->IsActorBeingDestroyed()
			&& !AvatarActor->IsActorBeingDestroyed()
			&& IsOwnerActorAuthoritative()
			&& AbilityScopeLockCount == 0;
	};
	if (Desired.Revision < 0)
	{
		return MakeResult(ESigilAbilityReconcileStatus::InvalidSnapshot, TEXT("Revision must be non-negative."));
	}

	auto ValidateRuntimeGrant = [this](const FAbilityEntitlementRuntimeGrant& RuntimeGrant, TSet<FGameplayAbilitySpecHandle>* OutOwnedHandles = nullptr)
	{
		if (RuntimeGrant.Contributors.IsEmpty()
			|| RuntimeGrant.GrantedHandles.AbilitySpecHandles.Num() != RuntimeGrant.ExpectedSpecCanonicalEntries.Num())
		{
			return false;
		}

		TSet<FString> SeenAbilityClasses;
		for (const FGameplayAbilitySpecHandle& Handle : RuntimeGrant.GrantedHandles.AbilitySpecHandles)
		{
			const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle, EConsiderPending::None);
			if (!Handle.IsValid() || !Spec || Spec->PendingRemove || !Spec->Ability || Spec->SourceObject.Get() != this)
			{
				return false;
			}

			const FString AbilityClassPath = Spec->Ability->GetClass()->GetPathName();
			const FString* ExpectedEntry = RuntimeGrant.ExpectedSpecCanonicalEntries.Find(AbilityClassPath);
			if (!ExpectedEntry || SeenAbilityClasses.Contains(AbilityClassPath) || *ExpectedEntry != BuildAbilitySpecCanonicalEntry(*Spec))
			{
				return false;
			}
			SeenAbilityClasses.Add(AbilityClassPath);
			if (OutOwnedHandles)
			{
				OutOwnedHandles->Add(Handle);
			}
		}
		return SeenAbilityClasses.Num() == RuntimeGrant.ExpectedSpecCanonicalEntries.Num();
	};
	auto ValidateGateLedger = [this]()
	{
		for (const TPair<FGameplayTag, TSet<FName>>& GatePair : AbilityActivationGateSources)
		{
			if (GatePair.Value.IsEmpty() || GameplayTagCountContainer.GetExplicitTagCount(GatePair.Key) != 1)
			{
				return false;
			}
		}
		return true;
	};

	TSet<FGameplayAbilitySpecHandle> ProjectionOwnedHandles;
	for (const TPair<FString, FAbilityEntitlementRuntimeGrant>& RuntimePair : AbilityEntitlementRuntimeGrants)
	{
		if (!ValidateRuntimeGrant(RuntimePair.Value, &ProjectionOwnedHandles))
		{
			return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("Runtime projection no longer matches its recorded ability identity."));
		}
	}
	for (const TPair<FGameplayTag, FString>& EntitlementPair : AbilityEntitlementToIdentity)
	{
		const FAbilityEntitlementRuntimeGrant* RuntimeGrant = AbilityEntitlementRuntimeGrants.Find(EntitlementPair.Value);
		if (!RuntimeGrant || !RuntimeGrant->Contributors.Contains(EntitlementPair.Key))
		{
			return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("Runtime entitlement and contributor indexes disagree."));
		}
	}
	if (!ValidateGateLedger())
	{
		bAbilityEntitlementProjectionNeedsReset = true;
		return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("The activation-gate source ledger no longer owns exactly one explicit contribution per gate tag; an authoritative reset is required."));
	}

	TMap<FGameplayTag, FString> DesiredEntitlementToIdentity;
	TMap<FString, USigilAbilitySet::FAbilityOnlyGrantPlan> DesiredPlans;
	TMap<FString, TSet<FGameplayTag>> DesiredContributors;
	TMap<FString, FString> DesiredAbilityClassToIdentity;
	TArray<TPair<FString, FString>> CanonicalGrants;

	for (int32 GrantIndex = 0; GrantIndex < Desired.Grants.Num(); ++GrantIndex)
	{
		const FSigilAbilityEntitlementGrant& Grant = Desired.Grants[GrantIndex];
		if (!Grant.EntitlementTag.IsValid())
		{
			return MakeResult(ESigilAbilityReconcileStatus::InvalidSnapshot, FString::Printf(TEXT("Grants[%d] has an invalid entitlement tag."), GrantIndex));
		}
		if (DesiredEntitlementToIdentity.Contains(Grant.EntitlementTag))
		{
			return MakeResult(ESigilAbilityReconcileStatus::InvalidSnapshot, FString::Printf(TEXT("Entitlement tag [%s] appears more than once."), *Grant.EntitlementTag.ToString()));
		}
		if (!IsValid(Grant.AbilitySet))
		{
			return MakeResult(ESigilAbilityReconcileStatus::InvalidSnapshot, FString::Printf(TEXT("Grants[%d] has no already-loaded ability set."), GrantIndex));
		}

		USigilAbilitySet::FAbilityOnlyGrantPlan Plan;
		FString PlanError;
		if (!Grant.AbilitySet->BuildAbilityOnlyEntitlementGrantPlan(Grant.OverrideLevel, Plan, PlanError))
		{
			return MakeResult(ESigilAbilityReconcileStatus::InvalidSnapshot, FString::Printf(TEXT("Grants[%d] failed preflight: %s"), GrantIndex, *PlanError));
		}
		for (const FString& AbilityClassPath : Plan.AbilityClassPaths)
		{
			if (const FString* ExistingIdentity = DesiredAbilityClassToIdentity.Find(AbilityClassPath); ExistingIdentity && *ExistingIdentity != Plan.CanonicalIdentity)
			{
				return MakeResult(ESigilAbilityReconcileStatus::InvalidSnapshot, FString::Printf(TEXT("Ability class [%s] is requested through incompatible grant identities."), *AbilityClassPath));
			}
			DesiredAbilityClassToIdentity.Add(AbilityClassPath, Plan.CanonicalIdentity);
		}

		DesiredEntitlementToIdentity.Add(Grant.EntitlementTag, Plan.CanonicalIdentity);
		DesiredContributors.FindOrAdd(Plan.CanonicalIdentity).Add(Grant.EntitlementTag);
		DesiredPlans.FindOrAdd(Plan.CanonicalIdentity) = MoveTemp(Plan);
		CanonicalGrants.Emplace(Grant.EntitlementTag.ToString(), DesiredEntitlementToIdentity[Grant.EntitlementTag]);
	}

	TSet<FGameplayAbilitySpecHandle> BaselineProjectionSourceHandles;
	for (const FGameplayAbilitySpec& ExistingSpec : ActivatableAbilities.Items)
	{
		if (!ExistingSpec.Ability || ExistingSpec.PendingRemove || ProjectionOwnedHandles.Contains(ExistingSpec.Handle))
		{
			continue;
		}
		if (ExistingSpec.SourceObject.Get() == this)
		{
			BaselineProjectionSourceHandles.Add(ExistingSpec.Handle);
		}
		const FString ExistingClassPath = ExistingSpec.Ability->GetClass()->GetPathName();
		if (DesiredAbilityClassToIdentity.Contains(ExistingClassPath))
		{
			return MakeResult(ESigilAbilityReconcileStatus::InvalidSnapshot, FString::Printf(TEXT("Ability class [%s] is already owned by a non-entitlement grant."), *ExistingClassPath));
		}
	}

	CanonicalGrants.Sort([](const TPair<FString, FString>& Left, const TPair<FString, FString>& Right)
	{
		return Left.Key == Right.Key ? Left.Value < Right.Value : Left.Key < Right.Key;
	});
	FString CanonicalSnapshot;
	AppendEntitlementCanonicalField(CanonicalSnapshot, TEXT("SigilAbilityEntitlementSnapshotV1"));
	for (const TPair<FString, FString>& CanonicalGrant : CanonicalGrants)
	{
		AppendEntitlementCanonicalField(CanonicalSnapshot, CanonicalGrant.Key);
		AppendEntitlementCanonicalField(CanonicalSnapshot, CanonicalGrant.Value);
	}
	const FString DesiredDigest = HashEntitlementCanonicalString(CanonicalSnapshot);

	if (bHasAcceptedAbilityEntitlementSnapshot)
	{
		if (Desired.Revision < LastAcceptedAbilityEntitlementRevision)
		{
			return MakeResult(ESigilAbilityReconcileStatus::StaleRevision, TEXT("Revision is older than the accepted projection."));
		}
		if (Desired.Revision == LastAcceptedAbilityEntitlementRevision)
		{
			return DesiredDigest == LastAcceptedAbilityEntitlementDigest
				? MakeResult(ESigilAbilityReconcileStatus::Unchanged)
				: MakeResult(ESigilAbilityReconcileStatus::RevisionConflict, TEXT("The accepted revision was reused with a different canonical payload."));
		}
	}
	for (const TPair<FString, FAbilityEntitlementRuntimeGrant>& RuntimePair : AbilityEntitlementRuntimeGrants)
	{
		for (const TPair<FString, FString>& ExistingSpecEntry : RuntimePair.Value.ExpectedSpecCanonicalEntries)
		{
			if (const FString* DesiredIdentity = DesiredAbilityClassToIdentity.Find(ExistingSpecEntry.Key);
				DesiredIdentity && *DesiredIdentity != RuntimePair.Key)
			{
				return MakeResult(
					ESigilAbilityReconcileStatus::InvalidSnapshot,
					FString::Printf(
						TEXT("Ability class [%s] cannot change grant identity while its previous entitlement projection is still active."),
						*ExistingSpecEntry.Key));
			}
		}
	}

	TArray<FString> AddedIdentities;
	for (const TPair<FString, TSet<FGameplayTag>>& DesiredPair : DesiredContributors)
	{
		if (!AbilityEntitlementRuntimeGrants.Contains(DesiredPair.Key))
		{
			AddedIdentities.Add(DesiredPair.Key);
		}
	}
	AddedIdentities.Sort();

	TMap<FString, FAbilityEntitlementRuntimeGrant> NewlyGrantedIdentities;
	int32 GrantOrdinal = 0;
	auto RetainUnexpectedOwnedHandlesForReset = [this](const TArray<FGameplayAbilitySpecHandle>& UnexpectedOwnedHandles)
	{
		if (UnexpectedOwnedHandles.IsEmpty())
		{
			return;
		}

		FAbilityEntitlementRuntimeGrant& RecoveryGrant = AbilityEntitlementRuntimeGrants.FindOrAdd(TEXT("__Recovery.UntrackedOnAbilityCallback"));
		for (const FGameplayAbilitySpecHandle& Handle : UnexpectedOwnedHandles)
		{
			if (!RecoveryGrant.GrantedHandles.AbilitySpecHandles.Contains(Handle))
			{
				RecoveryGrant.GrantedHandles.AddAbilitySpecHandle(Handle);
			}
		}
		bAbilityEntitlementProjectionNeedsReset = true;
	};
	auto ValidateNoUnexpectedAbilitySpecs = [this, &BaselineProjectionSourceHandles, &DesiredAbilityClassToIdentity](
		const TSet<FGameplayAbilitySpecHandle>& TrackedProjectionHandles,
		TArray<FGameplayAbilitySpecHandle>& OutUnexpectedOwnedHandles)
	{
		bool bValid = true;
		OutUnexpectedOwnedHandles.Reset();
		for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
		{
			if (!Spec.Ability || Spec.PendingRemove || TrackedProjectionHandles.Contains(Spec.Handle))
			{
				continue;
			}

			const bool bProjectionOwned = Spec.SourceObject.Get() == this && !BaselineProjectionSourceHandles.Contains(Spec.Handle);
			const bool bDesiredClassCollision = DesiredAbilityClassToIdentity.Contains(Spec.Ability->GetClass()->GetPathName());
			if (bProjectionOwned)
			{
				OutUnexpectedOwnedHandles.Add(Spec.Handle);
			}
			bValid &= !bProjectionOwned && !bDesiredClassCollision;
		}
		return bValid;
	};
	auto GatherTrackedProjectionHandles = [&ProjectionOwnedHandles, &NewlyGrantedIdentities]()
	{
		TSet<FGameplayAbilitySpecHandle> TrackedHandles = ProjectionOwnedHandles;
		for (const TPair<FString, FAbilityEntitlementRuntimeGrant>& NewPair : NewlyGrantedIdentities)
		{
			for (const FGameplayAbilitySpecHandle& Handle : NewPair.Value.GrantedHandles.AbilitySpecHandles)
			{
				TrackedHandles.Add(Handle);
			}
		}
		return TrackedHandles;
	};
	auto RollBackNewGrants = [this,
		&NewlyGrantedIdentities,
		&GatherTrackedProjectionHandles,
		&ValidateNoUnexpectedAbilitySpecs,
		&RetainUnexpectedOwnedHandlesForReset,
		&IsExpectedContextCurrent,
		&ValidateRuntimeGrant,
		&ValidateGateLedger]()
	{
		bool bCleanupComplete = true;
		for (TPair<FString, FAbilityEntitlementRuntimeGrant>& NewPair : NewlyGrantedIdentities)
		{
			FSigilAbilitySet_GrantedHandles HandlesToRemove = NewPair.Value.GrantedHandles;
			HandlesToRemove.TakeFromAbilitySystem(this);

			bool bAnyHandleRemains = false;
			for (const FGameplayAbilitySpecHandle& Handle : NewPair.Value.GrantedHandles.AbilitySpecHandles)
			{
				bAnyHandleRemains |= FindAbilitySpecFromHandle(Handle, EConsiderPending::All) != nullptr;
			}
			if (bAnyHandleRemains)
			{
				AbilityEntitlementRuntimeGrants.Add(NewPair.Key, NewPair.Value);
				bAbilityEntitlementProjectionNeedsReset = true;
				bCleanupComplete = false;
			}
		}

		TArray<FGameplayAbilitySpecHandle> UnexpectedOwnedHandles;
		if (!ValidateNoUnexpectedAbilitySpecs(GatherTrackedProjectionHandles(), UnexpectedOwnedHandles))
		{
			RetainUnexpectedOwnedHandlesForReset(UnexpectedOwnedHandles);
			bAbilityEntitlementProjectionNeedsReset = true;
			bCleanupComplete = false;
		}
		if (!IsExpectedContextCurrent())
		{
			bAbilityEntitlementProjectionNeedsReset = true;
			bCleanupComplete = false;
		}
		for (const TPair<FString, FAbilityEntitlementRuntimeGrant>& ExistingRuntimePair : AbilityEntitlementRuntimeGrants)
		{
			if (!ValidateRuntimeGrant(ExistingRuntimePair.Value))
			{
				bAbilityEntitlementProjectionNeedsReset = true;
				bCleanupComplete = false;
			}
		}
		if (!ValidateGateLedger())
		{
			bAbilityEntitlementProjectionNeedsReset = true;
			bCleanupComplete = false;
		}
		NewlyGrantedIdentities.Reset();
		return bCleanupComplete;
	};

	for (const FString& Identity : AddedIdentities)
	{
		const USigilAbilitySet::FAbilityOnlyGrantPlan* Plan = DesiredPlans.Find(Identity);
		check(Plan);
		FAbilityEntitlementRuntimeGrant& NewRuntimeGrant = NewlyGrantedIdentities.Add(Identity);
		NewRuntimeGrant.Contributors = DesiredContributors[Identity];
		for (const USigilAbilitySet::FAbilityOnlyGrantPlanEntry& Entry : Plan->Entries)
		{
			NewRuntimeGrant.ExpectedSpecCanonicalEntries.Add(Entry.AbilityClassPath, Entry.CanonicalEntry);
			++GrantOrdinal;
#if WITH_DEV_AUTOMATION_TESTS
			if (AbilityEntitlementFailGrantOrdinalForTest == GrantOrdinal)
			{
				if (!RollBackNewGrants())
				{
					return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, FString::Printf(TEXT("Injected grant failure at ordinal %d could not fully remove newly granted abilities; an authoritative reset is required."), GrantOrdinal));
				}
				return MakeResult(ESigilAbilityReconcileStatus::GrantFailed, FString::Printf(TEXT("Injected grant failure at ordinal %d."), GrantOrdinal));
			}
#endif
			UGameplayAbility* AbilityCDO = Entry.AbilityClass->GetDefaultObject<UGameplayAbility>();
			FGameplayAbilitySpec AbilitySpec(AbilityCDO, Entry.AbilityLevel);
			AbilitySpec.SourceObject = this;
			if (Entry.InputID > 0)
			{
				AbilitySpec.InputID = Entry.InputID;
			}
			AbilitySpec.GetDynamicSpecSourceTags().AppendTags(Entry.DynamicTags);
			const FGameplayAbilitySpecHandle Handle = GiveAbility(AbilitySpec);
			if (!Handle.IsValid())
			{
				if (!RollBackNewGrants())
				{
					return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, FString::Printf(TEXT("GiveAbility failed at ordinal %d and rollback could not fully remove newly granted abilities; an authoritative reset is required."), GrantOrdinal));
				}
				return MakeResult(ESigilAbilityReconcileStatus::GrantFailed, FString::Printf(TEXT("GiveAbility failed at ordinal %d."), GrantOrdinal));
			}
			NewRuntimeGrant.GrantedHandles.AddAbilitySpecHandle(Handle);
			if (!IsExpectedContextCurrent())
			{
				bAbilityEntitlementProjectionNeedsReset = true;
				RollBackNewGrants();
				return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("The ASC context changed during an OnGive callback; the projection now requires an authoritative reset."));
			}
			if (!ValidateRuntimeGrant(NewRuntimeGrant))
			{
				RollBackNewGrants();
				return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("A newly granted ability changed during an OnGive callback."));
			}
			for (const TPair<FString, FAbilityEntitlementRuntimeGrant>& ExistingRuntimePair : AbilityEntitlementRuntimeGrants)
			{
				if (!ValidateRuntimeGrant(ExistingRuntimePair.Value))
				{
					bAbilityEntitlementProjectionNeedsReset = true;
					RollBackNewGrants();
					return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("An existing projection grant changed during an OnGive callback; the projection now requires an authoritative reset."));
				}
			}
			if (!ValidateGateLedger())
			{
				bAbilityEntitlementProjectionNeedsReset = true;
				RollBackNewGrants();
				return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("An OnGive callback changed the activation-gate source ledger or explicit count; an authoritative reset is required."));
			}
			TArray<FGameplayAbilitySpecHandle> UnexpectedOwnedHandles;
			if (!ValidateNoUnexpectedAbilitySpecs(GatherTrackedProjectionHandles(), UnexpectedOwnedHandles))
			{
				RetainUnexpectedOwnedHandlesForReset(UnexpectedOwnedHandles);
				bAbilityEntitlementProjectionNeedsReset = true;
				RollBackNewGrants();
				return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("An OnGive callback introduced an untracked projection-owned or conflicting ability; an authoritative reset is required."));
			}
		}
	}

	TArray<FString> RemovedIdentities;
	for (const TPair<FString, FAbilityEntitlementRuntimeGrant>& RuntimePair : AbilityEntitlementRuntimeGrants)
	{
		if (!DesiredContributors.Contains(RuntimePair.Key))
		{
			RemovedIdentities.Add(RuntimePair.Key);
		}
	}
	RemovedIdentities.Sort();
	for (const FString& Identity : RemovedIdentities)
	{
		FSigilAbilitySet_GrantedHandles HandlesToRemove = AbilityEntitlementRuntimeGrants[Identity].GrantedHandles;
		HandlesToRemove.TakeFromAbilitySystem(this);
	}

	if (!IsExpectedContextCurrent())
	{
		bAbilityEntitlementProjectionNeedsReset = true;
		RollBackNewGrants();
		return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("The ASC context changed while removing obsolete entitlement grants; ownership bookkeeping was retained and an authoritative reset is required."));
	}
	for (const TPair<FString, TSet<FGameplayTag>>& DesiredPair : DesiredContributors)
	{
		const FAbilityEntitlementRuntimeGrant* DesiredRuntimeGrant = AbilityEntitlementRuntimeGrants.Find(DesiredPair.Key);
		if (!DesiredRuntimeGrant)
		{
			DesiredRuntimeGrant = NewlyGrantedIdentities.Find(DesiredPair.Key);
		}
		if (!DesiredRuntimeGrant || !ValidateRuntimeGrant(*DesiredRuntimeGrant))
		{
			bAbilityEntitlementProjectionNeedsReset = true;
			RollBackNewGrants();
			return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("The desired projection changed during an OnRemove callback; ownership bookkeeping was retained and an authoritative reset is required."));
		}
	}
	if (!ValidateGateLedger())
	{
		bAbilityEntitlementProjectionNeedsReset = true;
		RollBackNewGrants();
		return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("An OnRemove callback changed the activation-gate source ledger or explicit count; an authoritative reset is required."));
	}

	TSet<FGameplayAbilitySpecHandle> DesiredProjectionHandles;
	for (const TPair<FString, TSet<FGameplayTag>>& DesiredPair : DesiredContributors)
	{
		const FAbilityEntitlementRuntimeGrant* DesiredRuntimeGrant = AbilityEntitlementRuntimeGrants.Find(DesiredPair.Key);
		if (!DesiredRuntimeGrant)
		{
			DesiredRuntimeGrant = NewlyGrantedIdentities.Find(DesiredPair.Key);
		}
		check(DesiredRuntimeGrant);
		for (const FGameplayAbilitySpecHandle& Handle : DesiredRuntimeGrant->GrantedHandles.AbilitySpecHandles)
		{
			DesiredProjectionHandles.Add(Handle);
		}
	}
	TArray<FGameplayAbilitySpecHandle> UnexpectedOwnedHandles;
	if (!ValidateNoUnexpectedAbilitySpecs(DesiredProjectionHandles, UnexpectedOwnedHandles))
	{
		RetainUnexpectedOwnedHandlesForReset(UnexpectedOwnedHandles);
		bAbilityEntitlementProjectionNeedsReset = true;
		RollBackNewGrants();
		return MakeResult(ESigilAbilityReconcileStatus::RuntimeStateMismatch, TEXT("An ability lifecycle callback introduced an untracked projection-owned or conflicting ability; an authoritative reset is required."));
	}

	TMap<FString, FAbilityEntitlementRuntimeGrant> NextRuntimeGrants;
	for (const TPair<FString, TSet<FGameplayTag>>& DesiredPair : DesiredContributors)
	{
		if (const FAbilityEntitlementRuntimeGrant* ExistingRuntimeGrant = AbilityEntitlementRuntimeGrants.Find(DesiredPair.Key))
		{
			FAbilityEntitlementRuntimeGrant NextRuntimeGrant = *ExistingRuntimeGrant;
			NextRuntimeGrant.Contributors = DesiredPair.Value;
			NextRuntimeGrants.Add(DesiredPair.Key, MoveTemp(NextRuntimeGrant));
		}
		else
		{
			NextRuntimeGrants.Add(DesiredPair.Key, MoveTemp(NewlyGrantedIdentities[DesiredPair.Key]));
		}
	}

	AbilityEntitlementRuntimeGrants = MoveTemp(NextRuntimeGrants);
	AbilityEntitlementToIdentity = MoveTemp(DesiredEntitlementToIdentity);
	LastAcceptedAbilityEntitlementRevision = Desired.Revision;
	LastAcceptedAbilityEntitlementDigest = DesiredDigest;
	bHasAcceptedAbilityEntitlementSnapshot = true;
	bAbilityEntitlementProjectionNeedsReset = false;

	FSigilAbilityReconcileResult Result = MakeResult(ESigilAbilityReconcileStatus::Applied);
	Result.AcceptedRevision = Desired.Revision;
	Result.CanonicalDigest = DesiredDigest;
	return Result;
}

void USigilAbilitySystemComponent::ResetAbilityEntitlementProjection()
{
	if (!IsInGameThread() || bAbilityEntitlementReconcileInProgress || bAbilityEntitlementResetInProgress || bAbilityActivationGateMutationInProgress || AbilityScopeLockCount > 0)
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	const bool bHasOwnedRuntimeState = !AbilityEntitlementRuntimeGrants.IsEmpty() || !AbilityActivationGateSources.IsEmpty() || bAbilityEntitlementProjectionNeedsReset;
	if (bHasOwnedRuntimeState && (!ActorInfo || !ActorInfo->OwnerActor.IsValid() || !ActorInfo->AvatarActor.IsValid() || !IsOwnerActorAuthoritative()))
	{
		UE_LOG(LogSigilAbilitySystem, Warning, TEXT("ResetAbilityEntitlementProjection retained its bookkeeping because authority or ActorInfo was unavailable."));
		return;
	}
	const FGameplayAbilityActorInfo* ExpectedActorInfo = ActorInfo;
	const TWeakObjectPtr<AActor> ExpectedOwnerActor = ActorInfo ? ActorInfo->OwnerActor : TWeakObjectPtr<AActor>();
	const TWeakObjectPtr<AActor> ExpectedAvatarActor = ActorInfo ? ActorInfo->AvatarActor : TWeakObjectPtr<AActor>();
	auto IsResetContextCurrent = [this, bHasOwnedRuntimeState, ExpectedActorInfo, ExpectedOwnerActor, ExpectedAvatarActor]()
	{
		const AActor* OwnerActor = ExpectedOwnerActor.Get();
		const AActor* AvatarActor = ExpectedAvatarActor.Get();
		return !bHasOwnedRuntimeState
			|| (AbilityActorInfo.Get() == ExpectedActorInfo
			&& (!ExpectedActorInfo || ExpectedActorInfo->OwnerActor == ExpectedOwnerActor)
			&& (!ExpectedActorInfo || ExpectedActorInfo->AvatarActor == ExpectedAvatarActor)
			&& IsValid(OwnerActor)
			&& IsValid(AvatarActor)
			&& !OwnerActor->IsActorBeingDestroyed()
			&& !AvatarActor->IsActorBeingDestroyed()
			&& IsOwnerActorAuthoritative());
	};

	TGuardValue<bool> ResetGuard(bAbilityEntitlementResetInProgress, true);
	TSet<FGameplayAbilitySpecHandle> ProjectionHandlesToRemove;
	for (const TPair<FString, FAbilityEntitlementRuntimeGrant>& RuntimePair : AbilityEntitlementRuntimeGrants)
	{
		for (const FGameplayAbilitySpecHandle& Handle : RuntimePair.Value.GrantedHandles.AbilitySpecHandles)
		{
			ProjectionHandlesToRemove.Add(Handle);
		}
	}
	TSet<FGameplayAbilitySpecHandle> BaselineNonProjectionSourceHandles;
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && !Spec.PendingRemove && Spec.SourceObject.Get() == this && !ProjectionHandlesToRemove.Contains(Spec.Handle))
		{
			BaselineNonProjectionSourceHandles.Add(Spec.Handle);
		}
	}

	TMap<FString, FAbilityEntitlementRuntimeGrant> RuntimeGrantsToRemove = AbilityEntitlementRuntimeGrants;
	const TMap<FGameplayTag, TSet<FName>> GateSourcesToRemove = AbilityActivationGateSources;

	for (TPair<FString, FAbilityEntitlementRuntimeGrant>& RuntimePair : RuntimeGrantsToRemove)
	{
		RuntimePair.Value.GrantedHandles.TakeFromAbilitySystem(this);
	}

	bool bAnyTrackedHandleRemains = false;
	for (const FGameplayAbilitySpecHandle& Handle : ProjectionHandlesToRemove)
	{
		bAnyTrackedHandleRemains |= FindAbilitySpecFromHandle(Handle, EConsiderPending::All) != nullptr;
	}

	TArray<FGameplayAbilitySpecHandle> UnexpectedOwnedHandles;
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability
			&& !Spec.PendingRemove
			&& Spec.SourceObject.Get() == this
			&& !ProjectionHandlesToRemove.Contains(Spec.Handle)
			&& !BaselineNonProjectionSourceHandles.Contains(Spec.Handle))
		{
			UnexpectedOwnedHandles.Add(Spec.Handle);
		}
	}
	if (!UnexpectedOwnedHandles.IsEmpty())
	{
		FAbilityEntitlementRuntimeGrant& RecoveryGrant = AbilityEntitlementRuntimeGrants.FindOrAdd(TEXT("__Recovery.UntrackedOnResetCallback"));
		for (const FGameplayAbilitySpecHandle& Handle : UnexpectedOwnedHandles)
		{
			if (!RecoveryGrant.GrantedHandles.AbilitySpecHandles.Contains(Handle))
			{
				RecoveryGrant.GrantedHandles.AddAbilitySpecHandle(Handle);
			}
		}
	}
	if (bAnyTrackedHandleRemains || !UnexpectedOwnedHandles.IsEmpty())
	{
		bAbilityEntitlementProjectionNeedsReset = true;
		UE_LOG(LogSigilAbilitySystem, Error, TEXT("ResetAbilityEntitlementProjection retained ownership bookkeeping because a tracked AbilitySpec survived or an OnRemove callback created a new projection-owned AbilitySpec."));
		return;
	}
	if (!IsResetContextCurrent())
	{
		bAbilityEntitlementProjectionNeedsReset = true;
		UE_LOG(LogSigilAbilitySystem, Error, TEXT("ResetAbilityEntitlementProjection retained its gate and ownership bookkeeping because an OnRemove callback changed the ASC owner or avatar context."));
		return;
	}

	for (const TPair<FGameplayTag, TSet<FName>>& GatePair : GateSourcesToRemove)
	{
		if (!GatePair.Value.IsEmpty())
		{
			RemoveLooseGameplayTag(GatePair.Key, 1, EGameplayTagReplicationState::TagOnly);
		}
	}
	AbilityActivationGateSources.Reset();

	UnexpectedOwnedHandles.Reset();
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability
			&& !Spec.PendingRemove
			&& Spec.SourceObject.Get() == this
			&& !ProjectionHandlesToRemove.Contains(Spec.Handle)
			&& !BaselineNonProjectionSourceHandles.Contains(Spec.Handle))
		{
			UnexpectedOwnedHandles.Add(Spec.Handle);
		}
	}
	if (!UnexpectedOwnedHandles.IsEmpty())
	{
		FAbilityEntitlementRuntimeGrant& RecoveryGrant = AbilityEntitlementRuntimeGrants.FindOrAdd(TEXT("__Recovery.UntrackedOnResetCallback"));
		for (const FGameplayAbilitySpecHandle& Handle : UnexpectedOwnedHandles)
		{
			if (!RecoveryGrant.GrantedHandles.AbilitySpecHandles.Contains(Handle))
			{
				RecoveryGrant.GrantedHandles.AddAbilitySpecHandle(Handle);
			}
		}
	}
	if (!IsResetContextCurrent() || !UnexpectedOwnedHandles.IsEmpty())
	{
		bAbilityEntitlementProjectionNeedsReset = true;
		UE_LOG(LogSigilAbilitySystem, Error, TEXT("ResetAbilityEntitlementProjection removed its gate contribution but retained ability ownership bookkeeping because a tag delegate changed context or created a projection-owned AbilitySpec."));
		return;
	}

	AbilityEntitlementRuntimeGrants.Reset();
	AbilityEntitlementToIdentity.Reset();
	LastAcceptedAbilityEntitlementRevision = INDEX_NONE;
	LastAcceptedAbilityEntitlementDigest.Reset();
	bHasAcceptedAbilityEntitlementSnapshot = false;
	bAbilityEntitlementProjectionNeedsReset = false;
	AbilityEntitlementProjectionEpoch = AbilityEntitlementProjectionEpoch == MAX_int64 ? 1 : AbilityEntitlementProjectionEpoch + 1;
}

void USigilAbilitySystemComponent::SetAbilityActivationGateSource(FGameplayTag GateTag, FName SourceId, bool bPresent)
{
	if (!IsInGameThread()
		|| bAbilityEntitlementReconcileInProgress
		|| bAbilityEntitlementResetInProgress
		|| bAbilityActivationGateMutationInProgress
		|| bAbilityEntitlementProjectionNeedsReset
		|| AbilityScopeLockCount > 0
		|| !GateTag.IsValid()
		|| SourceId.IsNone())
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid() || !ActorInfo->AvatarActor.IsValid() || !IsOwnerActorAuthoritative())
	{
		return;
	}

	auto ValidateRuntimeProjection = [this](TSet<FGameplayAbilitySpecHandle>& OutTrackedHandles, TSet<FString>& OutTrackedClassPaths)
	{
		OutTrackedHandles.Reset();
		OutTrackedClassPaths.Reset();
		TSet<FGameplayTag> SeenContributors;

		if ((!AbilityEntitlementRuntimeGrants.IsEmpty() || !AbilityEntitlementToIdentity.IsEmpty()) && !bHasAcceptedAbilityEntitlementSnapshot)
		{
			return false;
		}

		for (const TPair<FString, FAbilityEntitlementRuntimeGrant>& RuntimePair : AbilityEntitlementRuntimeGrants)
		{
			const FAbilityEntitlementRuntimeGrant& RuntimeGrant = RuntimePair.Value;
			if (RuntimeGrant.Contributors.IsEmpty()
				|| RuntimeGrant.GrantedHandles.AbilitySpecHandles.Num() != RuntimeGrant.ExpectedSpecCanonicalEntries.Num())
			{
				return false;
			}

			TSet<FString> SeenRuntimeClasses;
			for (const FGameplayAbilitySpecHandle& Handle : RuntimeGrant.GrantedHandles.AbilitySpecHandles)
			{
				const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle, EConsiderPending::None);
				if (!Handle.IsValid()
					|| !Spec
					|| Spec->PendingRemove
					|| !Spec->Ability
					|| Spec->SourceObject.Get() != this
					|| OutTrackedHandles.Contains(Handle))
				{
					return false;
				}

				const FString AbilityClassPath = Spec->Ability->GetClass()->GetPathName();
				const FString* ExpectedEntry = RuntimeGrant.ExpectedSpecCanonicalEntries.Find(AbilityClassPath);
				if (!ExpectedEntry
					|| SeenRuntimeClasses.Contains(AbilityClassPath)
					|| OutTrackedClassPaths.Contains(AbilityClassPath)
					|| *ExpectedEntry != BuildAbilitySpecCanonicalEntry(*Spec))
				{
					return false;
				}

				SeenRuntimeClasses.Add(AbilityClassPath);
				OutTrackedHandles.Add(Handle);
				OutTrackedClassPaths.Add(AbilityClassPath);
			}
			if (SeenRuntimeClasses.Num() != RuntimeGrant.ExpectedSpecCanonicalEntries.Num())
			{
				return false;
			}

			for (const FGameplayTag& Contributor : RuntimeGrant.Contributors)
			{
				const FString* ContributorIdentity = AbilityEntitlementToIdentity.Find(Contributor);
				if (!ContributorIdentity || *ContributorIdentity != RuntimePair.Key || SeenContributors.Contains(Contributor))
				{
					return false;
				}
				SeenContributors.Add(Contributor);
			}
		}

		for (const TPair<FGameplayTag, FString>& EntitlementPair : AbilityEntitlementToIdentity)
		{
			const FAbilityEntitlementRuntimeGrant* RuntimeGrant = AbilityEntitlementRuntimeGrants.Find(EntitlementPair.Value);
			if (!RuntimeGrant || !RuntimeGrant->Contributors.Contains(EntitlementPair.Key))
			{
				return false;
			}
		}
		return SeenContributors.Num() == AbilityEntitlementToIdentity.Num();
	};

	auto HasUntrackedClassCollision = [this](const TSet<FGameplayAbilitySpecHandle>& TrackedHandles, const TSet<FString>& TrackedClassPaths)
	{
		for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
		{
			if (Spec.Ability
				&& !Spec.PendingRemove
				&& !TrackedHandles.Contains(Spec.Handle)
				&& TrackedClassPaths.Contains(Spec.Ability->GetClass()->GetPathName()))
			{
				return true;
			}
		}
		return false;
	};
	auto ValidateAllGateLedgers = [this]()
	{
		for (const TPair<FGameplayTag, TSet<FName>>& GatePair : AbilityActivationGateSources)
		{
			if (GatePair.Value.IsEmpty() || GameplayTagCountContainer.GetExplicitTagCount(GatePair.Key) != 1)
			{
				return false;
			}
		}
		return true;
	};

	TSet<FGameplayAbilitySpecHandle> TrackedHandlesBeforeMutation;
	TSet<FString> TrackedClassPathsBeforeMutation;
	if (!ValidateRuntimeProjection(TrackedHandlesBeforeMutation, TrackedClassPathsBeforeMutation)
		|| HasUntrackedClassCollision(TrackedHandlesBeforeMutation, TrackedClassPathsBeforeMutation)
		|| !ValidateAllGateLedgers())
	{
		bAbilityEntitlementProjectionNeedsReset = true;
		UE_LOG(LogSigilAbilitySystem, Error, TEXT("SetAbilityActivationGateSource rejected a gate mutation because the recorded entitlement projection was already inconsistent; an authoritative reset is required."));
		return;
	}
	const TSet<FName>* ExistingGateSources = AbilityActivationGateSources.Find(GateTag);
	const bool bHasGateSources = ExistingGateSources && !ExistingGateSources->IsEmpty();
	const int32 BaselineGateTagCount = GameplayTagCountContainer.GetExplicitTagCount(GateTag);
	if ((ExistingGateSources && ExistingGateSources->IsEmpty())
		|| (bHasGateSources && BaselineGateTagCount != 1)
		|| (!bHasGateSources && BaselineGateTagCount != 0))
	{
		bAbilityEntitlementProjectionNeedsReset = true;
		UE_LOG(LogSigilAbilitySystem, Error, TEXT("SetAbilityActivationGateSource requires exclusive ownership of the gate tag's explicit loose contribution; the source ledger and explicit count disagree, so an authoritative reset is required."));
		return;
	}

	TGuardValue<bool> GateMutationGuard(bAbilityActivationGateMutationInProgress, true);
	const FGameplayAbilityActorInfo* ExpectedActorInfo = ActorInfo;
	const TWeakObjectPtr<AActor> ExpectedOwnerActor = ActorInfo->OwnerActor;
	const TWeakObjectPtr<AActor> ExpectedAvatarActor = ActorInfo->AvatarActor;
	TSet<FGameplayAbilitySpecHandle> BaselineProjectionSourceHandles;
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && !Spec.PendingRemove && Spec.SourceObject.Get() == this)
		{
			BaselineProjectionSourceHandles.Add(Spec.Handle);
		}
	}
	auto FinishGateMutation = [this,
		ExpectedActorInfo,
		ExpectedOwnerActor,
		ExpectedAvatarActor,
		GateTag,
		&BaselineProjectionSourceHandles,
		&ValidateRuntimeProjection,
		&HasUntrackedClassCollision,
		&ValidateAllGateLedgers](int32 ExpectedGateTagCount, bool bBaselineGateCountValid)
	{
		const AActor* OwnerActor = ExpectedOwnerActor.Get();
		const AActor* AvatarActor = ExpectedAvatarActor.Get();
		const bool bContextCurrent = AbilityActorInfo.Get() == ExpectedActorInfo
			&& ExpectedActorInfo->OwnerActor == ExpectedOwnerActor
			&& ExpectedActorInfo->AvatarActor == ExpectedAvatarActor
			&& IsValid(OwnerActor)
			&& IsValid(AvatarActor)
			&& !OwnerActor->IsActorBeingDestroyed()
			&& !AvatarActor->IsActorBeingDestroyed()
			&& IsOwnerActorAuthoritative();

		TSet<FGameplayAbilitySpecHandle> TrackedHandlesAfterMutation;
		TSet<FString> TrackedClassPathsAfterMutation;
		const bool bRuntimeProjectionValid = ValidateRuntimeProjection(TrackedHandlesAfterMutation, TrackedClassPathsAfterMutation)
			&& !HasUntrackedClassCollision(TrackedHandlesAfterMutation, TrackedClassPathsAfterMutation);

		TArray<FGameplayAbilitySpecHandle> UnexpectedOwnedHandles;
		for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
		{
			if (Spec.Ability
				&& !Spec.PendingRemove
				&& Spec.SourceObject.Get() == this
				&& !BaselineProjectionSourceHandles.Contains(Spec.Handle))
			{
				UnexpectedOwnedHandles.Add(Spec.Handle);
			}
		}
		if (!UnexpectedOwnedHandles.IsEmpty())
		{
			FAbilityEntitlementRuntimeGrant& RecoveryGrant = AbilityEntitlementRuntimeGrants.FindOrAdd(TEXT("__Recovery.UntrackedOnGateCallback"));
			for (const FGameplayAbilitySpecHandle& Handle : UnexpectedOwnedHandles)
			{
				if (!RecoveryGrant.GrantedHandles.AbilitySpecHandles.Contains(Handle))
				{
					RecoveryGrant.GrantedHandles.AddAbilitySpecHandle(Handle);
				}
			}
		}
		const bool bGateCountValid = bBaselineGateCountValid
			&& GameplayTagCountContainer.GetExplicitTagCount(GateTag) == ExpectedGateTagCount
			&& ValidateAllGateLedgers();
		if (!bContextCurrent || !bRuntimeProjectionValid || !bGateCountValid || !UnexpectedOwnedHandles.IsEmpty())
		{
			bAbilityEntitlementProjectionNeedsReset = true;
			UE_LOG(LogSigilAbilitySystem, Error, TEXT("SetAbilityActivationGateSource completed its requested gate bookkeeping but a tag delegate changed ASC context, tracked AbilitySpecs, colliding AbilitySpecs, or the expected gate count; an authoritative reset is required."));
		}
	};

	if (bPresent)
	{
		TSet<FName>& Sources = AbilityActivationGateSources.FindOrAdd(GateTag);
		if (!Sources.Contains(SourceId))
		{
			const bool bWasEmpty = Sources.IsEmpty();
			Sources.Add(SourceId);
			if (bWasEmpty)
			{
				AddLooseGameplayTag(GateTag, 1, EGameplayTagReplicationState::TagOnly);
				FinishGateMutation(BaselineGateTagCount + 1, true);
			}
		}
		return;
	}

	if (TSet<FName>* Sources = AbilityActivationGateSources.Find(GateTag))
	{
		if (Sources->Remove(SourceId) > 0 && Sources->IsEmpty())
		{
			AbilityActivationGateSources.Remove(GateTag);
			RemoveLooseGameplayTag(GateTag, 1, EGameplayTagReplicationState::TagOnly);
			FinishGateMutation(FMath::Max(BaselineGateTagCount - 1, 0), BaselineGateTagCount > 0);
		}
	}
}

void USigilAbilitySystemComponent::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (!IsInGameThread() || bAbilityEntitlementReconcileInProgress || bAbilityEntitlementResetInProgress || bAbilityActivationGateMutationInProgress || !InputTag.IsValid())
	{
		return;
	}
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid() || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> MatchingHandles;
	{
		ABILITYLIST_SCOPE_LOCK();
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (!AbilitySpec.PendingRemove && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				MatchingHandles.Add(AbilitySpec.Handle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : MatchingHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(Handle, EConsiderPending::None); AbilitySpec && !AbilitySpec->PendingRemove)
		{
			USigilAbilitySystemFunctionLibrary::SetAbilityInputPressed(this, Handle);
		}
	}
}

void USigilAbilitySystemComponent::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!IsInGameThread() || bAbilityEntitlementReconcileInProgress || bAbilityEntitlementResetInProgress || bAbilityActivationGateMutationInProgress || !InputTag.IsValid())
	{
		return;
	}
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid() || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> MatchingHandles;
	{
		ABILITYLIST_SCOPE_LOCK();
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (!AbilitySpec.PendingRemove && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				MatchingHandles.Add(AbilitySpec.Handle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : MatchingHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(Handle, EConsiderPending::None); AbilitySpec && !AbilitySpec->PendingRemove)
		{
			USigilAbilitySystemFunctionLibrary::SetAbilityInputReleased(this, Handle);
		}
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
