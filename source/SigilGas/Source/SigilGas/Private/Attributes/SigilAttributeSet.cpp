// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Attributes/SigilAttributeSet.h"
#include "SigilAbilitySystemComponent.h"

USigilAttributeSet::USigilAttributeSet()
{
}

UWorld* USigilAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

USigilAbilitySystemComponent* USigilAttributeSet::GetSigilAbilitySystemComponent() const
{
	return Cast<USigilAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
