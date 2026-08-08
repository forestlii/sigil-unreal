// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Actions/SigilUIActionFactory.h"
#include "Misc/DataValidation.h"
#include "UI/Actions/SigilUIAction.h"

TArray<USigilUIAction*> USigilUIActionFactory::FindAvailableUIActionsForData(const UObject* Data) const
{
	TArray<USigilUIAction*> Ret;
	for (USigilUIAction* Action : PotentialActions)
	{
		if (Action != nullptr && Action->IsCompatible(Data))
		{
			Ret.Add(Action);
		}
	}
	return Ret;
}


#if WITH_EDITOR
EDataValidationResult USigilUIActionFactory::IsDataValid(FDataValidationContext& Context) const
{
	FText ValidationMessage;
	for (int32 i = 0; i < PotentialActions.Num(); i++)
	{
		if (PotentialActions[i] == nullptr)
		{
			Context.AddError(FText::FromString(FString::Format(TEXT("Invalid action on index:{0}"), {i})));
			return EDataValidationResult::Invalid;
		}
	}
	return Super::IsDataValid(Context);
}
#endif
