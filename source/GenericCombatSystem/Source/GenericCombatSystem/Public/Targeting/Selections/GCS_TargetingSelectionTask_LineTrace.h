// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CollisionShape.h"
#include "Engine/CollisionProfile.h"
#include "Types/TargetingSystemTypes.h"
#include "ScalableFloat.h"
#include "Tasks/TargetingTask.h"
#include "UObject/Object.h"

#include "GCS_TargetingSelectionTask_LineTrace.generated.h"

class UTargetingSubsystem;
struct FCollisionQueryParams;
struct FTargetingDebugInfo;
struct FTargetingDefaultResultData;
struct FTargetingRequestHandle;
struct FTraceDatum;
struct FTraceHandle;

/**
*	@class UGCS_TargetingSelectionTask_LineTrace
*	Selection task that can perform a synchronous or asynchronous line trace, always generate hit results.
*	to find all targets up to the first blocking hit (or its end point).
*/
UCLASS(Blueprintable, meta=(DisplayName="GCS:SelectionTask (Line Trace)"))
class GENERICCOMBATSYSTEM_API UGCS_TargetingSelectionTask_LineTrace : public UTargetingTask
{
	GENERATED_BODY()

public:
	UGCS_TargetingSelectionTask_LineTrace(const FObjectInitializer& ObjectInitializer);

	/** Evaluation function called by derived classes to process the targeting request */
	virtual void Execute(const FTargetingRequestHandle& TargetingHandle) const override;

protected:
	/** Native Event to get the source location for the Trace */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Target Trace Selection")
	FVector GetSourceLocation(const FTargetingRequestHandle& TargetingHandle) const;

	/** Native Event to get a source location offset for the Trace */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Target Trace Selection")
	FVector GetSourceOffset(const FTargetingRequestHandle& TargetingHandle) const;

	/**
	 * Native Event to get the direction for the Trace
	 * Default will use pawn's control rotation or fallback to actor forward direction.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Target Trace Selection")
	FVector GetTraceDirection(const FTargetingRequestHandle& TargetingHandle) const;

	/** Native Event to get the length for the Trace */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Target Trace Selection")
	float GetTraceLength(const FTargetingRequestHandle& TargetingHandle) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Target Trace Selection")
	float GetTraceLevel(const FTargetingRequestHandle& TargetingHandle) const;

	/** Native Event to get additional actors the Trace should ignore */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Target Trace Selection")
	void GetAdditionalActorsToIgnore(const FTargetingRequestHandle& TargetingHandle, TArray<AActor*>& OutAdditionalActorsToIgnore) const;

protected:
	/** Method to process the trace task immediately */
	void ExecuteImmediateTrace(const FTargetingRequestHandle& TargetingHandle) const;

	/** Method to process the trace task asynchronously */
	void ExecuteAsyncTrace(const FTargetingRequestHandle& TargetingHandle) const;

	/** Callback for an async trace */
	void HandleAsyncTraceComplete(const FTraceHandle& InTraceHandle, FTraceDatum& InTraceDatum, FTargetingRequestHandle TargetingHandle) const;

	/** Method to take the hit results and store them in the targeting result data */
	void ProcessHitResults(const FTargetingRequestHandle& TargetingHandle, const TArray<FHitResult>& Hits) const;

	/** Setup CollisionQueryParams for the trace */
	void InitCollisionParams(const FTargetingRequestHandle& TargetingHandle, FCollisionQueryParams& OutParams) const;

protected:
	/** The trace channel to use */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Trace Selection | Collision Data")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	/** The collision profile name to use instead of trace channel (does not work for async traces) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Trace Selection | Collision Data")
	FCollisionProfileName CollisionProfileName;

	/** The default trace length to use if GetTraceLength is not overridden by a child */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, BlueprintReadOnly, Category = "Target Trace Selection | Trace Data")
	FScalableFloat DefaultTraceLength = 10.0f;

	/** The default source location offset used by GetSourceOffset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Trace Selection | Trace Data")
	FVector DefaultSourceOffset = FVector::ZeroVector;

	/** Indicates the trace should perform a complex trace */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Trace Selection | Trace Data")
	uint8 bComplexTrace : 1;

	/** Indicates the trace should ignore the source actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Trace Selection | Trace Data")
	uint8 bIgnoreSourceActor : 1;

	/** Indicates the trace should ignore the source actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Trace Selection | Trace Data")
	uint8 bIgnoreInstigatorActor : 1;

	// If there were no hits, add a default HitResult at the end of the trace
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Trace Selection | Trace Data")
	uint8 bGenerateDefaultHitResult : 1;

protected:
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	/** Debug Helper Methods */
#if ENABLE_DRAW_DEBUG

private:
	virtual void DrawDebug(UTargetingSubsystem* TargetingSubsystem, FTargetingDebugInfo& Info, const FTargetingRequestHandle& TargetingHandle, float XOffset, float YOffset,
	                       int32 MinTextRowsToAdvance) const override;

	/** Draw debug info showing the results of the shape trace used for targeting. */
	virtual void DrawDebugTrace(const FTargetingRequestHandle TargetingHandle, const FVector& StartLocation, const FVector& EndLocation, const bool bHit, const TArray<FHitResult>& Hits) const;
	void BuildTraceResultsDebugString(const FTargetingRequestHandle& TargetingHandle, const TArray<FTargetingDefaultResultData>& TargetResults) const;
	void ResetTraceResultsDebugString(const FTargetingRequestHandle& TargetingHandle) const;
#endif // ENABLE_DRAW_DEBUG
	/** ~Debug Helper Methods */
};
