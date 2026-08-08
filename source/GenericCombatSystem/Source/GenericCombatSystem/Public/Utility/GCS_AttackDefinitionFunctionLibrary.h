// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GCS_AttackDefinitionFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class GENERICCOMBATSYSTEM_API UGCS_AttackDefinitionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | DataTable", DisplayName = "MigrateAttackDefinitionTable")
	static void MigrateAttackDefinitionTable(UDataTable* InTable);
#endif
};
