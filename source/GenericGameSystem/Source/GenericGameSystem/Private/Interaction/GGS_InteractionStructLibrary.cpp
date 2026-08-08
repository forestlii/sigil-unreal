// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Interaction/GGS_InteractionStructLibrary.h"
#include "Interaction/GGS_InteractionDefinition.h"

FString FGGS_InteractionOption::ToString() const
{
	return FString::Format(TEXT("{0} {1} {2}"), {
		                       Definition ? Definition->Text.ToString() : TEXT("Null Definition"), SlotState == ESmartObjectSlotState::Free ? TEXT("Valid") : TEXT("Invalid"), SlotIndex
	                       });
}

bool operator==(const FGGS_InteractionOption& Lhs, const FGGS_InteractionOption& RHS)
{
	return Lhs.Definition == RHS.Definition
		&& Lhs.RequestResult == RHS.RequestResult
		&& Lhs.SlotIndex == RHS.SlotIndex
		&& Lhs.SlotState == RHS.SlotState;
}

bool operator!=(const FGGS_InteractionOption& Lhs, const FGGS_InteractionOption& RHS)
{
	return !(Lhs == RHS);
}

bool operator<(const FGGS_InteractionOption& Lhs, const FGGS_InteractionOption& RHS)
{
	return Lhs.SlotIndex < RHS.SlotIndex;
}
