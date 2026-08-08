// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GCMS_CameraMode.h"

#include "GCMS_CameraSystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCMS_CameraMode)


//////////////////////////////////////////////////////////////////////////
// FGMS_CameraModeView
//////////////////////////////////////////////////////////////////////////
FGCMS_CameraModeView::FGCMS_CameraModeView()
	: Location(ForceInit)
	  , Rotation(ForceInit)
	  , ControlRotation(ForceInit)
	  , FieldOfView(80.0f)
{
}

void FGCMS_CameraModeView::Blend(const FGCMS_CameraModeView& Other, float OtherWeight)
{
	if (OtherWeight <= 0.0f)
	{
		return;
	}
	else if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	SprintArmSocketOffset = FMath::Lerp(SprintArmSocketOffset, Other.SprintArmSocketOffset, OtherWeight);
	SprintArmTargetOffset = FMath::Lerp(SprintArmTargetOffset, Other.SprintArmTargetOffset, OtherWeight);
	SprintArmLength = FMath::Lerp(SprintArmLength, Other.SprintArmLength, OtherWeight);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}


//////////////////////////////////////////////////////////////////////////
// UGCMS_CameraMode
//////////////////////////////////////////////////////////////////////////
UGCMS_CameraMode::UGCMS_CameraMode()
{
	FieldOfView = 80.0f;
	ViewPitchMin = -89.0f;
	ViewPitchMax = 89.0f;

	BlendTime = 0.5f;
	BlendFunction = EGCMS_CameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;
	ActiveTime = 0.0f;
	MaxActiveTime = 0.0f;
}

UWorld* UGCMS_CameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UGCMS_CameraMode::GetTargetActor() const
{
	if (UGCMS_CameraSystemComponent* Component = Cast<UGCMS_CameraSystemComponent>(GetOuter()))
	{
		return Component->GetOwner();
	}
	return nullptr;
}

FVector UGCMS_CameraMode::GetPivotLocation_Implementation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

			return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
		}

		return TargetPawn->GetPawnViewLocation();
	}

	return TargetActor->GetActorLocation();
}

FRotator UGCMS_CameraMode::GetPivotRotation_Implementation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	return TargetActor->GetActorRotation();
}

void UGCMS_CameraMode::UpdateCameraMode(float DeltaTime)
{
	ActiveTime += DeltaTime;

	if (MaxActiveTime > 0 && ActiveTime >= MaxActiveTime)
	{
		if (UGCMS_CameraSystemComponent* Component = Cast<UGCMS_CameraSystemComponent>(GetOuter()))
		{
			Component->PushDefaultCameraMode();
		}
	}

	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void UGCMS_CameraMode::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	OnUpdateView(DeltaTime, PivotLocation, PivotRotation);
}

void UGCMS_CameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
	const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

	switch (BlendFunction)
	{
	case EGCMS_CameraModeBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case EGCMS_CameraModeBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EGCMS_CameraModeBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EGCMS_CameraModeBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	default:
		checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

UCameraComponent* UGCMS_CameraMode::GetAssociatedCamera() const
{
	if (UGCMS_CameraSystemComponent* Component = Cast<UGCMS_CameraSystemComponent>(GetOuter()))
	{
		return Component->GetAssociatedCamera();
	}
	return nullptr;
}

USpringArmComponent* UGCMS_CameraMode::GetAssociatedSprintArm() const
{
	if (UGCMS_CameraSystemComponent* Component = Cast<UGCMS_CameraSystemComponent>(GetOuter()))
	{
		return Component->GetAssociatedSprintArm();
	}
	return nullptr;
}

void UGCMS_CameraMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

	switch (BlendFunction)
	{
	case EGCMS_CameraModeBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case EGCMS_CameraModeBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EGCMS_CameraModeBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EGCMS_CameraModeBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	default:
		checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UGCMS_CameraMode::OnUpdateView_Implementation(float DeltaTime, FVector PivotLocation, FRotator PivotRotation)
{
	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void UGCMS_CameraMode::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("      GMS_CameraMode: %s (%f)"), *GetName(), BlendWeight));
}
