// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SigilJsonSaveGame.generated.h"

UCLASS()
class USigilJsonSaveGame final : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	FString JsonText;
};
