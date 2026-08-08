// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Pickups/GIS_PickupComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_PickupComponent)

// Sets default values for this component's properties
UGIS_PickupComponent::UGIS_PickupComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

bool UGIS_PickupComponent::Pickup(UGIS_InventorySystemComponent* Picker)
{
	return true;
}

void UGIS_PickupComponent::NotifyPickupSuccess()
{
	OnPickupSuccess.Broadcast();
}

void UGIS_PickupComponent::NotifyPickupFailed()
{
	OnPickupFail.Broadcast();
}
