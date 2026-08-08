// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAbilitySystemComponent.h"
#include "SigilCharacter.h"
#include "SigilCharacterWithAbilities.generated.h"

UCLASS()
class SIGILGAS_API ASigilCharacterWithAbilities : public ASigilCharacter
{
	GENERATED_BODY()

public:
	ASigilCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USigilAbilitySystemComponent> AbilitySystemComponent;
};
