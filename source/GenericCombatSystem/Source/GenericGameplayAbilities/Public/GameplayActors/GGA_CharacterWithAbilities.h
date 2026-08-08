// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GGA_AbilitySystemComponent.h"
#include "GGA_Character.h"
#include "GGA_CharacterWithAbilities.generated.h"

UCLASS()
class GENERICGAMEPLAYABILITIES_API AGGA_CharacterWithAbilities : public AGGA_Character
{
	GENERATED_BODY()

public:
	AGGA_CharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UGGA_AbilitySystemComponent> AbilitySystemComponent;
};
