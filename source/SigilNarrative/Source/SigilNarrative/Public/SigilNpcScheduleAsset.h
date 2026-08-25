// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SigilNpcScheduleAsset.generated.h"

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilNpcScheduleEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative|NPC Schedule", meta = (ClampMin = "0", ClampMax = "1439"))
	int32 StartMinute = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative|NPC Schedule")
	FName ActivityId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative|NPC Schedule")
	FName LocationId;
};

UCLASS(BlueprintType)
class SIGILNARRATIVE_API USigilNpcScheduleAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative|NPC Schedule")
	TArray<FSigilNpcScheduleEntry> Entries;

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative|NPC Schedule")
	bool ResolveAtMinute(int32 MinuteOfDay, FSigilNpcScheduleEntry& OutEntry) const;
};
