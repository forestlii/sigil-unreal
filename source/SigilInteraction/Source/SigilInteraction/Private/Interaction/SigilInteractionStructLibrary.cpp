// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/SigilInteractionStructLibrary.h"
#include "Interaction/SigilInteractionDefinition.h"

FString FSigilInteractionOption::ToString() const
{
	return FString::Format(TEXT("{0} {1} {2}"), {
		                       Definition ? Definition->Text.ToString() : TEXT("Null Definition"), SlotState == ESmartObjectSlotState::Free ? TEXT("Valid") : TEXT("Invalid"), SlotIndex
	                       });
}

bool operator==(const FSigilInteractionOption& Lhs, const FSigilInteractionOption& RHS)
{
	return Lhs.Definition == RHS.Definition
		&& Lhs.RequestResult == RHS.RequestResult
		&& Lhs.SlotIndex == RHS.SlotIndex
		&& Lhs.SlotState == RHS.SlotState;
}

bool operator!=(const FSigilInteractionOption& Lhs, const FSigilInteractionOption& RHS)
{
	return !(Lhs == RHS);
}

bool operator<(const FSigilInteractionOption& Lhs, const FSigilInteractionOption& RHS)
{
	return Lhs.SlotIndex < RHS.SlotIndex;
}
