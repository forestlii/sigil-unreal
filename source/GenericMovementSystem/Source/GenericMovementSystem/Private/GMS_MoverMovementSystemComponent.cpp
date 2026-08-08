// Copyright 2024 RedMoonGames All Rights Reserved.


#include "GMS_MoverMovementSystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_MoverMovementSystemComponent)

// Sets default values for this component's properties
UGMS_MoverMovementSystemComponent::UGMS_MoverMovementSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGMS_MoverMovementSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UGMS_MoverMovementSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
