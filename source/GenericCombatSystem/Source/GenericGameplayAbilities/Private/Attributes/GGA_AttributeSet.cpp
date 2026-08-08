// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Attributes/GGA_AttributeSet.h"
#include "GGA_AbilitySystemComponent.h"

UGGA_AttributeSet::UGGA_AttributeSet()
{
}

UWorld* UGGA_AttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UGGA_AbilitySystemComponent* UGGA_AttributeSet::GetGGA_AbilitySystemComponent() const
{
	return Cast<UGGA_AbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
