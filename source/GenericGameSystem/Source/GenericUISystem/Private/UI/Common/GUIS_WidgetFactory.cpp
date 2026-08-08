// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Common/GUIS_WidgetFactory.h"
#include "Blueprint/UserWidget.h"
#include "Misc/DataValidation.h"


TSubclassOf<UUserWidget> UGUIS_WidgetFactory::FindWidgetClassForData_Implementation(const UObject* Data) const
{
	return TSubclassOf<UUserWidget>();
}

UGUIS_WidgetFactory::UGUIS_WidgetFactory()
{
}

bool UGUIS_WidgetFactory::OnDataValidation_Implementation(FText& ValidationMessage) const
{
	return true;
}

#if WITH_EDITOR
EDataValidationResult UGUIS_WidgetFactory::IsDataValid(FDataValidationContext& Context) const
{
	FText ValidationMessage;
	if (!OnDataValidation(ValidationMessage))
	{
		Context.AddError(ValidationMessage);
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
