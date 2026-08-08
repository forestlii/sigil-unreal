// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Bullet/SigilBulletInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "SigilCombatTags.h"
#include "SigilCombatLogChannels.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Bullet/SigilBulletDefinition.h"
#include "Bullet/SigilBulletSubsystem.h"
#include "CombatFlow/SigilAttackDefinition.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/SigilGameplayEffectContainerFunctionLibrary.h"
#include "Utilities/SigilGameplayEffectFunctionLibrary.h"
#include "Utility/SigilCombatFunctionLibrary.h"

// Sets default values
ASigilBulletInstance::ASigilBulletInstance()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
}

void ASigilBulletInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DefinitionHandle, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, BulletId, SharedParams);
}

UProjectileMovementComponent* ASigilBulletInstance::GetProjectileMovementComponent() const
{
	return ProjectileMovement;
}

void ASigilBulletInstance::SetDefinitionHandle(FDataTableRowHandle NewHandle)
{
	if ((GetOwner() != nullptr && GetOwner()->HasAuthority()) || HasAuthority())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DefinitionHandle, this);
		DefinitionHandle = NewHandle;
		ForceNetUpdate();
		OnRep_BulletDefinition();
	}
}

void ASigilBulletInstance::SetBulletId(const FGuid& NewId)
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
		UE_LOG(LogSigilCombat, Error, TEXT("Attempt to set invalid guid for bullet(%s)"), *GetName())
	}
}

FGuid ASigilBulletInstance::GetBulletId() const
{
	return BulletId;
}

void ASigilBulletInstance::SetParentBulletId_Implementation(FGuid NewParentId)
{
	ParentBulletId = NewParentId;
}

FGuid ASigilBulletInstance::GetParentBulletId() const
{
	return ParentBulletId;
}

void ASigilBulletInstance::SetHitResult(const FHitResult& NewHitResult)
{
	LastHitResult = NewHitResult;
	// HitActors.Push(NewHitResult.GetActor());
}

const FHitResult& ASigilBulletInstance::GetHitResult() const
{
	return LastHitResult;
}

bool ASigilBulletInstance::HasGameplayAuthority() const
{
	return HasAuthority() && !bIsLocalPredicting;
}

void ASigilBulletInstance::LaunchBullet_Implementation()
{
}

bool ASigilBulletInstance::GetEffectSpecHandle_Implementation(FGameplayEffectSpecHandle& OutHandle)
{
	OutHandle = EffectSpecHandle;
	return EffectSpecHandle.IsValid();
}

FSigilGameplayEffectContainer ASigilBulletInstance::GetEffectContainer_Implementation() const
{
	if (FSigilAttackDefinition* AtkDef = Definition.AttackDefinition.GetRow<FSigilAttackDefinition>(TEXT("ASigilBulletInstance::GetEffectContainer")))
	{
		return AtkDef->TargetEffectContainer;
	}
	return FSigilGameplayEffectContainer();
}

int32 ASigilBulletInstance::GetEffectContainerLevelOverride_Implementation() const
{
	return 0;
}

void ASigilBulletInstance::SetEffectContainerSpec_Implementation(const FSigilGameplayEffectContainerSpec& InEffectContainerSpec)
{
	EffectContainerSpec = InEffectContainerSpec;
}

void ASigilBulletInstance::SetEffectSpec_Implementation(FGameplayEffectSpecHandle& InEffectSpec)
{
	EffectSpecHandle = InEffectSpec;
}

UShapeComponent* ASigilBulletInstance::GetBulletShape_Implementation() const
{
	return nullptr;
}

FSigilGameplayEffectContainerSpec ASigilBulletInstance::GetEffectContainerSpec_Implementation() const
{
	return EffectContainerSpec;
}

void ASigilBulletInstance::PostNetInit()
{
	Super::PostNetInit();
}

void ASigilBulletInstance::PostNetReceive()
{
	Super::PostNetReceive();
}

void ASigilBulletInstance::FoundLocalPredictedBullet_Implementation(ASigilBulletInstance* PredictedBullet)
{
}

TSubclassOf<UGameplayEffect> ASigilBulletInstance::GetEffectClass_Implementation() const
{
	if (FSigilAttackDefinition* AtkDef = Definition.AttackDefinition.GetRow<FSigilAttackDefinition>(TEXT("ASigilBulletInstance::GetEffectClass")))
	{
		if (!AtkDef->TargetEffectClass.IsNull())
		{
			return AtkDef->TargetEffectClass.LoadSynchronous();
		}
	}
	return nullptr;
}

int32 ASigilBulletInstance::GetEffectLevel_Implementation() const
{
	if (FSigilAttackDefinition* AtkDef = Definition.AttackDefinition.GetRow<FSigilAttackDefinition>(TEXT("ASigilBulletInstance::GetEffectClass")))
	{
		return AtkDef->TargetEffectClassLevel;
	}
	return 0;
}


// Called when the game starts or when spawned
void ASigilBulletInstance::BeginPlay()
{
	Super::BeginPlay();
}

void ASigilBulletInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnBulletEndPlay();
	Super::EndPlay(EndPlayReason);
}

void ASigilBulletInstance::OnBulletBeginPlay_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	SetActorEnableCollision(true);
}

void ASigilBulletInstance::OnBulletEndPlay_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	SetActorEnableCollision(false);
	bIsLocalPredicting = false;
	Definition = FSigilBulletDefinition();
	Request = nullptr;
	EffectSpecHandle = FGameplayEffectSpecHandle();
	EffectContainerSpec = FSigilGameplayEffectContainerSpec();
	SetActorTransform(FTransform::Identity);
}

void ASigilBulletInstance::SetupInitialLocationAndRotation()
{
	InitialActorLocation = GetActorLocation();
	InitialActorRotation = GetActorRotation();
}

void ASigilBulletInstance::RefreshTravelStates()
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

bool ASigilBulletInstance::ShouldPenetrateHitResult(const FHitResult& InHitResult) const
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

bool ASigilBulletInstance::ShouldGenerateBullet_Implementation()
{
	if (Definition.HitBulletDefinition.IsNull() || Definition.HitBulletDefinition == DefinitionHandle)
	{
		return false;
	}
	if (Definition.LaunchCondition == SigilBulletLaunch::Always || Definition.LaunchCondition == FGameplayTag::EmptyTag)
	{
		return true;
	}
	if (Definition.LaunchCondition == SigilBulletLaunch::DidNotHitPawn)
	{
		if (LastHitResult.GetActor() && !LastHitResult.GetActor()->GetClass()->IsChildOf(APawn::StaticClass()))
		{
			return true;
		}
	}
	if (Definition.LaunchCondition == SigilBulletLaunch::HitPawn)
	{
		if (LastHitResult.GetActor() && LastHitResult.GetActor()->GetClass()->IsChildOf(APawn::StaticClass()))
		{
			return true;
		}
	}
	return false;
}


void ASigilBulletInstance::HandleBulletHitChains_Implementation()
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

	FSigilBulletDefinition* SubBullet = Definition.HitBulletDefinition.GetRow<FSigilBulletDefinition>(TEXT("HandleBulletHitChains"));
	if (SubBullet == nullptr)
	{
		return;
	}


	// Setup bullet gameplay effect instance and launch.

	FSigilBulletSpawnParameters SpawnParams;
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

	TArray<ASigilBulletInstance*> BulletInstances = GetWorld()->GetSubsystem<USigilBulletSubsystem>()->SpawnBullets(SpawnParams);

	//Setup each bullets
	for (ASigilBulletInstance* BulletInstance : BulletInstances)
	{
		// Setup normal gameplay effects.
		TSubclassOf<UGameplayEffect> GE = Execute_GetEffectClass(BulletInstance);
		int32 GELevel = Execute_GetEffectLevel(BulletInstance);
		if (GE != nullptr)
		{
			FGameplayEffectSpecHandle GESpec = AbilitySystem->MakeOutgoingSpec(GE, GELevel, AbilitySystem->MakeEffectContext());
			USigilCombatFunctionLibrary::AddAttackHandleToGameplayEffectSpec(GESpec, SubBullet->AttackDefinition);
			FGameplayEffectContextHandle ContextHandle = UAbilitySystemBlueprintLibrary::GetEffectContext(GESpec);
			USigilGameplayEffectFunctionLibrary::SetEffectCauser(ContextHandle, BulletInstance);
			Execute_SetEffectSpec(BulletInstance, GESpec);
		}

		// Setup gameplay effects container.
		const FSigilGameplayEffectContainer& GEContainer = Execute_GetEffectContainer(BulletInstance);

		if (USigilGameplayEffectContainerFunctionLibrary::IsValidContainer(GEContainer))
		{
			FSigilGameplayEffectContainerSpec GEContainerSpec = USigilGameplayEffectContainerFunctionLibrary::MakeEffectContainerSpec(
				GEContainer, EventData);

			// Setup each gameplay effect instance.
			for (const FGameplayEffectSpecHandle& GESpec : GEContainerSpec.TargetGameplayEffectSpecs)
			{
				USigilCombatFunctionLibrary::AddAttackHandleToGameplayEffectSpec(GESpec, SubBullet->AttackDefinition);
				FGameplayEffectContextHandle ContextHandle = UAbilitySystemBlueprintLibrary::GetEffectContext(GESpec);
				USigilGameplayEffectFunctionLibrary::SetEffectCauser(ContextHandle, BulletInstance);
			}
			Execute_SetEffectContainerSpec(BulletInstance, GEContainerSpec);
		}
	}

	//Launch each bullets
	for (ASigilBulletInstance* BulletInstance : BulletInstances)
	{
		BulletInstance->LaunchBullet();
	}
}

void ASigilBulletInstance::ApplyGameplayEffects_Implementation(FHitResult HitResult)
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

		FSigilGameplayEffectContainerSpec ContainerSpec = Execute_GetEffectContainerSpec(this);
		if (ContainerSpec.HasValidEffects())
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
			ContainerSpec.TargetData.Add(NewData);
			for (const FGameplayEffectSpecHandle& TargetGameplayEffectSpec : ContainerSpec.TargetGameplayEffectSpecs)
			{
				TargetGameplayEffectSpec.Data->GetContext().AddHitResult(HitResult, true);
			}
		}
		USigilGameplayEffectContainerFunctionLibrary::ApplyExternalEffectContainerSpec(ContainerSpec);
	}
}

void ASigilBulletInstance::OnRep_BulletId(FGuid Prev)
{
	if (USigilBulletSubsystem* BulletSubsystem = GetWorld()->GetSubsystem<USigilBulletSubsystem>())
	{
		if (!bIsLocalPredicting && BulletSubsystem->BulletInstances.Contains(BulletId))
		{
			UE_LOG(LogSigilCombat, Warning, TEXT("Found local predicted bullet(%s)"), *BulletSubsystem->BulletInstances[BulletId]->GetName());
			FoundLocalPredictedBullet(BulletSubsystem->BulletInstances[BulletId]);
		}
	}
}

void ASigilBulletInstance::OnRep_BulletDefinition()
{
	if (DefinitionHandle.IsNull())
	{
		Definition = FSigilBulletDefinition();
		OnBulletEndPlay();
	}
	else
	{
		if (const FSigilBulletDefinition* NewDefinition = DefinitionHandle.GetRow<FSigilBulletDefinition>(TEXT("RefreshDefinition")))
		{
			Definition = *NewDefinition;
			OnBulletBeginPlay();
		}
		else
		{
			UE_LOG(LogSigilCombat, Verbose, TEXT("Failed to load  definition(%s) for bullet(%s)"), *DefinitionHandle.ToDebugString(), *GetPathName(this));
		}
	}
}

// Called every frame
void ASigilBulletInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
