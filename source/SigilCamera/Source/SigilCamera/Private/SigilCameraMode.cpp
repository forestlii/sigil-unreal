// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilCameraMode.h"

#include "SigilCameraSystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilCameraMode)


//////////////////////////////////////////////////////////////////////////
// FSigilMovementCameraModeView
//////////////////////////////////////////////////////////////////////////
FSigilCameraModeView::FSigilCameraModeView()
	: Location(ForceInit)
	  , Rotation(ForceInit)
	  , ControlRotation(ForceInit)
	  , FieldOfView(80.0f)
{
}

void FSigilCameraModeView::Blend(const FSigilCameraModeView& Other, float OtherWeight)
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

	SpringArmSocketOffset = FMath::Lerp(SpringArmSocketOffset, Other.SpringArmSocketOffset, OtherWeight);
	SpringArmTargetOffset = FMath::Lerp(SpringArmTargetOffset, Other.SpringArmTargetOffset, OtherWeight);
	SpringArmLength = FMath::Lerp(SpringArmLength, Other.SpringArmLength, OtherWeight);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}


//////////////////////////////////////////////////////////////////////////
// USigilCameraMode
//////////////////////////////////////////////////////////////////////////
USigilCameraMode::USigilCameraMode()
{
	FieldOfView = 80.0f;
	ViewPitchMin = -89.0f;
	ViewPitchMax = 89.0f;

	BlendTime = 0.5f;
	BlendFunction = ESigilCameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;
	ActiveTime = 0.0f;
	MaxActiveTime = 0.0f;
}

UWorld* USigilCameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* USigilCameraMode::GetTargetActor() const
{
	if (USigilCameraSystemComponent* Component = Cast<USigilCameraSystemComponent>(GetOuter()))
	{
		return Component->GetOwner();
	}
	return nullptr;
}

FVector USigilCameraMode::GetPivotLocation_Implementation() const
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

FRotator USigilCameraMode::GetPivotRotation_Implementation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	return TargetActor->GetActorRotation();
}

void USigilCameraMode::UpdateCameraMode(float DeltaTime)
{
	ActiveTime += DeltaTime;

	if (MaxActiveTime > 0 && ActiveTime >= MaxActiveTime)
	{
		if (USigilCameraSystemComponent* Component = Cast<USigilCameraSystemComponent>(GetOuter()))
		{
			Component->PushDefaultCameraMode();
		}
	}

	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void USigilCameraMode::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	OnUpdateView(DeltaTime, PivotLocation, PivotRotation);
}

void USigilCameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
	const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

	switch (BlendFunction)
	{
	case ESigilCameraModeBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case ESigilCameraModeBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case ESigilCameraModeBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case ESigilCameraModeBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	default:
		checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

UCameraComponent* USigilCameraMode::GetAssociatedCamera() const
{
	if (USigilCameraSystemComponent* Component = Cast<USigilCameraSystemComponent>(GetOuter()))
	{
		return Component->GetAssociatedCamera();
	}
	return nullptr;
}

USpringArmComponent* USigilCameraMode::GetAssociatedSpringArm() const
{
	if (USigilCameraSystemComponent* Component = Cast<USigilCameraSystemComponent>(GetOuter()))
	{
		return Component->GetAssociatedSpringArm();
	}
	return nullptr;
}

void USigilCameraMode::UpdateBlending(float DeltaTime)
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
	case ESigilCameraModeBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case ESigilCameraModeBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case ESigilCameraModeBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case ESigilCameraModeBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	default:
		checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void USigilCameraMode::OnUpdateView_Implementation(float DeltaTime, FVector PivotLocation, FRotator PivotRotation)
{
	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void USigilCameraMode::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("      SigilMovementCameraMode: %s (%f)"), *GetName(), BlendWeight));
}
