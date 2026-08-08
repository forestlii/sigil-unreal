// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilMovementSystemComponent.h"
#include "SigilMoverMovementSystemComponent.generated.h"

/**
 * WIP You should not use this class. Work will be continued on 5.5
 */
UCLASS(Abstract, ClassGroup=GMS, meta=(BlueprintSpawnableComponent), DisplayName="GMS Movement System Component(Mover)")
class SIGILMOVEMENT_API USigilMoverMovementSystemComponent : public USigilMovementSystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USigilMoverMovementSystemComponent();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
