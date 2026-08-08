// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utility/SigilAttackDefinitionFunctionLibrary.h"

#include "CombatFlow/SigilAttackDefinition.h"

#if WITH_EDITOR
void USigilAttackDefinitionFunctionLibrary::MigrateAttackDefinitionTable(UDataTable* InTable)
{
	if (InTable && InTable->GetRowStruct()->IsChildOf(FSigilAttackDefinition::StaticStruct()))
	{
		TArray<FSigilAttackDefinition*> Rows;
		InTable->GetAllRows<FSigilAttackDefinition>(TEXT("Migration"),Rows);

		InTable->MarkPackageDirty();
	}
}
#endif
