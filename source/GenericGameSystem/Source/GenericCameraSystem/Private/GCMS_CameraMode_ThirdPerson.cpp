// // Copyright 2025 RedMoonGames All Rights Reserved.
//
// #include "GCMS_CameraMode_ThirdPerson.h"
//
// #include "GCMS_CameraAssistInterface.h"
// #include "GCMS_CameraMode.h"
// #include "Components/PrimitiveComponent.h"
// #include "GCMS_CameraPenetrationAvoidanceFeeler.h"
// #include "Curves/CurveVector.h"
// #include "Engine/Canvas.h"
// #include "GameFramework/Controller.h"
// #include "GameFramework/Character.h"
// #include "Math/RotationMatrix.h"
//
// #include UE_INLINE_GENERATED_CPP_BY_NAME(GCMS_CameraMode_ThirdPerson)
//
//
// UGCMS_CameraMode_ThirdPerson::UGCMS_CameraMode_ThirdPerson()
// {
// 	TargetOffsetCurve = nullptr;
// }
//
// void UGCMS_CameraMode_ThirdPerson::UpdateView_Implementation(float DeltaTime)
// {
// 	UpdateForTarget(DeltaTime);
// 	UpdateCrouchOffset(DeltaTime);
//
// 	FVector PivotLocation = GetPivotLocation() + CurrentCrouchOffset;
// 	FRotator PivotRotation = GetPivotRotation();
//
// 	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);
//
// 	View.Location = PivotLocation;
// 	View.Rotation = PivotRotation;
// 	View.ControlRotation = View.Rotation;
// 	View.FieldOfView = FieldOfView;
//
// 	// Apply third person offset using pitch.
// 	if (!bUseRuntimeFloatCurves)
// 	{
// 		if (TargetOffsetCurve)
// 		{
// 			const FVector TargetOffset = TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);
// 			View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);
// 		}
// 	}
// 	else
// 	{
// 		FVector TargetOffset(0.0f);
//
// 		TargetOffset.X = TargetOffsetX.GetRichCurveConst()->Eval(PivotRotation.Pitch);
// 		TargetOffset.Y = TargetOffsetY.GetRichCurveConst()->Eval(PivotRotation.Pitch);
// 		TargetOffset.Z = TargetOffsetZ.GetRichCurveConst()->Eval(PivotRotation.Pitch);
//
// 		View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);
// 	}
//
// 	// Adjust final desired camera location to prevent any penetration
// 	UpdatePreventPenetration(DeltaTime);
// }
//
// void UGCMS_CameraMode_ThirdPerson::UpdateForTarget(float DeltaTime)
// {
// 	if (const ACharacter* TargetCharacter = Cast<ACharacter>(GetTargetActor()))
// 	{
// 		if (TargetCharacter->bIsCrouched)
// 		{
// 			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
// 			const float CrouchedHeightAdjustment = TargetCharacterCDO->CrouchedEyeHeight - TargetCharacterCDO->BaseEyeHeight;
//
// 			SetTargetCrouchOffset(FVector(0.f, 0.f, CrouchedHeightAdjustment));
//
// 			return;
// 		}
// 	}
//
// 	SetTargetCrouchOffset(FVector::ZeroVector);
// }
//
// void UGCMS_CameraMode_ThirdPerson::SetTargetCrouchOffset(FVector NewTargetOffset)
// {
// 	CrouchOffsetBlendPct = 0.0f;
// 	InitialCrouchOffset = CurrentCrouchOffset;
// 	TargetCrouchOffset = NewTargetOffset;
// }
//
//
// void UGCMS_CameraMode_ThirdPerson::UpdateCrouchOffset(float DeltaTime)
// {
// 	if (CrouchOffsetBlendPct < 1.0f)
// 	{
// 		CrouchOffsetBlendPct = FMath::Min(CrouchOffsetBlendPct + DeltaTime * CrouchOffsetBlendMultiplier, 1.0f);
// 		CurrentCrouchOffset = FMath::InterpEaseInOut(InitialCrouchOffset, TargetCrouchOffset, CrouchOffsetBlendPct, 1.0f);
// 	}
// 	else
// 	{
// 		CurrentCrouchOffset = TargetCrouchOffset;
// 		CrouchOffsetBlendPct = 1.0f;
// 	}
// }
