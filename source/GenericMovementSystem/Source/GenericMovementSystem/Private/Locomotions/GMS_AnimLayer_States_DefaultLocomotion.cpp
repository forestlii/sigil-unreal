// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Locomotions/GMS_AnimLayer_States_DefaultLocomotion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "AnimationStateMachineLibrary.h"
#include "GameFramework/Pawn.h"
#include "AnimCharacterMovementLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "GMS_MovementSystemComponent.h"
#include "KismetAnimationLibrary.h"
#include "Animation/AnimSequence.h"
#include "SequenceEvaluatorLibrary.h"
#include "SequencePlayerLibrary.h"
#include "AnimNodes/AnimNode_SequenceEvaluator.h"
#include "BlendStack/AnimNode_BlendStack.h"
#include "Kismet/KismetMathLibrary.h"
#include "Locomotions/GMS_MainAnimInstance.h"
#include "Utility/GMS_Log.h"
#include "Utility/GMS_Math.h"
#include "Utility/GMS_Utility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimLayer_States_DefaultLocomotion)

#pragma region Setting


void FGMS_AnimData_Jump::Validate()
{
	Super::Validate();
	bValidJumpStartLoop = IsValid(JumpStartLoop);
	bValidJumpApex = IsValid(JumpApex);
	bValidJumpFallLoop = IsValid(JumpFallLoop);
	bValidJumpFallLand = IsValid(JumpFallLand);
	// bValid = bValidJumpStartLoop && bValidJumpApex && bValidJumpFallLand && IsValid(JumpStart);
	bValid = IsValid(JumpStart);
}


void FGMS_AnimData_Idle::Validate()
{
	Super::Validate();
	bValid = Idle != nullptr;
	bValidCrouchAnim = CrouchEntry != nullptr && CrouchExit != nullptr;
}

void FGMS_AnimData_Start_ViewDirection::Validate()
{
	Super::Validate();
	bValid = Animations.Forward && Animations.Backward && Animations.Left && Animations.Right;
}

void FGMS_AnimData_Start_VelocityDirection::Validate()
{
	Super::Validate();
	if (AnimType == EGMS_StartAnimType_VelocityDir::Reface)
	{
		bValid = Animations.StartForward && Animations.StartForwardL90 && Animations.StartForwardL180 && Animations.StartForwardR90 && Animations.StartForwardR180;
	}
	if (AnimType == EGMS_StartAnimType_VelocityDir::Single)
	{
		bValid = Animation != nullptr;
	}
}

void FGMS_AnimData_Cycle::Validate()
{
	Super::Validate();

	if (AnimType == EGMS_CycleAnimType::Direction_4)
	{
		bValid = Animations.ValidAnimations();
		if (bValid)
		{
			// bHasRootMotion = Animations.HasRootMotion();
		}
	}

	if (AnimType == EGMS_CycleAnimType::Direction_8)
	{
		bValid = Animations_8Direction.ValidAnimations();
		if (bValid)
		{
			// bHasRootMotion = Animations_8Direction.HasRootMotion();
		}
	}

	if (AnimType == EGMS_CycleAnimType::Single)
	{
		bValid = Animation != nullptr;
		if (bValid)
		{
			// bHasRootMotion = Animation->HasRootMotion();
		}
	}
}

void FGMS_AnimData_Stop::Validate()
{
	Super::Validate();

	if (AnimType == EGMS_StopAnimType::Single)
	{
		bValid = Animation != nullptr;
	}
	if (AnimType == EGMS_StopAnimType::Direction_4)
	{
		bValid = Animations.Forward && Animations.Backward && Animations.Left && Animations.Right;
	}
	if (AnimType == EGMS_StopAnimType::Direction_8)
	{
		bValid = Animations_8Direction.Forward && Animations_8Direction.Backward && Animations_8Direction.Left && Animations_8Direction.Right
			&& Animations_8Direction.ForwardLeft && Animations_8Direction.ForwardRight && Animations_8Direction.BackwardLeft && Animations_8Direction.BackwardRight;
	}
}

void FGMS_AnimData_Pivot::Validate()
{
	Super::Validate();
	bValid = Animations.Forward && Animations.Backward && Animations.Left && Animations.Right;
}


void FGMS_AnimData_Land::Validate()
{
	Super::Validate();
	bValid = !Lands.IsEmpty();

#if WITH_EDITORONLY_DATA
	for (FGMS_AnimationWithDistance& Anim : Lands)
	{
		if (Anim.Animation != nullptr)
		{
			Anim.EditorFriendlyName = FString::Format(TEXT("Play {0} With Fall Velocity:{1}"), {Anim.Animation->GetName(), Anim.Distance});
		}
		else
		{
			Anim.EditorFriendlyName = TEXT("Empty Anim");
		}
	}
#endif
}


void FGMS_AnimData_Lean::Validate()
{
	Super::Validate();
	bValid = BlendSpace != nullptr;
}

void FGMS_AnimData_TurnInPlace::Validate()
{
	Super::Validate();
	bValid = Left != nullptr && Right != nullptr;
}


#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

void UGMS_AnimLayerSetting_States_Default::PreSave(FObjectPreSaveContext SaveContext)
{
	if (Idle_Inst.IsValid())
	{
		if (FGMS_AnimData* Data = Idle_Inst.GetMutablePtr<FGMS_AnimData>())
		{
			Data->Validate();
		}
	}

	if (Turn_Inst.IsValid())
	{
		if (FGMS_AnimData* Data = Turn_Inst.GetMutablePtr<FGMS_AnimData>())
		{
			Data->Validate();
		}
	}

	if (Jump_Inst.IsValid())
	{
		if (FGMS_AnimData* Data = Jump_Inst.GetMutablePtr<FGMS_AnimData>())
		{
			Data->Validate();
		}
	}

	if (Land_Inst.IsValid())
	{
		if (FGMS_AnimData* Data = Land_Inst.GetMutablePtr<FGMS_AnimData>())
		{
			Data->Validate();
		}
	}

	if (Lean_Inst.IsValid())
	{
		if (FGMS_AnimData* Data = Lean_Inst.GetMutablePtr<FGMS_AnimData>())
		{
			Data->Validate();
		}
	}
	AcceleratedMovingStates.Empty();

	for (FGMS_AnimData_MovingStates& GS : MovingStates)
	{
		if (GS.Start_ViewDir_Inst.IsValid())
		{
			if (FGMS_AnimData* Data = GS.Start_ViewDir_Inst.GetMutablePtr<FGMS_AnimData>())
			{
				Data->Validate();
			}
		}
		if (GS.Start_VelocityDir_Inst.IsValid())
		{
			if (FGMS_AnimData* Data = GS.Start_VelocityDir_Inst.GetMutablePtr<FGMS_AnimData>())
			{
				Data->Validate();
			}
		}
		if (GS.Cycle_Inst.IsValid())
		{
			if (FGMS_AnimData* Data = GS.Cycle_Inst.GetMutablePtr<FGMS_AnimData>())
			{
				Data->Validate();
			}
		}
		if (GS.Stop_Inst.IsValid())
		{
			if (FGMS_AnimData* Data = GS.Stop_Inst.GetMutablePtr<FGMS_AnimData>())
			{
				Data->Validate();
			}
		}
		if (GS.Pivot_Inst.IsValid())
		{
			if (FGMS_AnimData* Data = GS.Pivot_Inst.GetMutablePtr<FGMS_AnimData>())
			{
				Data->Validate();
			}
		}
		AcceleratedMovingStates.Emplace(GS.Tag, GS);
	}
	Super::PreSave(SaveContext);
}


#endif

#pragma endregion

UGMS_AnimLayer_States_DefaultLocomotion::UGMS_AnimLayer_States_DefaultLocomotion()
{
	RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
}

void UGMS_AnimLayer_States_DefaultLocomotion::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UGMS_AnimLayer_States_DefaultLocomotion::NativeThreadSafeUpdateAnimation(const float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGMS_AnimLayer_States_DefaultLocomotion::NativeThreadSafeUpdateAnimation()"),
	                            STAT_UGMS_LocomotionAnimInstance_NativeThreadSafeUpdateAnimation, STATGROUP_GMS)

	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	if (!PawnOwner || !IsValid(MSC) || !IsValid(GetParent()))
	{
		return;
	}

	PivotState.Direction2D = UKismetMathLibrary::VLerp(PivotState.Direction2D, GetParent()->InputDirection.GetSafeNormal2D(), 0.5).GetSafeNormal();
	PivotState.DesiredDirection = GetOppositeCardinalDirection(SelectCardinalDirectionFromAngle(
		UKismetAnimationLibrary::CalculateDirection(PivotState.Direction2D, GetParent()->LocomotionState.Rotation),
		10, EGMS_MovementDirection::Forward, false));
}

void UGMS_AnimLayer_States_DefaultLocomotion::NativePostEvaluateAnimation()
{
	Super::NativePostEvaluateAnimation();

	TurnInPlaceState.bUpdatedThisFrame = false;
}

void UGMS_AnimLayer_States_DefaultLocomotion::ApplySetting_Implementation(const UGMS_AnimLayerSetting* NewSetting)
{
	if (GetParent() == nullptr)
	{
		return;
	}

	const UGMS_AnimLayerSetting_States_Default* DS = Cast<UGMS_AnimLayerSetting_States_Default>(NewSetting);

	if (!DS)
	{
		ResetSetting();
		return;
	}

	Setting = DS;

	if (IsValid(MSC->AnimGraphSetting))
	{
		OWSettings = MSC->AnimGraphSetting->OrientationWarping;
	}

	if (DS->Idle_Inst.IsValid())
	{
		AnimData_Idle = DS->Idle_Inst.Get<FGMS_AnimData_Idle>();
	}
	else
	{
		AnimData_Idle = FGMS_AnimData_Idle();
	}

	if (DS->Turn_Inst.IsValid())
	{
		AnimData_TurnInPlace = DS->Turn_Inst.Get<FGMS_AnimData_TurnInPlace>();
	}
	else
	{
		AnimData_TurnInPlace = FGMS_AnimData_TurnInPlace();
	}

	if (DS->Jump_Inst.IsValid())
	{
		AnimData_Jump = DS->Jump_Inst.Get<FGMS_AnimData_Jump>();
		bEnableJump = AnimData_Jump.bValid;
		bEnableFall = AnimData_Jump.bValidJumpFallLoop;
	}
	else
	{
		AnimData_Jump = FGMS_AnimData_Jump();
		bEnableJump = false;
		bEnableFall = false;
	}

	GetParent()->bEnableGroundPrediction = AnimData_Jump.bValidJumpFallLand && AnimData_Jump.bEnableGroundPrediction;

	if (DS->Land_Inst.IsValid())
	{
		AnimData_Land = DS->Land_Inst.Get<FGMS_AnimData_Land>();
		bEnableLand = AnimData_Land.bValid;
	}
	else
	{
		AnimData_Land = FGMS_AnimData_Land();
		bEnableLand = false;
	}

	if (DS->Lean_Inst.IsValid())
	{
		AnimData_GroundedLean = DS->Lean_Inst.Get<FGMS_AnimData_Lean>();
	}
	else
	{
		AnimData_GroundedLean = FGMS_AnimData_Lean();
	}

	checkf(!DS->MovingStates.IsEmpty(), TEXT("Moving states on %s can't be empty!"), *DS->GetName());

	//Apply gounrded states.
	{
		AnimData_MovingStates = DS->AcceleratedMovingStates.Contains(GetParent()->MovementState) ? DS->AcceleratedMovingStates[GetParent()->MovementState] : DS->MovingStates.Last();

		{
			if (GetParent()->RotationMode == GMS_RotationModeTags::ViewDirection)
			{
				if (AnimData_MovingStates.Start_ViewDir_Inst.IsValid())
				{
					AnimData_Start_ViewDirection = AnimData_MovingStates.Start_ViewDir_Inst.Get<FGMS_AnimData_Start_ViewDirection>();
					bEnableStart = AnimData_Start_ViewDirection.bValid;
				}
				else
				{
					AnimData_Start_ViewDirection = FGMS_AnimData_Start_ViewDirection();
					bEnableStart = false;
				}
			}

			if (GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection)
			{
				if (AnimData_MovingStates.Start_VelocityDir_Inst.IsValid())
				{
					AnimData_Start_VelocityDirection = AnimData_MovingStates.Start_VelocityDir_Inst.Get<FGMS_AnimData_Start_VelocityDirection>();
					bEnableStart = AnimData_Start_VelocityDirection.bValid;
				}
				else
				{
					AnimData_Start_VelocityDirection = FGMS_AnimData_Start_VelocityDirection();
					bEnableStart = false;
				}
			}
		}

		if (AnimData_MovingStates.Cycle_Inst.IsValid())
		{
			AnimData_Cycle = AnimData_MovingStates.Cycle_Inst.Get<FGMS_AnimData_Cycle>();
		}
		else
		{
			UE_LOG(LogGMS, Error, TEXT("Missing AnimData_Cycle for movement state(%s) on actor(%s)!"), *GetParent()->MovementState.ToString(), *PawnOwner->GetName());
			AnimData_Cycle = FGMS_AnimData_Cycle();
		}

		if (AnimData_MovingStates.Stop_Inst.IsValid())
		{
			AnimData_Stop = AnimData_MovingStates.Stop_Inst.Get<FGMS_AnimData_Stop>();
			bEnableStop = AnimData_Stop.bValid;
		}
		else
		{
			AnimData_Stop = FGMS_AnimData_Stop();
			bEnableStop = false;
		}

		if (AnimData_MovingStates.Pivot_Inst.IsValid())
		{
			AnimData_Pivot = AnimData_MovingStates.Pivot_Inst.Get<FGMS_AnimData_Pivot>();
			bEnablePivot = AnimData_Pivot.bValid;
		}
		else
		{
			AnimData_Pivot = FGMS_AnimData_Pivot();
			bEnablePivot = false;
		}
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::ResetSetting_Implementation()
{
	OWSettings = FGMS_OrientationWarpingSettings();
	AnimData_Idle = FGMS_AnimData_Idle();
	AnimData_TurnInPlace = FGMS_AnimData_TurnInPlace();
	AnimData_Start_ViewDirection = FGMS_AnimData_Start_ViewDirection();
	AnimData_Start_VelocityDirection = FGMS_AnimData_Start_VelocityDirection();
	AnimData_Cycle = FGMS_AnimData_Cycle();
	AnimData_GroundedLean = FGMS_AnimData_Lean();
	AnimData_Stop = FGMS_AnimData_Stop();
	AnimData_Pivot = FGMS_AnimData_Pivot();
	AnimData_Jump = FGMS_AnimData_Jump();
	AnimData_Land = FGMS_AnimData_Land();

	bEnableStart = false;
	bEnableLand = false;
	bEnablePivot = false;
	bEnableStop = false;
	if (IsValid(GetParent()))
	{
		GetParent()->bEnableGroundPrediction = false;
	}
	bEnableJump = false;
	bEnableFall = false;
}

// float UGMS_AnimLayer_States_DefaultLocomotion::GetInputVelocityDelta() const
// {
// 	if (GetParent()->LocomotionState.Velocity.Equals(FVector(0, 0, 0), 10) || GetParent()->InputDirection.Equals(FVector(0, 0, 0), 0.01))
// 	{
// 		return 0;
// 	}
//
// 	return UKismetMathLibrary::DegAcos(
// 		FVector::DotProduct(
// 			GetParent()->LocomotionState.Velocity.GetSafeNormal(0.0001),
// 			GetParent()->InputDirection.GetSafeNormal(0.0001)));
// }

bool UGMS_AnimLayer_States_DefaultLocomotion::IsMovingPerpendicularToInitialPivot() const
{
	//We stay in a pivot when pivoting along a line (e.g. triggering a left-right pivot while playing a right-left pivot), but break out if the character makes a perpendicular change in direction.
	EGMS_MovementDirection CurrentDirection = GetParent()->LocomotionState.LocalVelocityDirection;
	bool A = PivotState.InitialDirection == EGMS_MovementDirection::Forward || PivotState.InitialDirection == EGMS_MovementDirection::Backward;
	bool B = !(CurrentDirection == EGMS_MovementDirection::Forward || CurrentDirection == EGMS_MovementDirection::Backward);
	bool C = PivotState.InitialDirection == EGMS_MovementDirection::Left || PivotState.InitialDirection == EGMS_MovementDirection::Right;
	bool D = !(CurrentDirection == EGMS_MovementDirection::Left || CurrentDirection == EGMS_MovementDirection::Right);
	return (A && B) || (C && D);
}

bool UGMS_AnimLayer_States_DefaultLocomotion::IsPivoting_Implementation() const
{
	return GetParent()->LocomotionMode == GMS_MovementModeTags::Grounded && UKismetMathLibrary::Dot_VectorVector(GetParent()->LocomotionState.Velocity.GetSafeNormal2D(),
	                                                                                                             GetParent()->InputDirection.GetSafeNormal2D()) < 0.0f;

	// return GetParent()->LocomotionMode == GMS_MovementModeTags::Grounded && UKismetMathLibrary::Dot_VectorVector(GetParent()->LocomotionState.LocalVelocity2D,GetParent()->LocomotionState.LocalAcceleration2D) < 0.0f;
}

bool UGMS_AnimLayer_States_DefaultLocomotion::IsStarting_Implementation() const
{
	return GetParent()->LocomotionState.bHasInput || GetParent()->LocomotionState.bHasVelocity;
}


EGMS_MovementDirection UGMS_AnimLayer_States_DefaultLocomotion::SelectCardinalDirectionFromAngle(float Angle, float DeadZone, EGMS_MovementDirection CurrentDirection, bool bUseCurrentDirection) const
{
	const float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;
	if (bUseCurrentDirection)
	{
		if (CurrentDirection == EGMS_MovementDirection::Forward)
		{
			FwdDeadZone *= 2;
		}
		if (CurrentDirection == EGMS_MovementDirection::Backward)
		{
			BwdDeadZone *= 2;
		}
	}

	if (AbsAngle <= 45 + FwdDeadZone)
	{
		return EGMS_MovementDirection::Forward;
	}

	if (AbsAngle >= 135 - BwdDeadZone)
	{
		return EGMS_MovementDirection::Backward;
	}
	if (Angle > 0)
	{
		return EGMS_MovementDirection::Right;
	}

	return EGMS_MovementDirection::Left;
}

EGMS_MovementDirection_8Way UGMS_AnimLayer_States_DefaultLocomotion::SelectOctagonalDirectionFromAngle(float Angle, float DeadZone, EGMS_MovementDirection_8Way CurrentDirection,
                                                                                                       bool bUseCurrentDirection) const
{
	const float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;
	if (bUseCurrentDirection)
	{
		if (CurrentDirection == EGMS_MovementDirection_8Way::Forward)
		{
			FwdDeadZone *= 2;
		}
		if (CurrentDirection == EGMS_MovementDirection_8Way::Backward)
		{
			BwdDeadZone *= 2;
		}
	}

	if (AbsAngle <= 22.5f + FwdDeadZone)
	{
		return EGMS_MovementDirection_8Way::Forward;
	}
	else if (AbsAngle >= 157.5f - BwdDeadZone)
	{
		return EGMS_MovementDirection_8Way::Backward;
	}
	else if (Angle >= 22.5f && Angle < 67.5f)
	{
		return EGMS_MovementDirection_8Way::ForwardRight;
	}
	else if (Angle >= 67.5f && Angle < 112.5f)
	{
		return EGMS_MovementDirection_8Way::Right;
	}
	else if (Angle >= 112.5f && Angle < 157.5f)
	{
		return EGMS_MovementDirection_8Way::BackwardRight;
	}
	else if (Angle >= -157.5f && Angle < -112.5f)
	{
		return EGMS_MovementDirection_8Way::BackwardLeft;
	}
	else if (Angle >= -112.5f && Angle < -67.5f)
	{
		return EGMS_MovementDirection_8Way::Left;
	}
	else
	{
		return EGMS_MovementDirection_8Way::ForwardLeft;
	}
}

EGMS_MovementDirection UGMS_AnimLayer_States_DefaultLocomotion::GetOppositeCardinalDirection(EGMS_MovementDirection CurrentDirection) const
{
	switch (CurrentDirection)
	{
	case EGMS_MovementDirection::Forward:
		return EGMS_MovementDirection::Backward;
	case EGMS_MovementDirection::Backward:
		return EGMS_MovementDirection::Forward;
	case EGMS_MovementDirection::Left:
		return EGMS_MovementDirection::Right;
	case EGMS_MovementDirection::Right:
		return EGMS_MovementDirection::Left;
	default:
		return CurrentDirection;
	}
}

bool UGMS_AnimLayer_States_DefaultLocomotion::IsViewDirection() const
{
	return GetParent()->RotationMode == GMS_RotationModeTags::ViewDirection;
}

bool UGMS_AnimLayer_States_DefaultLocomotion::IsVelocityDirection() const
{
	return GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection;
}

const FGMS_ViewDirectionSetting& UGMS_AnimLayer_States_DefaultLocomotion::GetViewDirectionSetting() const
{
	return GetParent()->GetMovementSystemComponent()->GetViewDirSetting();
}

const FGMS_VelocityDirectionSetting& UGMS_AnimLayer_States_DefaultLocomotion::GetVelocityDirectionSetting() const
{
	return GetParent()->GetMovementSystemComponent()->GetVelocityDirSetting();
}

#pragma region Idle

void UGMS_AnimLayer_States_DefaultLocomotion::Idle_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Failed)
		return;

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, AnimData_Idle.Idle, AnimData_Idle.BlendTime);
}

void UGMS_AnimLayer_States_DefaultLocomotion::InitializeIdleBreak_Implementation()
{
	if (!GetParent())
	{
		return;
	}
	if (AnimData_Idle.IdleBreakDelayTime > 0)
	{
		IdleBreakState.IdleBreakDelayTime = AnimData_Idle.IdleBreakDelayTime;
	}
	else
	{
		IdleBreakState.IdleBreakDelayTime = 6 + FMath::TruncToInt(FMath::Abs(GetParent()->LocomotionState.Location.X + GetParent()->LocomotionState.Location.Y)) % 10;
	}
	IdleBreakState.TimeUntilNextIdleBreak = IdleBreakState.IdleBreakDelayTime;
}

void UGMS_AnimLayer_States_DefaultLocomotion::RefreshIdleBreak_Implementation()
{
	if (IsIdleBreakAllowed())
	{
		IdleBreakState.TimeUntilNextIdleBreak -= GetDeltaSeconds();
	}
	else
	{
		IdleBreakState.TimeUntilNextIdleBreak = IdleBreakState.IdleBreakDelayTime;
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::IdleBreak_AnimRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	if (!IsIdleBreakAllowed())
	{
		return;
	}
	bool Result = false;
	FSequencePlayerReference SequencePlayer;
	USequencePlayerLibrary::ConvertToSequencePlayerPure(Node, SequencePlayer, Result);
	if (!Result)
		return;

	// clamp index in range(in case swapped animation during last update.)
	if (IdleBreakState.CurrentIdleBreakIndex >= AnimData_Idle.Idle_Breaks.Num())
	{
		IdleBreakState.CurrentIdleBreakIndex = 0;
	}

	USequencePlayerLibrary::SetSequence(SequencePlayer, AnimData_Idle.Idle_Breaks[IdleBreakState.CurrentIdleBreakIndex]);

	//推进IdleBreak动画.
	IdleBreakState.CurrentIdleBreakIndex = IdleBreakState.CurrentIdleBreakIndex + 1;
	if (IdleBreakState.CurrentIdleBreakIndex >= AnimData_Idle.Idle_Breaks.Num())
	{
		IdleBreakState.CurrentIdleBreakIndex = 0;
	}
}
#pragma endregion

#pragma region IdleBreak

bool UGMS_AnimLayer_States_DefaultLocomotion::IsIdleBreakAllowed_Implementation() const
{
	if (AnimData_Idle.bDisableIdleBreaks)
		return false;
	if (AnimData_Idle.Idle_Breaks.IsEmpty())
	{
		return false;
	}
	if (IsViewDirection() && GetViewDirectionSetting().DirectionMode == EGMS_ViewDirectionMode::Aiming)
	{
		return false;
	}
	return true;
}

void UGMS_AnimLayer_States_DefaultLocomotion::Idle_StateUpdate(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	FAnimationStateResultReference AnimationStateResult;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationStateResult, Result);

	// try to enable root rotation offset.
	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationStateResult))
		{
			RefreshTurnInPlaceMode();
		}
	}
}

#pragma endregion

#pragma region TurnInPlace

void UGMS_AnimLayer_States_DefaultLocomotion::SetupTurnInPlace(float TurnAngle)
{
	bool bHas180 = AnimData_TurnInPlace.Left180 && AnimData_TurnInPlace.Right180;
	if (FMath::Abs(TurnAngle) < AnimData_TurnInPlace.Turn180AngleThreshold || !bHas180)
	{
		TurnInPlaceState.Animation = TurnAngle <= 0.0f || TurnAngle > 180.0f - UGMS_Math::CounterClockwiseRotationAngleThreshold
			                             ? AnimData_TurnInPlace.Left
			                             : AnimData_TurnInPlace.Right;
		TurnInPlaceState.b180 = false;
	}
	else
	{
		TurnInPlaceState.Animation = TurnAngle <= 0.0f ||
		                             TurnAngle > 180.0f - UGMS_Math::CounterClockwiseRotationAngleThreshold
			                             ? AnimData_TurnInPlace.Left180
			                             : AnimData_TurnInPlace.Right180;
		TurnInPlaceState.b180 = true;
	}

	TurnInPlaceState.TriggeredAngle = TurnAngle;
}

void UGMS_AnimLayer_States_DefaultLocomotion::RefreshTurnInPlaceMode()
{
#if WITH_EDITOR
	if (!IsValid(GetWorld()) || !GetWorld()->IsGameWorld())
	{
		return;
	}
#endif

	if (TurnInPlaceState.bUpdatedThisFrame)
	{
		return;
	}
	TurnInPlaceState.bUpdatedThisFrame = true;

	// has no animation or has core state changes.
	if (!AnimData_TurnInPlace.bValid || GetParent()->HasCoreStateChanges())
	{
		TurnInPlaceState.bShouldTurn = false;
		TurnInPlaceState.ActivationDelay = 0.0f;
		return;
	}

	RefreshTurnInPlaceInVelocityDirection();
	RefreshTurnInPlaceInViewDirection();
}

void UGMS_AnimLayer_States_DefaultLocomotion::RefreshTurnInPlaceInViewDirection()
{
	static constexpr auto PlayRateInterpolationSpeed{5.0f};
	if (!IsViewDirection())
	{
		return;
	}

	// Refresh root rotation offset mode.
	{
		if (GetViewDirectionSetting().bRotateToViewDirectionWhileNotMoving)
		{
			if (GetViewDirectionSetting().DirectionMode == EGMS_ViewDirectionMode::Aiming && !AnimData_TurnInPlace.bTurnWhenAimingInViewDirection)
			{
				GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Release);
			}
			else
			{
				GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Accumulate);
			}
		}
		else
		{
			GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Release);
		}
	}

	bool bWantsToTurn = FMath::Abs(GetParent()->ViewState.YawAngle) >= AnimData_TurnInPlace.ViewYawAngleThreshold;

	bool bShouldDelay = AnimData_TurnInPlace.ViewYawAngleToActivationDelay.X > 0 && GetViewDirectionSetting().DirectionMode != EGMS_ViewDirectionMode::Aiming;

	if (!bWantsToTurn && !TurnInPlaceState.bShouldTurn)
	{
		TurnInPlaceState.ActivationDelay = 0.0f;
	}

	if (!bWantsToTurn && TurnInPlaceState.bShouldTurn)
	{
		TurnInPlaceState.bShouldTurn = false;
	}

	//更新转身过程的状态。
	if (TurnInPlaceState.bShouldTurn)
	{
		// if (TurnInPlaceState.ActivationDelay <= 0)
		if (!bShouldDelay)
		{
			if (GetParent()->GetOffsetRootBoneRotationMode() == EOffsetRootBoneMode::Accumulate)
			{
				TurnInPlaceState.PlayRate = AnimData_TurnInPlace.PlayRate;
				TurnInPlaceState.ScaledPlayRate = TurnInPlaceState.PlayRate;
			}
			else
			{
				const auto NewPlayRate{
					FMath::GetMappedRangeValueClamped(AnimData_TurnInPlace.ReferenceViewYawSpeed, AnimData_TurnInPlace.PlayRateRange, GetParent()->ViewState.YawSpeed)
				};

				TurnInPlaceState.PlayRate = FMath::FInterpTo(TurnInPlaceState.PlayRate, NewPlayRate, GetDeltaSeconds(), PlayRateInterpolationSpeed);
				TurnInPlaceState.ScaledPlayRate = TurnInPlaceState.PlayRate;
			}
			// TurnInPlaceState.PlayRate = AnimData_TurnInPlace.PlayRateRange.X;
			// TurnInPlaceState.ScaledPlayRate = FMath::FInterpTo(TurnInPlaceState.PlayRate, NewPlayRate, GetDeltaSeconds(), PlayRateInterpolationSpeed);
		}
		else
		{
			// TurnInPlaceState.PlayRate = FMath::FInterpTo(TurnInPlaceState.PlayRate, AnimData_TurnInPlace.PlayRate, GetDeltaSeconds(), PlayRateInterpolationSpeed);
			TurnInPlaceState.PlayRate = AnimData_TurnInPlace.PlayRate;
			TurnInPlaceState.ScaledPlayRate = AnimData_TurnInPlace.bScaleTurnRate
				                                  ? TurnInPlaceState.PlayRate * FMath::Abs(TurnInPlaceState.TriggeredAngle / (TurnInPlaceState.b180 ? 180 : 90))
				                                  : TurnInPlaceState.PlayRate;
		}
	}


	// Not start
	if (bWantsToTurn && !TurnInPlaceState.bShouldTurn && bShouldDelay)
	{
		const auto& DesiredDelayTime = bShouldDelay
			                               ? FMath::GetMappedRangeValueClamped({AnimData_TurnInPlace.ViewYawAngleThreshold, 180.0f},
			                                                                   AnimData_TurnInPlace.ViewYawAngleToActivationDelay, FMath::Abs(GetParent()->ViewState.YawAngle))
			                               : 0.0f;
		TurnInPlaceState.ActivationDelay += GetDeltaSeconds();
		if (!TurnInPlaceState.bShouldTurn && TurnInPlaceState.ActivationDelay <= DesiredDelayTime)
		{
			return;
		}
	}

	// Trigger turn-in-place.
	if (bWantsToTurn && !TurnInPlaceState.bShouldTurn)
	{
		SetupTurnInPlace(GetParent()->ViewState.YawAngle);
		TurnInPlaceState.bShouldTurn = true;
		UE_LOG(LogGMS, VeryVerbose, TEXT("Trigger turn-in-place in view direction, Anim:%s, Angle:%f"), *(TurnInPlaceState.Animation?TurnInPlaceState.Animation->GetName():TEXT("Null")),
		       GetParent()->ViewState.YawAngle);
		return;
	}

	TurnInPlaceState.bShouldTurn = bWantsToTurn;
}

void UGMS_AnimLayer_States_DefaultLocomotion::RefreshTurnInPlaceInVelocityDirection()
{
	if (!IsVelocityDirection())
	{
		return;
	}


	bool bHasRootYawOffset = !UKismetMathLibrary::NearlyEqual_FloatFloat(GetParent()->RootState.YawOffset, 0.00001);
	// has any root offset.
	if (bHasRootYawOffset && AnimData_TurnInPlace.bTurnInVelocityDirection)
	{
		GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Accumulate);
	}
	else
	{
		GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Release);
	}

	bool bWantsToTurn = bHasRootYawOffset && FMath::Abs(GetParent()->RootState.YawOffset) >= AnimData_TurnInPlace.RootYawAngleThreshold && AnimData_TurnInPlace.bTurnInVelocityDirection;

	// cancel turn.
	if (!bWantsToTurn && TurnInPlaceState.bShouldTurn)
	{
		TurnInPlaceState.bShouldTurn = false;
	}

	// Trigger turn-in-place.
	if (bWantsToTurn && !TurnInPlaceState.bShouldTurn)
	{
		SetupTurnInPlace(GetParent()->RootState.YawOffset * -1);
		TurnInPlaceState.PlayRate = AnimData_TurnInPlace.PlayRate;
		TurnInPlaceState.ScaledPlayRate = AnimData_TurnInPlace.bScaleTurnRate
			                                  ? AnimData_TurnInPlace.PlayRate * FMath::Abs(TurnInPlaceState.TriggeredAngle / (TurnInPlaceState.b180 ? 180 : 90))
			                                  : AnimData_TurnInPlace.PlayRate;
		TurnInPlaceState.bShouldTurn = true;
		UE_LOG(LogGMS, VeryVerbose, TEXT("Trigger turn-in-place in velocity direction, Anim:%s, Angle:%f"), *(TurnInPlaceState.Animation?TurnInPlaceState.Animation->GetName():TEXT("Null")),
		       GetParent()->RootState.YawOffset * -1);
	}
}

#pragma endregion

#pragma region Start

UAnimSequence* UGMS_AnimLayer_States_DefaultLocomotion::GetStartAnimSequence_Implementation() const
{
	UAnimSequence* Animation = nullptr;

	if (GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection)
	{
		float Delta = StartState.YawDeltaToAcceleration;

		if (AnimData_Start_VelocityDirection.AnimType == EGMS_StartAnimType_VelocityDir::Reface)
		{
			if (FMath::Abs(Delta) > 45 && FMath::Abs(Delta) < 135)
			{
				Animation = Delta <= 0.0f || Delta > 180.0f - UGMS_Math::CounterClockwiseRotationAngleThreshold
					            ? AnimData_Start_VelocityDirection.Animations.StartForwardL90
					            : AnimData_Start_VelocityDirection.Animations.StartForwardR90;
			}
			else if (FMath::Abs(Delta) >= 135)
			{
				Animation = Delta <= 0.0f || Delta > 180.0f - UGMS_Math::CounterClockwiseRotationAngleThreshold
					            ? AnimData_Start_VelocityDirection.Animations.StartForwardL180
					            : AnimData_Start_VelocityDirection.Animations.StartForwardR180;
			}
			else
			{
				Animation = AnimData_Start_VelocityDirection.Animations.StartForward;
			}
			// if (UKismetMathLibrary::InRange_FloatFloat(Delta, -135, -45, false, true))
			// {
			// 	Animation = AnimData_Start_VelocityDirection.Animations.StartForwardL90;
			// }
			// else if (UKismetMathLibrary::InRange_FloatFloat(Delta, -180, -135, true, true))
			// {
			// 	Animation = AnimData_Start_VelocityDirection.Animations.StartForwardL180;
			// }
			// else if (UKismetMathLibrary::InRange_FloatFloat(Delta, 45, 135, true, false))
			// {
			// 	Animation = AnimData_Start_VelocityDirection.Animations.StartForwardR90;
			// }
			// else if (UKismetMathLibrary::InRange_FloatFloat(Delta, 135, 180, true, true))
			// {
			// 	Animation = AnimData_Start_VelocityDirection.Animations.StartForwardR180;
			// }
			// else
			// {
			// 	Animation = AnimData_Start_VelocityDirection.Animations.StartForward;
			// }
		}
		if (AnimData_Start_VelocityDirection.AnimType == EGMS_StartAnimType_VelocityDir::Single)
		{
			Animation = AnimData_Start_VelocityDirection.Animation;
		}

		if (Animation)
		{
			UE_LOG(LogGMS, VeryVerbose, TEXT("GetStartAnimSequence selected anim:%s with delta:%f"), *Animation->GetName(), Delta);
		}
		else
		{
			UE_LOG(LogGMS, Error, TEXT("GetStartAnimSequence failed to select anim with delta:%f"), Delta);
		}
	}

	if (GetParent()->RotationMode == GMS_RotationModeTags::ViewDirection)
	{
		if (AnimData_Start_ViewDirection.AnimType == EGMS_StartAnimType_ViewDir::Direction_4)
		{
			switch (GetParent()->LocomotionState.LocalVelocityDirection)
			{
			case EGMS_MovementDirection::Forward:
				Animation = AnimData_Start_ViewDirection.Animations.Forward;
				break;
			case EGMS_MovementDirection::Backward:
				Animation = AnimData_Start_ViewDirection.Animations.Backward;
				break;
			case EGMS_MovementDirection::Left:
				Animation = AnimData_Start_ViewDirection.Animations.Left;
				break;
			case EGMS_MovementDirection::Right:
				Animation = AnimData_Start_ViewDirection.Animations.Right;
				break;
			}
		}

		if (AnimData_Start_ViewDirection.AnimType == EGMS_StartAnimType_ViewDir::Direction_8)
		{
			switch (GetParent()->LocomotionState.LocalVelocityOctagonalDirection)
			{
			case EGMS_MovementDirection_8Way::Forward:
				Animation = AnimData_Start_ViewDirection.Animations_8Direction.Forward;
				break;
			case EGMS_MovementDirection_8Way::ForwardLeft:
				Animation = AnimData_Start_ViewDirection.Animations_8Direction.ForwardLeft;
				break;
			case EGMS_MovementDirection_8Way::ForwardRight:
				Animation = AnimData_Start_ViewDirection.Animations_8Direction.ForwardRight;
				break;
			case EGMS_MovementDirection_8Way::Backward:
				Animation = AnimData_Start_ViewDirection.Animations_8Direction.Backward;
				break;
			case EGMS_MovementDirection_8Way::BackwardLeft:
				Animation = AnimData_Start_ViewDirection.Animations_8Direction.BackwardLeft;
				break;
			case EGMS_MovementDirection_8Way::BackwardRight:
				Animation = AnimData_Start_ViewDirection.Animations_8Direction.BackwardRight;
				break;
			case EGMS_MovementDirection_8Way::Left:
				Animation = AnimData_Start_ViewDirection.Animations_8Direction.Left;
				break;
			case EGMS_MovementDirection_8Way::Right:
				Animation = AnimData_Start_ViewDirection.Animations_8Direction.Right;
				break;
			}
		}
	}

	return Animation;
}


const FGMS_StrideWarpingSettings& UGMS_AnimLayer_States_DefaultLocomotion::GetStartStrideWarpingSettings() const
{
	return GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection
		       ? AnimData_Start_VelocityDirection.StrideWarping
		       : AnimData_Start_ViewDirection.StrideWarping;
}

const FGMS_SteeringSettings& UGMS_AnimLayer_States_DefaultLocomotion::GetStartSteeringSettings() const
{
	if (IsVelocityDirection())
	{
		return AnimData_Start_VelocityDirection.Steering;
	}
	const static FGMS_SteeringSettings DisabledSteering = FGMS_SteeringSettings(false, 0, 0);
	return DisabledSteering;
}

void UGMS_AnimLayer_States_DefaultLocomotion::Start_AnimRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	//Setup start anim.
	{
		UAnimSequence* Animation = GetStartAnimSequence();

		StartState.Animation = Animation;

		if (!StartState.Animation)
		{
			UE_LOG(LogGMS, Error, TEXT("Start_AnimRelevant failed to select start animation"));
			return;
		}

		StartState.StrideWarpingAlpha = 0;
		StartState.AccumulatedTime = 0.0f;
		StartState.TimeRemaining = StartState.Animation->GetPlayLength();

		if (GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection)
		{
			StartState.OrientationAlpha = 0.0f;
		}
		if (GetParent()->RotationMode == GMS_RotationModeTags::ViewDirection)
		{
			StartState.OrientationAlpha = AnimData_Start_ViewDirection.AnimType == EGMS_StartAnimType_ViewDir::Direction_4 ? 1.0f : 0.0f;
		}
	}

	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, StartState.Animation);
		USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0.0f);
	}

	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		if (USequencePlayerLibrary::GetSequencePure(SequencePlayer) != StartState.Animation)
		{
			USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, StartState.Animation, 0.2);
			USequencePlayerLibrary::SetStartPosition(SequencePlayer, 0.0f);
		}
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Start_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		const float AccumulatedTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluator);
		StartState.AccumulatedTime = AccumulatedTime;
		auto& SW = GetStartStrideWarpingSettings();
		StartState.StrideWarpingAlpha = SW.bEnabled ? UKismetMathLibrary::MapRangeClamped(AccumulatedTime - SW.BlendInStartOffset, 0, SW.BlendInDurationScaled, 0, 1.f) : 0;

		const FVector2D DesiredPlayRateClamp = GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection
			                                       ? AnimData_Start_VelocityDirection.PlayRateClamp
			                                       : AnimData_Start_ViewDirection.PlayRateClamp;

		StartState.PlayRateClamp = UKismetMathLibrary::MakeVector2D(UKismetMathLibrary::Lerp(0.2, DesiredPlayRateClamp.X, StartState.StrideWarpingAlpha), DesiredPlayRateClamp.Y);

		if (USequenceEvaluatorLibrary::GetSequence(SequenceEvaluator) != nullptr)
		{
			if (FAnimNode_SequenceEvaluator* SequenceEvaluatorPtr = Node.GetAnimNodePtr<FAnimNode_SequenceEvaluator>())
			{
				StartState.TimeRemaining = SequenceEvaluatorPtr->GetCurrentAssetLength() - SequenceEvaluatorPtr->GetCurrentAssetTimePlayRateAdjusted();
			}
			UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, SequenceEvaluator, GetParent()->LocomotionState.PreviousDisplacement, FName("Distance"), StartState.PlayRateClamp);
		}
		else
		{
			UE_LOG(LogGMS, Error, TEXT("Start Sequence evaluator does not have an anim sequence to play.%S"), __FUNCTION__);
		}
	}

	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		const float AccumulatedTime = USequencePlayerLibrary::GetAccumulatedTime(SequencePlayer);
		StartState.AccumulatedTime = AccumulatedTime;
		auto& SW = GetStartStrideWarpingSettings();
		StartState.StrideWarpingAlpha = SW.bEnabled ? UKismetMathLibrary::MapRangeClamped(AccumulatedTime - SW.BlendInStartOffset, 0, SW.BlendInDurationScaled, 0, 1.f) : 0;

		const FVector2D DesiredPlayRateClamp = GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection
			                                       ? AnimData_Start_VelocityDirection.PlayRateClamp
			                                       : AnimData_Start_ViewDirection.PlayRateClamp;

		StartState.PlayRateClamp = UKismetMathLibrary::MakeVector2D(UKismetMathLibrary::Lerp(0.2, DesiredPlayRateClamp.X, StartState.StrideWarpingAlpha), DesiredPlayRateClamp.Y);

		if (USequencePlayerLibrary::GetSequencePure(SequencePlayer) != nullptr)
		{
			if (FAnimNode_SequencePlayer* SequencePlayerPtr = Node.GetAnimNodePtr<FAnimNode_SequencePlayer>())
			{
				StartState.TimeRemaining = SequencePlayerPtr->GetCurrentAssetLength() - SequencePlayerPtr->GetCurrentAssetTimePlayRateAdjusted();
			}
			UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayer, GetParent()->LocomotionState.DisplacementSpeed, DesiredPlayRateClamp); //TODO X过低可能有起步停滞的感觉。{0.8,1.75}
		}
		else
		{
			UE_LOG(LogGMS, Error, TEXT("Start Sequence player does not have an anim sequence to play.%S"), __FUNCTION__);
		}
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Start_StateEntry_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	// when triggering start, use the ywa delta of acceleration direction and root bone direction to get start animation. 
	StartState.YawDeltaToAcceleration = UKismetMathLibrary::NormalizedDeltaRotator(GetParent()->InputDirection.Rotation(), GetParent()->RootState.RootTransform.Rotator()).Yaw;
	StartState.LocalVelocityDirection = GetParent()->LocomotionState.LocalVelocityDirection;
}

void UGMS_AnimLayer_States_DefaultLocomotion::Start_StateUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	FAnimationStateResultReference AnimationStateResult;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationStateResult, Result);

	if (Result == EAnimNodeReferenceConversionResult::Succeeded && !UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationStateResult))
	{
		// try to enable root rotation offset.
		if (IsVelocityDirection() && AnimData_Start_VelocityDirection.bValid &&
			AnimData_Start_VelocityDirection.AnimType == EGMS_StartAnimType_VelocityDir::Reface && AnimData_Start_VelocityDirection.Steering.bEnabled)
		{
			GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Accumulate);
		}
		else
		{
			GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Release);
		}
	}
}


#pragma endregion

#pragma region Cycle

void UGMS_AnimLayer_States_DefaultLocomotion::Cycle_AnimRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
}

void UGMS_AnimLayer_States_DefaultLocomotion::Cycle_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	const FSequencePlayerReference SequencePlayerReference = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	if (Result == EAnimNodeReferenceConversionResult::Failed)
	{
		return;
	}
	if (UAnimSequence* Anim = GetCycleAnimation())
	{
		CycleState.Animation = Anim;

		float AnimationSpeed = AnimData_Cycle.bHasRootMotion ? UGMS_Utility::CalculateAnimatedSpeed(Anim) : AnimData_Cycle.AnimatedSpeed;
		float DesiredPlayRate = GetParent()->LocomotionState.DisplacementSpeed / AnimationSpeed;

		if (AnimData_Cycle.PlayRateClamp.X >= 0.0f && AnimData_Cycle.PlayRateClamp.X < AnimData_Cycle.PlayRateClamp.Y)
		{
			DesiredPlayRate = FMath::Clamp(DesiredPlayRate, AnimData_Cycle.PlayRateClamp.X, AnimData_Cycle.PlayRateClamp.Y);
		}

		CycleState.PlayRate = DesiredPlayRate;
		USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerReference, CycleState.Animation, AnimData_Cycle.BlendTime);

		USequencePlayerLibrary::SetPlayRate(SequencePlayerReference, CycleState.PlayRate);

		CycleState.StrideWarpingAlpha = UKismetMathLibrary::FInterpTo(CycleState.StrideWarpingAlpha, AnimData_Cycle.bEnableStrideWarping ? (GetParent()->bBlocked ? 0.5f : 1.0f) : 0.0f,
		                                                              Context.GetContext()->GetDeltaTime(), 10.0);
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Cycle_StateEntry_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	if (GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection)
	{
		CycleState.OrientationAlpha = 1.0f;
	}
	if (GetParent()->RotationMode == GMS_RotationModeTags::ViewDirection)
	{
		CycleState.OrientationAlpha = AnimData_Cycle.AnimType == EGMS_CycleAnimType::Direction_4 ? 1.0f : 0.0f;
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Cycle_StateUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	// if (CycleState.Animation)
	// {
	// 	float AnimationSpeed = AnimData_Cycle.bHasRootMotion ? UGMS_Utility::CalculateAnimatedSpeed(CycleState.Animation) : AnimData_Cycle.AnimatedSpeed;
	// 	float DesiredPlayRate = GetParent()->LocomotionState.DisplacementSpeed / AnimationSpeed;
	//
	// 	if (AnimData_Cycle.PlayRateClamp.X >= 0.0f && AnimData_Cycle.PlayRateClamp.X < AnimData_Cycle.PlayRateClamp.Y)
	// 	{
	// 		DesiredPlayRate = FMath::Clamp(DesiredPlayRate, AnimData_Cycle.PlayRateClamp.X, AnimData_Cycle.PlayRateClamp.Y);
	// 	}
	// 	CycleState.PlayRate = DesiredPlayRate;
	// }

	GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Release);
}

UAnimSequence* UGMS_AnimLayer_States_DefaultLocomotion::GetCycleAnimation_Implementation() const
{
	UAnimSequence* OutAnim = nullptr;

	if (AnimData_Cycle.AnimType == EGMS_CycleAnimType::Single)
	{
		OutAnim = AnimData_Cycle.Animation;
	}

	if (AnimData_Cycle.AnimType == EGMS_CycleAnimType::Direction_4)
	{
		switch (GetParent()->LocomotionState.LocalVelocityDirectionNoOffset)
		{
		case EGMS_MovementDirection::Forward:
			OutAnim = AnimData_Cycle.Animations.Forward;
			break;
		case EGMS_MovementDirection::Backward:
			OutAnim = AnimData_Cycle.Animations.Backward;
			break;
		case EGMS_MovementDirection::Left:
			OutAnim = AnimData_Cycle.Animations.Left;
			break;
		case EGMS_MovementDirection::Right:
			OutAnim = AnimData_Cycle.Animations.Right;
			break;
		default: ;
		}
	}

	if (AnimData_Cycle.AnimType == EGMS_CycleAnimType::Direction_8)
	{
		switch (GetParent()->LocomotionState.LocalVelocityOctagonalDirection)
		{
		case EGMS_MovementDirection_8Way::Forward:
			OutAnim = AnimData_Cycle.Animations_8Direction.Forward;
			break;
		case EGMS_MovementDirection_8Way::ForwardLeft:
			OutAnim = AnimData_Cycle.Animations_8Direction.ForwardLeft;
			break;
		case EGMS_MovementDirection_8Way::ForwardRight:
			OutAnim = AnimData_Cycle.Animations_8Direction.ForwardRight;
			break;
		case EGMS_MovementDirection_8Way::Backward:
			OutAnim = AnimData_Cycle.Animations_8Direction.Backward;
			break;
		case EGMS_MovementDirection_8Way::BackwardLeft:
			OutAnim = AnimData_Cycle.Animations_8Direction.BackwardLeft;
			break;
		case EGMS_MovementDirection_8Way::BackwardRight:
			OutAnim = AnimData_Cycle.Animations_8Direction.BackwardRight;
			break;
		case EGMS_MovementDirection_8Way::Left:
			OutAnim = AnimData_Cycle.Animations_8Direction.Left;
			break;
		case EGMS_MovementDirection_8Way::Right:
			OutAnim = AnimData_Cycle.Animations_8Direction.Right;
			break;
		default: ;
		}
	}
	return OutAnim;
}

UAnimSequence* UGMS_AnimLayer_States_DefaultLocomotion::GetStopAnimation_Implementation() const
{
	if (AnimData_Stop.AnimType == EGMS_StopAnimType::Single)
	{
		return AnimData_Stop.Animation;
	}
	if (AnimData_Stop.AnimType == EGMS_StopAnimType::Direction_4)
	{
		switch (GetParent()->LocomotionState.LocalVelocityDirection)
		{
		case EGMS_MovementDirection::Forward:
			return AnimData_Stop.Animations.Forward;
		case EGMS_MovementDirection::Backward:
			return AnimData_Stop.Animations.Backward;
		case EGMS_MovementDirection::Left:
			return AnimData_Stop.Animations.Left;
		case EGMS_MovementDirection::Right:
			return AnimData_Stop.Animations.Right;
		}
	}
	if (AnimData_Stop.AnimType == EGMS_StopAnimType::Direction_8)
	{
		switch (GetParent()->LocomotionState.LocalVelocityOctagonalDirection)
		{
		case EGMS_MovementDirection_8Way::Forward:
			return AnimData_Stop.Animations_8Direction.Forward;
		case EGMS_MovementDirection_8Way::ForwardLeft:
			return AnimData_Stop.Animations_8Direction.ForwardLeft;
		case EGMS_MovementDirection_8Way::ForwardRight:
			return AnimData_Stop.Animations_8Direction.ForwardRight;
		case EGMS_MovementDirection_8Way::Backward:
			return AnimData_Stop.Animations_8Direction.Backward;
		case EGMS_MovementDirection_8Way::BackwardLeft:
			return AnimData_Stop.Animations_8Direction.BackwardLeft;
		case EGMS_MovementDirection_8Way::BackwardRight:
			return AnimData_Stop.Animations_8Direction.BackwardRight;
		case EGMS_MovementDirection_8Way::Left:
			return AnimData_Stop.Animations_8Direction.Left;
		case EGMS_MovementDirection_8Way::Right:
			return AnimData_Stop.Animations_8Direction.Right;
		}
	}
	return nullptr;
}


#pragma endregion

#pragma region Stop

bool UGMS_AnimLayer_States_DefaultLocomotion::ShouldDistanceMatchStop() const
{
	if (GetParent()->bAnyMontagePlaying)
	{
		return false;
	}
	return GetParent()->LocomotionState.bHasVelocity && !GetParent()->LocomotionState.bHasInput;
}

void UGMS_AnimLayer_States_DefaultLocomotion::Stop_AnimRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (Result == EAnimNodeReferenceConversionResult::Failed)
	{
		return;
	}

	UAnimSequence* Animation = GetStopAnimation();

	if (IsValid(Animation))
	{
		StopState.Animation = Animation;

		USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, Animation);

		// If we got here, and we can't distance match a stop on start, match to 0 distance
		if (!ShouldDistanceMatchStop())
		{
			// immediately match to stop point.
			UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, 0.f, FName("Distance"));
		}
	}
	else
	{
		UE_LOG(LogGMS, Error, TEXT("Sequence evaluator does not have an anim sequence to play.%S"), __FUNCTION__);
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Stop_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (Result == EAnimNodeReferenceConversionResult::Failed)
	{
		return;
	}

	if (USequenceEvaluatorLibrary::GetSequence(SequenceEvaluator) != nullptr)
	{
		if (ShouldDistanceMatchStop())
		{
			float DistanceToMatch = GetDistanceToStopTarget();
			if (DistanceToMatch > 0.f)
			{
				if (GetParent()->bAnyMontagePlaying)
				{
					UE_LOG(LogGMS, Warning, TEXT("Cancel Distance matching while montage active."));
					return;
				}
  				UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, DistanceToMatch, FName("Distance"));
				return;
			}
		}
		USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEvaluator, 1);
	}
	else
	{
		UE_LOG(LogGMS, Error, TEXT("Sequence evaluator does not have an anim sequence to play.%S"), __FUNCTION__);
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Stop_StateRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	StopState.Animation = nullptr;
	if (GetParent()->RotationMode == GMS_RotationModeTags::VelocityDirection)
	{
		StopState.OrientationAlpha = 1.0f;
	}
	if (GetParent()->RotationMode == GMS_RotationModeTags::ViewDirection)
	{
		StopState.OrientationAlpha = AnimData_Stop.AnimType == EGMS_StopAnimType::Direction_4 ? 1.0f : 0.0f;
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Stop_StateUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	FAnimationStateResultReference AnimationStateResult;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationStateResult, Result);

	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationStateResult))
		{
#if ENGINE_MINOR_VERSION > 4
			GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::LockOffsetAndIgnoreAnimation);
#else
			GetParent()->SetOffsetRootBoneRotationMode(EOffsetRootBoneMode::Hold);
#endif
		}
	}
}

float UGMS_AnimLayer_States_DefaultLocomotion::GetDistanceToStopTarget() const
{
	FGMS_PredictGroundMovementStopLocationParams Params = MSC->GetPredictGroundMovementStopLocationParams();
	return UKismetMathLibrary::VSizeXY(UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
		Params.Velocity,
		Params.bUseSeparateBrakingFriction,
		Params.BrakingFriction,
		Params.GroundFriction,
		Params.BrakingFriction,
		Params.BrakingDecelerationWalking));
}

#pragma endregion

#pragma region Pivot

void UGMS_AnimLayer_States_DefaultLocomotion::Pivot_AnimRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	PivotState.StartingAcceleration = GetParent()->LocomotionState.LocalAcceleration2D;

	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (Result == EAnimNodeReferenceConversionResult::Failed)
	{
		return;
	}

	UAnimSequence* DesiredAnim = nullptr;
	switch (PivotState.DesiredDirection)
	{
	case EGMS_MovementDirection::Forward:
		DesiredAnim = AnimData_Pivot.Animations.Forward;
		break;
	case EGMS_MovementDirection::Backward:
		DesiredAnim = AnimData_Pivot.Animations.Backward;
		break;
	case EGMS_MovementDirection::Left:
		DesiredAnim = AnimData_Pivot.Animations.Left;
		break;
	case EGMS_MovementDirection::Right:
		DesiredAnim = AnimData_Pivot.Animations.Right;
		break;
	}

	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, DesiredAnim);
	PivotState.Animation = DesiredAnim;

	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0.f);

	PivotState.RemainingCooldown = AnimData_Pivot.PivotCooldown;
	PivotState.TimeAtPivotStop = 0;
	PivotState.AccumulatedTime = 0;

	UE_LOG(LogGMS, VeryVerbose, TEXT("Pivot_AnimRelevant selected anim:%s"), *PivotState.Animation.GetName());
}

void UGMS_AnimLayer_States_DefaultLocomotion::Pivot_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Failed)
	{
		return;
	}

	PivotState.AccumulatedTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluator);

	if (PivotState.RemainingCooldown > 0)
	{
		UAnimSequence* NewDesiredAnim = nullptr;

		switch (PivotState.DesiredDirection)
		{
		case EGMS_MovementDirection::Forward:
			NewDesiredAnim = AnimData_Pivot.Animations.Forward;
			break;
		case EGMS_MovementDirection::Backward:
			NewDesiredAnim = AnimData_Pivot.Animations.Backward;
			break;
		case EGMS_MovementDirection::Left:
			NewDesiredAnim = AnimData_Pivot.Animations.Left;
			break;
		case EGMS_MovementDirection::Right:
			NewDesiredAnim = AnimData_Pivot.Animations.Right;
			break;
		default: ;
		}

		if (NewDesiredAnim != USequenceEvaluatorLibrary::GetSequence(SequenceEvaluator))
		{
			USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEvaluator, NewDesiredAnim, 0.2f);
			PivotState.StartingAcceleration = GetParent()->LocomotionState.LocalAcceleration2D;
			PivotState.Animation = NewDesiredAnim;
			UE_LOG(LogGMS, VeryVerbose, TEXT("Pivot_AnimUpdate selected new anim:%s"), *PivotState.Animation.GetName());
		}
	}

	//Does acceleration oppose velocity?
	if (FVector::DotProduct(GetParent()->LocomotionState.LocalVelocity2D, GetParent()->LocomotionState.LocalAcceleration2D) < 0)
	{
		//While acceleration opposes velocity, the character is still approaching the pivot point, so we distance match to that point.
		FGMS_PredictGroundMovementPivotLocationParams Params = MSC->GetPredictGroundMovementPivotLocationParams();
		const float DistanceToTarget = UAnimCharacterMovementLibrary::PredictGroundMovementPivotLocation(Params.Acceleration, Params.Velocity, Params.GroundFriction).Size2D();

		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, DistanceToTarget, FName("Distance"));
		PivotState.TimeAtPivotStop = PivotState.AccumulatedTime;
	}
	else
	{
		//Alpha = (ExplicitTime - StopTime - Offset)/Duration We want the blend in to start after we've already stopped, and just started accelerating
		PivotState.StrideWarpingAlpha = AnimData_Pivot.StrideWarping.bEnabled
			                                ? UKismetMathLibrary::MapRangeClamped(PivotState.AccumulatedTime - PivotState.TimeAtPivotStop - AnimData_Pivot.StrideWarping.BlendInStartOffset,
			                                                                      0.f,
			                                                                      AnimData_Pivot.StrideWarping.BlendInDurationScaled,
			                                                                      0.f,
			                                                                      1.f)
			                                : 0;

		// Smoothly increase the minimum playrate speed, as we blend in stride warping
		const float PlayRateClamp_X = UKismetMathLibrary::Lerp(0.2f, AnimData_Pivot.PlayRateClamp.X, PivotState.StrideWarpingAlpha);
		PivotState.PlayRateClamp = UKismetMathLibrary::MakeVector2D(PlayRateClamp_X, AnimData_Pivot.PlayRateClamp.Y);

		if (USequenceEvaluatorLibrary::GetSequence(SequenceEvaluator) != nullptr)
		{
			UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, SequenceEvaluator, GetParent()->LocomotionState.PreviousDisplacement, FName("Distance"), PivotState.PlayRateClamp);
		}
		else
		{
			UE_LOG(LogGMS, Error, TEXT("Sequence evaluator does not have an anim sequence to play.%S"), __FUNCTION__);
		}
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Pivot_StateRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	PivotState.InitialDirection = GetParent()->LocomotionState.LocalVelocityDirection;

	UE_LOG(LogGMS, VeryVerbose, TEXT("Pivot_StateRelevant selected direction:%d"), PivotState.InitialDirection);
}

void UGMS_AnimLayer_States_DefaultLocomotion::Pivot_StateUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	if (PivotState.RemainingCooldown > 0)
	{
		PivotState.RemainingCooldown -= Context.GetContext()->GetDeltaTime();
	}

	PivotState.bMovingPerpendicularToInitialDirection = IsMovingPerpendicularToInitialPivot();
}

#pragma endregion

#pragma region Jump
void UGMS_AnimLayer_States_DefaultLocomotion::JumpStart_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	const FSequencePlayerReference SequencePlayerReference = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	UAnimSequenceBase* Animation = AnimData_Jump.JumpStart;

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerReference, Animation, 0.2f);
}

void UGMS_AnimLayer_States_DefaultLocomotion::JumpStartLoop_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	const FSequencePlayerReference SequencePlayerReference = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	UAnimSequenceBase* Animation = AnimData_Jump.JumpStartLoop;

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerReference, Animation, 0.2f);
}

void UGMS_AnimLayer_States_DefaultLocomotion::JumpApex_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	const FSequencePlayerReference SequencePlayerReference = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	UAnimSequenceBase* Animation = AnimData_Jump.JumpApex;

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerReference, Animation, 0.2f);
}

void UGMS_AnimLayer_States_DefaultLocomotion::FallLoop_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	const FSequencePlayerReference SequencePlayerReference = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	UAnimSequenceBase* Animation = AnimData_Jump.JumpFallLoop;

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerReference, Animation, 0.2f);
}

void UGMS_AnimLayer_States_DefaultLocomotion::FallLand_AnimRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	UAnimSequenceBase* Animation = AnimData_Jump.JumpFallLand;

	const FSequenceEvaluatorReference Reference = USequenceEvaluatorLibrary::SetSequence(USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result), Animation);

	USequenceEvaluatorLibrary::SetExplicitTime(Reference, 0.f);
}

void UGMS_AnimLayer_States_DefaultLocomotion::FallLand_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	const FSequenceEvaluatorReference Reference = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	UAnimDistanceMatchingLibrary::DistanceMatchToTarget(Reference, GetParent()->InAirState.GroundDistance, FName("GroundDistance"));
}

bool UGMS_AnimLayer_States_DefaultLocomotion::Rule_JumpApex_Implementation() const
{
	if (IsValid(GetParent()))
	{
		return GetParent()->InAirState.TimeToJumpApex <= 0.4 && AnimData_Jump.bValidJumpApex;
	}
	return false;
}

bool UGMS_AnimLayer_States_DefaultLocomotion::Rule_FallToFallLand_Implementation() const
{
	return AnimData_Jump.bValidJumpFallLand && GetParent()->InAirState.bValidGround && GetParent()->InAirState.GroundDistance < 200.0f;
}
#pragma endregion

#pragma region Land


void UGMS_AnimLayer_States_DefaultLocomotion::Land_AnimRelevant_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result = EAnimNodeReferenceConversionResult::Succeeded;

	float LandedSpeed = FMath::Abs(GetParent()->InAirState.VerticalSpeed);
	if (UAnimSequence* Animation = UGMS_Utility::SelectAnimationWithFloat(AnimData_Land.Lands, LandedSpeed))
	{
		const FSequencePlayerReference Reference = USequencePlayerLibrary::SetSequence(USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result), Animation);

		USequencePlayerLibrary::SetAccumulatedTime(Reference, 0.f);
		UE_LOG(LogGMS, VeryVerbose, TEXT("Land_AnimRelevant selected animation:%s at VerticalSpeed of %f"), *Animation->GetName(), LandedSpeed);
	}
	else
	{
		UE_LOG(LogGMS, Error, TEXT("Land_AnimRelevant failed to select animation at VerticalSpeed of %f"), LandedSpeed)
	}
}

void UGMS_AnimLayer_States_DefaultLocomotion::Land_AnimUpdate_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
}
#pragma endregion
