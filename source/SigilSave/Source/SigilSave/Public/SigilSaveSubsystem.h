// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SigilSaveSubsystem.generated.h"

UCLASS()
class SIGILSAVE_API USigilSaveSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sigil|Save")
	bool SaveJson(const FString& SlotName, const FString& JsonText, int32 UserIndex = 0) const;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Save")
	bool LoadJson(const FString& SlotName, FString& OutJsonText, int32 UserIndex = 0) const;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Save")
	bool DeleteJson(const FString& SlotName, int32 UserIndex = 0) const;
};
