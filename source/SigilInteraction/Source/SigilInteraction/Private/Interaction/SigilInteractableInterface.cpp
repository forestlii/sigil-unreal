// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/SigilInteractableInterface.h"


// Add default functionality here for any ISigilInteractableInterface functions that are not pure virtual.

FText ISigilInteractableInterface::GetInteractionDisplayNameText_Implementation() const
{
	if (UObject* Object = _getUObject())
	{
		return FText::FromString(GetNameSafe(Object));
	}
	return FText::GetEmpty();
}
