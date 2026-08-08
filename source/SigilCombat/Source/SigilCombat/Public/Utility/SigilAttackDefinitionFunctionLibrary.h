// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilAttackDefinitionFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SIGILCOMBAT_API USigilAttackDefinitionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | DataTable", DisplayName = "MigrateAttackDefinitionTable")
	static void MigrateAttackDefinitionTable(UDataTable* InTable);
#endif
};
