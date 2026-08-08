// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "GameplayActors/SigilCharacterWithAbilities.h"
#include "SigilAbilitySystemComponent.h"

ASigilCharacterWithAbilities::ASigilCharacterWithAbilities(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<USigilAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* ASigilCharacterWithAbilities::GetAbilitySystemComponent() const
{
	if (UAbilitySystemComponent* BpProvidedASC = CustomGetAbilitySystemComponent())
	{
		return BpProvidedASC;
	}

	return AbilitySystemComponent;
}
