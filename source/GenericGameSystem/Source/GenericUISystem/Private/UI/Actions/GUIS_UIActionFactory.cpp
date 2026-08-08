// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Actions/GUIS_UIActionFactory.h"
#include "Misc/DataValidation.h"
#include "UI/Actions/GUIS_UIAction.h"

TArray<UGUIS_UIAction*> UGUIS_UIActionFactory::FindAvailableUIActionsForData(const UObject* Data) const
{
	TArray<UGUIS_UIAction*> Ret;
	for (UGUIS_UIAction* Action : PotentialActions)
	{
		if (Action != nullptr && Action->IsCompatible(Data))
		{
			Ret.Add(Action);
		}
	}
	return Ret;
}


#if WITH_EDITOR
EDataValidationResult UGUIS_UIActionFactory::IsDataValid(FDataValidationContext& Context) const
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
