// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Drops/GIS_DropperComponent.h"
#include "Engine/World.h"
#include "GIS_LogChannels.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_DropperComponent)


UGIS_DropperComponent::UGIS_DropperComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGIS_DropperComponent::Drop()
{
}


AActor* UGIS_DropperComponent::CreatePickupActorInstance_Implementation()
{
	UWorld* World = GetWorld();
	check(World);
	if (PickupActorClass.IsNull())
	{
		GIS_CLOG(Error, "missing PickupActorClass!");
		return nullptr;
	}
	UClass* PickupClass = PickupActorClass.LoadSynchronous();
	if (PickupClass == nullptr)
	{
		GIS_CLOG(Error, "failed to load PickupActorClass!");
		return nullptr;
	}

	FVector Origin = CalcDropOrigin();
	AActor* Pickup = World->SpawnActor<AActor>(PickupClass, FTransform(Origin + CalcDropOffset()));
	if (Pickup == nullptr)
	{
		GIS_CLOG(Error, "failed to spawn pickup actor from PickupActorClass(%s)!", *PickupClass->GetName());
		return nullptr;
	}
	return Pickup;
}

FVector UGIS_DropperComponent::CalcDropOrigin_Implementation() const
{
	if (IsValid(DropTransform))
	{
		return DropTransform->GetActorLocation();
	}

	FVector OriginLocation = GetOwner()->GetActorLocation();

	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		OriginLocation.Z -= Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
	else if (const UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(GetOwner()->GetRootComponent()))
	{
		OriginLocation.Z -= CapsuleComponent->GetScaledCapsuleHalfHeight();
	}
	return OriginLocation;
}

FVector UGIS_DropperComponent::CalcDropOffset_Implementation() const
{
	const float RandomX = UKismetMathLibrary::RandomFloatInRange(-DropRadius, DropRadius);
	const float RandomY = UKismetMathLibrary::RandomFloatInRange(-DropRadius, DropRadius);

	return FVector(RandomX, RandomY, 0);
}
