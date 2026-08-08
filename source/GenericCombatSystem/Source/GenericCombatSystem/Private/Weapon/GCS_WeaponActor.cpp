// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Weapon/GCS_WeaponActor.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "GCS_LogChannels.h"
#include "Collision/GCS_CollisionSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

AGCS_WeaponActor::AGCS_WeaponActor(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bWeaponActive = false;
}

APawn* AGCS_WeaponActor::GetWeaponOwner_Implementation() const
{
	return Cast<APawn>(GetOwner());
}

const FGameplayTagContainer AGCS_WeaponActor::GetWeaponTags_Implementation() const
{
	return WeaponTags;
}

void AGCS_WeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bWeaponActive, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SourceObject, Parameters)
}

AActor* AGCS_WeaponActor::AsActor_Implementation() const
{
	return GetOwner();
}

void AGCS_WeaponActor::SetWeaponActive_Implementation(bool bNewActive)
{
	if (GetOwner()->HasAuthority())
	{
		const bool prev = bWeaponActive;
		bWeaponActive = bNewActive;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bWeaponActive, this);
		OnWeaponActiveStateChanged(prev);
	}
}

bool AGCS_WeaponActor::IsWeaponActive_Implementation() const
{
	return bWeaponActive;
}

UPrimitiveComponent* AGCS_WeaponActor::GetPrimitiveComponent_Implementation()
{
	if (CachedPrimitiveComponent)
	{
		return CachedPrimitiveComponent;
	}

	if (WeaponMeshTagName.IsValid())
	{
		CachedPrimitiveComponent = Cast<UPrimitiveComponent>(GetOwner()->FindComponentByTag(UPrimitiveComponent::StaticClass(), WeaponMeshTagName));
		if (!IsValid(CachedPrimitiveComponent))
		{
			UE_LOG(LogGCS, Warning, TEXT("Failed to find weapon mesh via tag (%s) on actor(%s)."), *WeaponMeshTagName.ToString(), *GetOwner()->GetName());
		}
	}

	return CachedPrimitiveComponent;
}

UObject* AGCS_WeaponActor::GetSourceObject_Implementation() const
{
	return SourceObject;
}

void AGCS_WeaponActor::SetSourceObject_Implementation(UObject* NewSourceObject)
{
	if (GetOwner()->HasAuthority())
	{
		const UObject* prev = SourceObject;
		SourceObject = NewSourceObject;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, SourceObject, this);
		OnSourceObjectChanged(prev);
	}
}

void AGCS_WeaponActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = IGCS_WeaponInterface::Execute_GetWeaponTags(this);
}

void AGCS_WeaponActor::OnWeaponActiveStateChanged_Implementation(bool Prev)
{
	OnWeaponActiveStateChangedEvent.Broadcast(bWeaponActive);
}

void AGCS_WeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

void AGCS_WeaponActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bWeaponActive)
	{
		bWeaponActive = false;
		RefreshTraceInstance();
	}
	Super::EndPlay(EndPlayReason);
}

void AGCS_WeaponActor::OnSourceObjectChanged_Implementation(const UObject* Prev)
{
}

void AGCS_WeaponActor::RefreshTraceInstance_Implementation()
{
	if (APawn* Pawn = Execute_GetWeaponOwner(this))
	{
		if (UPrimitiveComponent* Primitive = Execute_GetPrimitiveComponent(this))
		{
			if (UGCS_CollisionSystemComponent* CSC = UGCS_CollisionSystemComponent::GetCollisionSystemComponent(Pawn))
			{
				if (bWeaponActive)
				{
					TraceInstances = CSC->CreateTraceInstances(TraceDefinitions, Primitive);
					CSC->OnTraceInstanceHitEvent.AddDynamic(this, &ThisClass::OnAnyTraceHit);
					CSC->OnTraceInstanceStateChangedEvent.AddDynamic(this, &ThisClass::OnAnyTraceStateChanged);
				}
				else
				{
					CSC->OnTraceInstanceHitEvent.RemoveDynamic(this, &ThisClass::OnAnyTraceHit);
					CSC->OnTraceInstanceStateChangedEvent.RemoveDynamic(this, &ThisClass::OnAnyTraceStateChanged);
					for (TObjectPtr<UGCS_CollisionTraceInstance> Instance : TraceInstances)
					{
						CSC->RemoveTraceFromCreatedTraces(Instance);
					}
					TraceInstances.Empty();
				}
			}
		}
	}
}

void AGCS_WeaponActor::OnAnyTraceHit_Implementation(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
}

void AGCS_WeaponActor::OnAnyTraceStateChanged_Implementation(UGCS_CollisionTraceInstance* TraceInstance, bool NewState)
{
}

void AGCS_WeaponActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
