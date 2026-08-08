// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilInputConfig.h"



#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#include "Misc/DataValidation.h"

void USigilInputConfig::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}

EDataValidationResult USigilInputConfig::IsDataValid(FDataValidationContext& Context) const
{
	if (InputActionMappings.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("InputActionMappings can't be empty!")));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
