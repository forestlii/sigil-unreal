// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GMS_MovementSystemComponent.h"
#include "GMS_MoverMovementSystemComponent.generated.h"

/**
 * WIP You should not use this class. Work will be continued on 5.5
 */
UCLASS(Abstract, ClassGroup=GMS, meta=(BlueprintSpawnableComponent), DisplayName="GMS Movement System Component(Mover)")
class GENERICMOVEMENTSYSTEM_API UGMS_MoverMovementSystemComponent : public UGMS_MovementSystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGMS_MoverMovementSystemComponent();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
