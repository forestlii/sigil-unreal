// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilMoverMovementSystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilMoverMovementSystemComponent)

// Sets default values for this component's properties
USigilMoverMovementSystemComponent::USigilMoverMovementSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USigilMoverMovementSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void USigilMoverMovementSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
