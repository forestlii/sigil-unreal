// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/TargetingSelectionTask_Trace.h"
#include "SigilTargetingSelectionTask_TraceExt.generated.h"

class USigilCollisionTraceInstance;

/**
*	@class USigilTargetingSelectionTask_TraceExt
*	Specialized version of SelectionTask_Trace,Allow passing data via source object to Trace execution.
*	@attention SourceObject should be provided and implement SigilTargetingSourceInterface.
*/
UCLASS(meta=(DisplayName="GCS:SelectionTask (Trace)"))
class SIGILCOMBAT_API USigilTargetingSelectionTask_TraceExt : public UTargetingSelectionTask_Trace
{
	GENERATED_BODY()

public:
	virtual void Execute(const FTargetingRequestHandle& TargetingHandle) const override;

protected:
	
	/**
	 * If ticked, user context's source location as trace source location.
	 * Or it will try to get context's source actor location first,then fall back to context's source location.
	 * 如果勾选，会使用上下文的源位置作为Trace的源位置。
	 * 否则它会先从上下文的源Actor上获取位置，如果没有Actor，则回退到上下文的源位置。
	 */
	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Trace Data")
	bool bUseContextLocationAsSourceLocation{false};

	virtual FVector GetSourceLocation_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	virtual FVector GetTraceDirection_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	virtual void GetAdditionalActorsToIgnore_Implementation(const FTargetingRequestHandle& TargetingHandle, TArray<AActor*>& OutAdditionalActorsToIgnore) const override;
	
	/** Native Event to get the source location for the Trace */
	UFUNCTION(BlueprintNativeEvent, Category = "Target Trace Selection")
	USigilCollisionTraceInstance* GetSourceTraceInstance(const FTargetingRequestHandle& TargetingHandle) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Target Trace Selection")
	float GetTraceLevel(const FTargetingRequestHandle& TargetingHandle) const;

	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Trace Data")
	bool bTraceLengthLevel{true};

	virtual float GetTraceLength_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Swept Data")
	bool bSweptTraceRadiusLevel{true};

	virtual float GetSweptTraceRadius_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Swept Data")
	bool bSweptTraceCapsuleHalfHeightLevel{true};

	virtual float GetSweptTraceCapsuleHalfHeight_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

	UPROPERTY(EditAnywhere, Category = "Target Trace Selection | Swept Data")
	bool bSweptTraceBoxHalfExtentLevel{true};

	virtual FVector GetSweptTraceBoxHalfExtents_Implementation(const FTargetingRequestHandle& TargetingHandle) const override;

};
