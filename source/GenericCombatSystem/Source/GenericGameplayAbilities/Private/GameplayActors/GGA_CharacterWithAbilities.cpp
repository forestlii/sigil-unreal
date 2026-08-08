// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GameplayActors/GGA_CharacterWithAbilities.h"
#include "GGA_AbilitySystemComponent.h"

AGGA_CharacterWithAbilities::AGGA_CharacterWithAbilities(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UGGA_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* AGGA_CharacterWithAbilities::GetAbilitySystemComponent() const
{
	if (UAbilitySystemComponent* BpProvidedASC = CustomGetAbilitySystemComponent())
	{
		return BpProvidedASC;
	}

	return AbilitySystemComponent;
}
