// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativePresentation.h"
#include "SigilNarrativePresentationTestTypes.generated.h"

UCLASS()
class USigilNarrativePresentationTestAsset final : public USigilNarrativePresentationAsset
{
	GENERATED_BODY()
};

UCLASS()
class USigilNarrativePresentationTestHost final : public UObject, public ISigilNarrativePresentationHost
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bCanBegin = true;

	UPROPERTY()
	bool bAcceptBegin = true;

	UPROPERTY()
	int32 BeginCount = 0;

	UPROPERTY()
	int32 CancelCount = 0;

	UPROPERTY()
	FSigilNarrativePresentationHandle LastHandle;

	virtual bool CanBeginPresentation_Implementation(
		USigilNarrativePresentationAsset* Presentation,
		FSigilNarrativePresentationHandle Handle,
		UObject* ContextObject) const override;

	virtual bool BeginPresentation_Implementation(
		USigilNarrativePresentationAsset* Presentation,
		FSigilNarrativePresentationHandle Handle,
		UObject* ContextObject) override;

	virtual void CancelPresentation_Implementation(FSigilNarrativePresentationHandle Handle) override;
};
