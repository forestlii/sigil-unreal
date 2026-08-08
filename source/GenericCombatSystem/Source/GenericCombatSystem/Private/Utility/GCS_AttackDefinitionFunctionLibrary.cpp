// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utility/GCS_AttackDefinitionFunctionLibrary.h"

#include "CombatFlow/GCS_AttackDefinition.h"

#if WITH_EDITOR
void UGCS_AttackDefinitionFunctionLibrary::MigrateAttackDefinitionTable(UDataTable* InTable)
{
	if (InTable && InTable->GetRowStruct()->IsChildOf(FGCS_AttackDefinition::StaticStruct()))
	{
		TArray<FGCS_AttackDefinition*> Rows;
		InTable->GetAllRows<FGCS_AttackDefinition>(TEXT("Migration"),Rows);

		InTable->MarkPackageDirty();
	}
}
#endif
