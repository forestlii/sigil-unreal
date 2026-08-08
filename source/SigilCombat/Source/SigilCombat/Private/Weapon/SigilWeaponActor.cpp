// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Weapon/SigilWeaponActor.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "SigilCombatLogChannels.h"
#include "Collision/SigilCollisionSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

ASigilWeaponActor::ASigilWeaponActor(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bWeaponActive = false;
}

APawn* ASigilWeaponActor::GetWeaponOwner_Implementation() const
{
	return Cast<APawn>(GetOwner());
}

const FGameplayTagContainer ASigilWeaponActor::GetWeaponTags_Implementation() const
{
	return WeaponTags;
}

void ASigilWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bWeaponActive, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SourceObject, Parameters)
}

AActor* ASigilWeaponActor::AsActor_Implementation() const
{
	return GetOwner();
}

void ASigilWeaponActor::SetWeaponActive_Implementation(bool bNewActive)
{
	if (GetOwner()->HasAuthority())
	{
		const bool prev = bWeaponActive;
		bWeaponActive = bNewActive;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bWeaponActive, this);
		OnWeaponActiveStateChanged(prev);
	}
}

bool ASigilWeaponActor::IsWeaponActive_Implementation() const
{
	return bWeaponActive;
}

UPrimitiveComponent* ASigilWeaponActor::GetPrimitiveComponent_Implementation()
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
			UE_LOG(LogSigilCombat, Warning, TEXT("Failed to find weapon mesh via tag (%s) on actor(%s)."), *WeaponMeshTagName.ToString(), *GetOwner()->GetName());
		}
	}

	return CachedPrimitiveComponent;
}

UObject* ASigilWeaponActor::GetSourceObject_Implementation() const
{
	return SourceObject;
}

void ASigilWeaponActor::SetSourceObject_Implementation(UObject* NewSourceObject)
{
	if (GetOwner()->HasAuthority())
	{
		const UObject* prev = SourceObject;
		SourceObject = NewSourceObject;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, SourceObject, this);
		OnSourceObjectChanged(prev);
	}
}

void ASigilWeaponActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = ISigilWeaponInterface::Execute_GetWeaponTags(this);
}

void ASigilWeaponActor::OnWeaponActiveStateChanged_Implementation(bool Prev)
{
	OnWeaponActiveStateChangedEvent.Broadcast(bWeaponActive);
}

void ASigilWeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASigilWeaponActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bWeaponActive)
	{
		bWeaponActive = false;
		RefreshTraceInstance();
	}
	Super::EndPlay(EndPlayReason);
}

void ASigilWeaponActor::OnSourceObjectChanged_Implementation(const UObject* Prev)
{
}

void ASigilWeaponActor::RefreshTraceInstance_Implementation()
{
	if (APawn* Pawn = Execute_GetWeaponOwner(this))
	{
		if (UPrimitiveComponent* Primitive = Execute_GetPrimitiveComponent(this))
		{
			if (USigilCollisionSystemComponent* CSC = USigilCollisionSystemComponent::GetCollisionSystemComponent(Pawn))
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
					for (TObjectPtr<USigilCollisionTraceInstance> Instance : TraceInstances)
					{
						CSC->RemoveTraceFromCreatedTraces(Instance);
					}
					TraceInstances.Empty();
				}
			}
		}
	}
}

void ASigilWeaponActor::OnAnyTraceHit_Implementation(USigilCollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
}

void ASigilWeaponActor::OnAnyTraceStateChanged_Implementation(USigilCollisionTraceInstance* TraceInstance, bool NewState)
{
}

void ASigilWeaponActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
