// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIPS_InputConfig.h"



#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#include "Misc/DataValidation.h"

void UGIPS_InputConfig::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}

EDataValidationResult UGIPS_InputConfig::IsDataValid(FDataValidationContext& Context) const
{
	if (InputActionMappings.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("InputActionMappings can't be empty!")));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
