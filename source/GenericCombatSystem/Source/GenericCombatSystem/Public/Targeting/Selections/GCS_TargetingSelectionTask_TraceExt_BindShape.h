// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GCS_TargetingSelectionTask_TraceExt.h"
#include "GCS_TargetingSelectionTask_TraceExt_BindShape.generated.h"


class UShapeComponent;

UENUM(BlueprintType)
enum class EGCS_TraceDataModifyType :uint8
{
	None UMETA(DisplayName="None"),
	Add UMETA(DisplayName="Add"),
	Multiply UMETA(DisplayName = "Multiply"),
};

/**
 * 
 */
UCLASS(meta=(DisplayName="GCS:SelectionTask (Trace Bind Shape)"))
class GENERICCOMBATSYSTEM_API UGCS_TargetingSelectionTask_TraceExt_BindShape : public UGCS_TargetingSelectionTask_TraceExt
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Trace Data")
	EGCS_TraceDataModifyType SweptTraceRadiusModType{EGCS_TraceDataModifyType::None};

	virtual float GetSweptTraceRadius_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Swept Data")
	EGCS_TraceDataModifyType SweptTraceCapsuleHalfHeightModType{EGCS_TraceDataModifyType::None};

	virtual float GetSweptTraceCapsuleHalfHeight_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Swept Data")
	EGCS_TraceDataModifyType SweptTraceBoxHalfExtentModType{EGCS_TraceDataModifyType::None};

	virtual FVector GetSweptTraceBoxHalfExtents_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;
	
	virtual UShapeComponent* GetTraceShape(const FTargetingRequestHandle& TargetingHandle) const;

	virtual FRotator GetSweptTraceRotation_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;
	
public:
#if WITH_EDITORONLY_DATA
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
