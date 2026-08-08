// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Common/SigilWidgetFactory.h"
#include "Blueprint/UserWidget.h"
#include "Misc/DataValidation.h"


TSubclassOf<UUserWidget> USigilWidgetFactory::FindWidgetClassForData_Implementation(const UObject* Data) const
{
	return TSubclassOf<UUserWidget>();
}

USigilWidgetFactory::USigilWidgetFactory()
{
}

bool USigilWidgetFactory::OnDataValidation_Implementation(FText& ValidationMessage) const
{
	return true;
}

#if WITH_EDITOR
EDataValidationResult USigilWidgetFactory::IsDataValid(FDataValidationContext& Context) const
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
