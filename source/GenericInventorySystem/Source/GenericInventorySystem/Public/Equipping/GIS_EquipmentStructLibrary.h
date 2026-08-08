// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "GIS_EquipmentStructLibrary.generated.h"

USTRUCT(BlueprintType)
struct GENERICINVENTORYSYSTEM_API FGIS_EquipmentActorToSpawn
{
	GENERATED_BODY()

	FGIS_EquipmentActorToSpawn()
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category=Equipment)
	TSoftClassPtr<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, Category=Equipment)
	bool bShouldAttach{true};

	UPROPERTY(EditAnywhere, Category=Equipment, meta=(EditCondition="bShouldAttach", EditConditionHides))
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category=Equipment, meta=(EditCondition="bShouldAttach", EditConditionHides))
	FTransform AttachTransform;
};
