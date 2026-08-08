// // Copyright 2025 RedMoonGames All Rights Reserved.
//
// #pragma once
//
// #include "Curves/CurveFloat.h"
// #include "DrawDebugHelpers.h"
// #include "GCMS_CameraMode_WithPenetrationAvoidance.h"
// #include "GCMS_CameraMode_ThirdPerson.generated.h"
//
// class UCurveVector;
//
// /**
//  * UGCMS_CameraMode_ThirdPerson
//  *
//  *	A basic third person camera mode.
//  */
// UCLASS(Abstract, Blueprintable)
// class UGCMS_CameraMode_ThirdPerson : public UGCMS_CameraMode_WithPenetrationAvoidance
// {
// 	GENERATED_BODY()
//
// public:
// 	UGCMS_CameraMode_ThirdPerson();
//
// protected:
// 	
// 	virtual void UpdateView_Implementation(float DeltaTime) override;
//
// 	UFUNCTION(BlueprintCallable, Category="Third Person")
// 	void UpdateForTarget(float DeltaTime);
//
// protected:
// 	// Curve that defines local-space offsets from the target using the view pitch to evaluate the curve.
// 	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "!bUseRuntimeFloatCurves"))
// 	TObjectPtr<const UCurveVector> TargetOffsetCurve;
//
// 	// UE-103986: Live editing of RuntimeFloatCurves during PIE does not work (unlike curve assets).
// 	// Once that is resolved this will become the default and TargetOffsetCurve will be removed.
// 	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
// 	bool bUseRuntimeFloatCurves;
//
// 	// time will be [-ViewPitchMin,ViewPitchMax]
// 	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
// 	FRuntimeFloatCurve TargetOffsetX;
//
// 	// time will be [-ViewPitchMin,ViewPitchMax]
// 	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
// 	FRuntimeFloatCurve TargetOffsetY;
//
// 	// time will be [-ViewPitchMin,ViewPitchMax]
// 	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
// 	FRuntimeFloatCurve TargetOffsetZ;
//
// 	// Alters the speed that a crouch offset is blended in or out
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
// 	float CrouchOffsetBlendMultiplier = 5.0f;
//
// protected:
// 	void SetTargetCrouchOffset(FVector NewTargetOffset);
// 	void UpdateCrouchOffset(float DeltaTime);
//
// 	FVector InitialCrouchOffset = FVector::ZeroVector;
// 	FVector TargetCrouchOffset = FVector::ZeroVector;
// 	float CrouchOffsetBlendPct = 1.0f;
// 	FVector CurrentCrouchOffset = FVector::ZeroVector;
// };
