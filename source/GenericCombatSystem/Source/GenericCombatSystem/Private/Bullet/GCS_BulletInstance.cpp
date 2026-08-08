// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Bullet/GCS_BulletInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GCS_GameplayTags.h"
#include "GCS_LogChannels.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Bullet/GCS_BulletDefinition.h"
#include "Bullet/GCS_BulletSubsystem.h"
#include "CombatFlow/GCS_AttackDefinition.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/GGA_GameplayEffectContainerFunctionLibrary.h"
#include "Utilities/GGA_GameplayEffectFunctionLibrary.h"
#include "Utility/GCS_CombatFunctionLibrary.h"

// Sets default values
AGCS_BulletInstance::AGCS_BulletInstance()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
}

void AGCS_BulletInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DefinitionHandle, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, BulletId, SharedParams);
}

UProjectileMovementComponent* AGCS_BulletInstance::GetProjectileMovementComponent() const
{
	return ProjectileMovement;
}

void AGCS_BulletInstance::SetDefinitionHandle(FDataTableRowHandle NewHandle)
{
	if ((GetOwner() != nullptr && GetOwner()->HasAuthority()) || HasAuthority())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DefinitionHandle, this);
		DefinitionHandle = NewHandle;
		ForceNetUpdate();
		OnRep_BulletDefinition();
	}
}

void AGCS_BulletInstance::SetBulletId(const FGuid& NewId)
{
	if (NewId.IsValid())
	{
		if ((GetOwner() != nullptr && GetOwner()->HasAuthority()) || HasAuthority())
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DefinitionHandle, this);
		}
		BulletId = NewId;
	}
	else
	{
		UE_LOG(LogGCS, Error, TEXT("Attempt to set invalid guid for bullet(%s)"), *GetName())
	}
}

FGuid AGCS_BulletInstance::GetBulletId() const
{
	return BulletId;
}

void AGCS_BulletInstance::SetParentBulletId_Implementation(FGuid NewParentId)
{
	ParentBulletId = NewParentId;
}

FGuid AGCS_BulletInstance::GetParentBulletId() const
{
	return ParentBulletId;
}

void AGCS_BulletInstance::SetHitResult(const FHitResult& NewHitResult)
{
	LastHitResult = NewHitResult;
	// HitActors.Push(NewHitResult.GetActor());
}

const FHitResult& AGCS_BulletInstance::GetHitResult() const
{
	return LastHitResult;
}

bool AGCS_BulletInstance::HasGameplayAuthority() const
{
	return HasAuthority() && !bIsLocalPredicting;
}

void AGCS_BulletInstance::LaunchBullet_Implementation()
{
}

bool AGCS_BulletInstance::GetEffectSpecHandle_Implementation(FGameplayEffectSpecHandle& OutHandle)
{
	OutHandle = EffectSpecHandle;
	return EffectSpecHandle.IsValid();
}

FGGA_GameplayEffectContainer AGCS_BulletInstance::GetEffectContainer_Implementation() const
{
	if (FGCS_AttackDefinition* AtkDef = Definition.AttackDefinition.GetRow<FGCS_AttackDefinition>(TEXT("AGCS_BulletInstance::GetEffectContainer")))
	{
		return AtkDef->TargetEffectContainer;
	}
	return FGGA_GameplayEffectContainer();
}

int32 AGCS_BulletInstance::GetEffectContainerLevelOverride_Implementation() const
{
	return 0;
}

void AGCS_BulletInstance::SetEffectContainerSpec_Implementation(const FGGA_GameplayEffectContainerSpec& InEffectContainerSpec)
{
	EffectContainerSpec = InEffectContainerSpec;
}

void AGCS_BulletInstance::SetEffectSpec_Implementation(FGameplayEffectSpecHandle& InEffectSpec)
{
	EffectSpecHandle = InEffectSpec;
}

UShapeComponent* AGCS_BulletInstance::GetBulletShape_Implementation() const
{
	return nullptr;
}

FGGA_GameplayEffectContainerSpec AGCS_BulletInstance::GetEffectContainerSpec_Implementation() const
{
	return EffectContainerSpec;
}

void AGCS_BulletInstance::PostNetInit()
{
	Super::PostNetInit();
}

void AGCS_BulletInstance::PostNetReceive()
{
	Super::PostNetReceive();
}

void AGCS_BulletInstance::FoundLocalPredictedBullet_Implementation(AGCS_BulletInstance* PredictedBullet)
{
}

TSubclassOf<UGameplayEffect> AGCS_BulletInstance::GetEffectClass_Implementation() const
{
	if (FGCS_AttackDefinition* AtkDef = Definition.AttackDefinition.GetRow<FGCS_AttackDefinition>(TEXT("AGCS_BulletInstance::GetEffectClass")))
	{
		if (!AtkDef->TargetEffectClass.IsNull())
		{
			return AtkDef->TargetEffectClass.LoadSynchronous();
		}
	}
	return nullptr;
}

int32 AGCS_BulletInstance::GetEffectLevel_Implementation() const
{
	if (FGCS_AttackDefinition* AtkDef = Definition.AttackDefinition.GetRow<FGCS_AttackDefinition>(TEXT("AGCS_BulletInstance::GetEffectClass")))
	{
		return AtkDef->TargetEffectClassLevel;
	}
	return 0;
}


// Called when the game starts or when spawned
void AGCS_BulletInstance::BeginPlay()
{
	Super::BeginPlay();
}

void AGCS_BulletInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnBulletEndPlay();
	Super::EndPlay(EndPlayReason);
}

void AGCS_BulletInstance::OnBulletBeginPlay_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	SetActorEnableCollision(true);
}

void AGCS_BulletInstance::OnBulletEndPlay_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	SetActorEnableCollision(false);
	bIsLocalPredicting = false;
	Definition = FGCS_BulletDefinition();
	Request = nullptr;
	EffectSpecHandle = FGameplayEffectSpecHandle();
	EffectContainerSpec = FGGA_GameplayEffectContainerSpec();
	SetActorTransform(FTransform::Identity);
}

void AGCS_BulletInstance::SetupInitialLocationAndRotation()
{
	InitialActorLocation = GetActorLocation();
	InitialActorRotation = GetActorRotation();
}

void AGCS_BulletInstance::RefreshTravelStates()
{
	if (HasAuthority() && GetProjectileMovementComponent() && GetProjectileMovementComponent()->IsActive())
	{
		// update Traveled distance and gravity scale.
		TraveledDistance = FVector::Dist2D(GetActorLocation(), InitialActorLocation);
		float DesiredGravityScale = TraveledDistance <= Definition.AttenuationRange ? Definition.GravityScaleInRange : Definition.GravityScaleOutRage;
		if (GetProjectileMovementComponent()->ProjectileGravityScale != DesiredGravityScale)
		{
			GetProjectileMovementComponent()->ProjectileGravityScale = DesiredGravityScale;
		}
	}
}

// bool AGCS_BulletInstance::AlreadyHit(const FHitResult& InHitResult) const
// {
// 	for (int i = 0; i < HitActors.Num(); ++i)
// 	{
// 		if (HitActors[i] == InHitResult.GetActor())
// 		{
// 			return true;
// 		}
// 	}
// 	return false;
// }

bool AGCS_BulletInstance::ShouldPenetrateHitResult(const FHitResult& InHitResult) const
{
	if (InHitResult.GetActor() != nullptr)
	{
		if (Definition.bPenetrateCharacter && InHitResult.GetActor()->GetClass()->IsChildOf(APawn::StaticClass()))
		{
			return true;
		}
		return Definition.bPenetrateMap;
	}
	return false;
}

bool AGCS_BulletInstance::ShouldGenerateBullet_Implementation()
{
	if (Definition.HitBulletDefinition.IsNull() || Definition.HitBulletDefinition == DefinitionHandle)
	{
		return false;
	}
	if (Definition.LaunchCondition == GCS_BulletLaunch::Always || Definition.LaunchCondition == FGameplayTag::EmptyTag)
	{
		return true;
	}
	if (Definition.LaunchCondition == GCS_BulletLaunch::DidNotHitPawn)
	{
		if (LastHitResult.GetActor() && !LastHitResult.GetActor()->GetClass()->IsChildOf(APawn::StaticClass()))
		{
			return true;
		}
	}
	if (Definition.LaunchCondition == GCS_BulletLaunch::HitPawn)
	{
		if (LastHitResult.GetActor() && LastHitResult.GetActor()->GetClass()->IsChildOf(APawn::StaticClass()))
		{
			return true;
		}
	}
	return false;
}


void AGCS_BulletInstance::HandleBulletHitChains_Implementation()
{
	if (!HasGameplayAuthority() || !ShouldGenerateBullet())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (AbilitySystem == nullptr)
	{
		return;
	}

	FGCS_BulletDefinition* SubBullet = Definition.HitBulletDefinition.GetRow<FGCS_BulletDefinition>(TEXT("HandleBulletHitChains"));
	if (SubBullet == nullptr)
	{
		return;
	}


	// Setup bullet gameplay effect instance and launch.

	FGCS_BulletSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.DefinitionHandle = Definition.HitBulletDefinition;

	//TODO Various different launch location.
	FTransform SpawnTransform = FTransform::Identity;
	SpawnTransform.SetLocation(GetHitResult().Location);
	SpawnTransform.SetRotation(GetActorRotation().Quaternion());
	SpawnTransform.SetScale3D(FVector::One());
	SpawnParams.SpawnTransform = SpawnTransform;
	SpawnParams.Request = Request;
	SpawnParams.ParentId = BulletId;

	FGameplayEventData EventData;
	EventData.Instigator = GetOwner();
	EventData.EventMagnitude = GetEffectContainerLevelOverride_Implementation();

	TArray<AGCS_BulletInstance*> BulletInstances = GetWorld()->GetSubsystem<UGCS_BulletSubsystem>()->SpawnBullets(SpawnParams);

	//Setup each bullets
	for (AGCS_BulletInstance* BulletInstance : BulletInstances)
	{
		// Setup normal gameplay effects.
		TSubclassOf<UGameplayEffect> GE = Execute_GetEffectClass(BulletInstance);
		int32 GELevel = Execute_GetEffectLevel(BulletInstance);
		if (GE != nullptr)
		{
			FGameplayEffectSpecHandle GESpec = AbilitySystem->MakeOutgoingSpec(GE, GELevel, AbilitySystem->MakeEffectContext());
			UGCS_CombatFunctionLibrary::AddAttackHandleToGameplayEffectSpec(GESpec, SubBullet->AttackDefinition);
			FGameplayEffectContextHandle ContextHandle = UAbilitySystemBlueprintLibrary::GetEffectContext(GESpec);
			UGGA_GameplayEffectFunctionLibrary::SetEffectCauser(ContextHandle, BulletInstance);
			Execute_SetEffectSpec(BulletInstance, GESpec);
		}

		// Setup gameplay effects container.
		const FGGA_GameplayEffectContainer& GEContainer = Execute_GetEffectContainer(BulletInstance);

		if (UGGA_GameplayEffectContainerFunctionLibrary::IsValidContainer(GEContainer))
		{
			FGGA_GameplayEffectContainerSpec GEContainerSpec = UGGA_GameplayEffectContainerFunctionLibrary::MakeEffectContainerSpec(
				GEContainer, EventData);

			// Setup each gameplay effect instance.
			for (const FGameplayEffectSpecHandle& GESpec : GEContainerSpec.TargetGameplayEffectSpecs)
			{
				UGCS_CombatFunctionLibrary::AddAttackHandleToGameplayEffectSpec(GESpec, SubBullet->AttackDefinition);
				FGameplayEffectContextHandle ContextHandle = UAbilitySystemBlueprintLibrary::GetEffectContext(GESpec);
				UGGA_GameplayEffectFunctionLibrary::SetEffectCauser(ContextHandle, BulletInstance);
			}
			Execute_SetEffectContainerSpec(BulletInstance, GEContainerSpec);
		}
	}

	//Launch each bullets
	for (AGCS_BulletInstance* BulletInstance : BulletInstances)
	{
		BulletInstance->LaunchBullet();
	}
}

void AGCS_BulletInstance::ApplyGameplayEffects_Implementation(FHitResult HitResult)
{
	if (!HasGameplayAuthority())
	{
		return;
	}
	if (UAbilitySystemComponent* TargetAsc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult.GetActor()))
	{
		FGameplayEffectSpecHandle SpecHandle;
		if (Execute_GetEffectSpecHandle(this, SpecHandle))
		{
			FGameplayEffectContextHandle ContextHandle = UAbilitySystemBlueprintLibrary::GetEffectContext(SpecHandle);
			ContextHandle.AddHitResult(HitResult, true);
			ContextHandle.GetInstigatorAbilitySystemComponent()->BP_ApplyGameplayEffectSpecToTarget(SpecHandle, TargetAsc);
		}

		FGGA_GameplayEffectContainerSpec ContainerSpec = Execute_GetEffectContainerSpec(this);
		if (ContainerSpec.HasValidEffects())
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
			ContainerSpec.TargetData.Add(NewData);
			for (const FGameplayEffectSpecHandle& TargetGameplayEffectSpec : ContainerSpec.TargetGameplayEffectSpecs)
			{
				TargetGameplayEffectSpec.Data->GetContext().AddHitResult(HitResult, true);
			}
		}
		UGGA_GameplayEffectContainerFunctionLibrary::ApplyExternalEffectContainerSpec(ContainerSpec);
	}
}

void AGCS_BulletInstance::OnRep_BulletId(FGuid Prev)
{
	if (UGCS_BulletSubsystem* BulletSubsystem = GetWorld()->GetSubsystem<UGCS_BulletSubsystem>())
	{
		if (!bIsLocalPredicting && BulletSubsystem->BulletInstances.Contains(BulletId))
		{
			UE_LOG(LogGCS, Warning, TEXT("Found local predicted bullet(%s)"), *BulletSubsystem->BulletInstances[BulletId]->GetName());
			FoundLocalPredictedBullet(BulletSubsystem->BulletInstances[BulletId]);
		}
	}
}

void AGCS_BulletInstance::OnRep_BulletDefinition()
{
	if (DefinitionHandle.IsNull())
	{
		Definition = FGCS_BulletDefinition();
		OnBulletEndPlay();
	}
	else
	{
		if (const FGCS_BulletDefinition* NewDefinition = DefinitionHandle.GetRow<FGCS_BulletDefinition>(TEXT("RefreshDefinition")))
		{
			Definition = *NewDefinition;
			OnBulletBeginPlay();
		}
		else
		{
			UE_LOG(LogGCS, Verbose, TEXT("Failed to load  definition(%s) for bullet(%s)"), *DefinitionHandle.ToDebugString(), *GetPathName(this));
		}
	}
}

// Called every frame
void AGCS_BulletInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
