// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilTargetingSelectionTask_TraceExt.h"
#include "SigilTargetingSelectionTask_TraceExt_BindShape.generated.h"


class UShapeComponent;

UENUM(BlueprintType)
enum class ESigilTraceDataModifyType :uint8
{
	None UMETA(DisplayName="None"),
	Add UMETA(DisplayName="Add"),
	Multiply UMETA(DisplayName = "Multiply"),
};

/**
 * 
 */
UCLASS(meta=(DisplayName="GCS:SelectionTask (Trace Bind Shape)"))
class SIGILCOMBAT_API USigilTargetingSelectionTask_TraceExt_BindShape : public USigilTargetingSelectionTask_TraceExt
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Trace Data")
	ESigilTraceDataModifyType SweptTraceRadiusModType{ESigilTraceDataModifyType::None};

	virtual float GetSweptTraceRadius_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Swept Data")
	ESigilTraceDataModifyType SweptTraceCapsuleHalfHeightModType{ESigilTraceDataModifyType::None};

	virtual float GetSweptTraceCapsuleHalfHeight_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Swept Data")
	ESigilTraceDataModifyType SweptTraceBoxHalfExtentModType{ESigilTraceDataModifyType::None};

	virtual FVector GetSweptTraceBoxHalfExtents_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;
	
	virtual UShapeComponent* GetTraceShape(const FTargetingRequestHandle& TargetingHandle) const;

	virtual FRotator GetSweptTraceRotation_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;
	
public:
#if WITH_EDITORONLY_DATA
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
