// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/Interface.h"
#include "SigilNarrativePresentation.generated.h"

UENUM(BlueprintType)
enum class ESigilNarrativePresentationResult : uint8
{
	Completed,
	Skipped,
	Cancelled,
	Failed
};

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilNarrativePresentationHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Narrative|Presentation")
	FGuid Id;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Narrative|Presentation")
	int32 Generation = 0;

	bool IsValid() const
	{
		return Id.IsValid() && Generation != 0;
	}

	friend bool operator==(
		const FSigilNarrativePresentationHandle& Left,
		const FSigilNarrativePresentationHandle& Right)
	{
		return Left.Id == Right.Id && Left.Generation == Right.Generation;
	}

	friend bool operator!=(
		const FSigilNarrativePresentationHandle& Left,
		const FSigilNarrativePresentationHandle& Right)
	{
		return !(Left == Right);
	}
};

UCLASS(Abstract, BlueprintType)
class SIGILNARRATIVE_API USigilNarrativePresentationAsset : public UDataAsset
{
	GENERATED_BODY()
};

UINTERFACE(BlueprintType)
class SIGILNARRATIVE_API USigilNarrativePresentationHost : public UInterface
{
	GENERATED_BODY()
};

class SIGILNARRATIVE_API ISigilNarrativePresentationHost
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sigil|Narrative|Presentation")
	bool CanBeginPresentation(
		USigilNarrativePresentationAsset* Presentation,
		FSigilNarrativePresentationHandle Handle,
		UObject* ContextObject) const;
	virtual bool CanBeginPresentation_Implementation(
		USigilNarrativePresentationAsset* Presentation,
		FSigilNarrativePresentationHandle Handle,
		UObject* ContextObject) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sigil|Narrative|Presentation")
	bool BeginPresentation(
		USigilNarrativePresentationAsset* Presentation,
		FSigilNarrativePresentationHandle Handle,
		UObject* ContextObject);
	virtual bool BeginPresentation_Implementation(
		USigilNarrativePresentationAsset* Presentation,
		FSigilNarrativePresentationHandle Handle,
		UObject* ContextObject);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sigil|Narrative|Presentation")
	void CancelPresentation(FSigilNarrativePresentationHandle Handle);
	virtual void CancelPresentation_Implementation(FSigilNarrativePresentationHandle Handle);
};
