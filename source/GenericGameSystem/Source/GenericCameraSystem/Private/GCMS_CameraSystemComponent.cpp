// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GCMS_CameraSystemComponent.h"
#include "GameFramework/HUD.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GCMS_CameraMode.h"
#include "GCMS_CameraModeStack.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCMS_CameraSystemComponent)


UGCMS_CameraSystemComponent::UGCMS_CameraSystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
	PrimaryComponentTick.bCanEverTick = true;
	FieldOfViewOffset = 0.0f;
}

void UGCMS_CameraSystemComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (DisplayInfo.IsDisplayOn(TEXT("CAMERA")))
	{
		if (const UGCMS_CameraSystemComponent* CameraComponent = GetCameraSystemComponent(HUD->GetCurrentDebugTargetActor()))
		{
			CameraComponent->DrawDebug(Canvas);
		}
	}
}

void UGCMS_CameraSystemComponent::OnRegister()
{
	Super::OnRegister();

	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UGCMS_CameraModeStack>(this);
		check(CameraModeStack);
	}
}

void UGCMS_CameraSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CameraModeStack && CameraModeStack->IsStackActivate() && AssociatedCameraComponent && AssociatedSprintArmComponent)
	{
		UpdateCameraModes();
		FGCMS_CameraModeView CameraModeView;
		CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

		// Keep player controller in sync with the latest view.
		if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
		{
			if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
			{
				PC->SetControlRotation(CameraModeView.ControlRotation);
			}
		}

		AssociatedCameraComponent->FieldOfView = CameraModeView.FieldOfView;

		AssociatedSprintArmComponent->TargetArmLength = CameraModeView.SprintArmLength;

		AssociatedSprintArmComponent->SocketOffset = CameraModeView.SprintArmSocketOffset;
		AssociatedSprintArmComponent->TargetOffset = CameraModeView.SprintArmTargetOffset;
	}
}

UCameraComponent* UGCMS_CameraSystemComponent::GetAssociatedCamera() const
{
	return AssociatedCameraComponent;
}

USpringArmComponent* UGCMS_CameraSystemComponent::GetAssociatedSprintArm() const
{
	return AssociatedSprintArmComponent;
}

void UGCMS_CameraSystemComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	if (CameraModeStack)
	{
		if (IsActive())
		{
			CameraModeStack->ActivateStack();
		}
		else
		{
			CameraModeStack->DeactivateStack();
		}
	}
}

void UGCMS_CameraSystemComponent::Deactivate()
{
	Super::Deactivate();
	if (CameraModeStack)
	{
		CameraModeStack->DeactivateStack();
	}
}

void UGCMS_CameraSystemComponent::UpdateCameraModes()
{
	check(CameraModeStack);

	if (CameraModeStack->IsStackActivate())
	{
		if (DetermineCameraModeDelegate.IsBound())
		{
			if (const TSubclassOf<UGCMS_CameraMode> CameraMode = DetermineCameraModeDelegate.Execute())
			{
				CameraModeStack->PushCameraMode(CameraMode);
			}
		}
	}
}

void UGCMS_CameraSystemComponent::PushCameraMode(TSubclassOf<UGCMS_CameraMode> NewCameraMode)
{
	if (CameraModeStack->IsStackActivate())
	{
		if (NewCameraMode)
		{
			CameraModeStack->PushCameraMode(NewCameraMode);
		}
	}
}

void UGCMS_CameraSystemComponent::PushDefaultCameraMode()
{
	if (CameraModeStack->IsStackActivate())
	{
		if (DefaultCameraMode)
		{
			CameraModeStack->PushCameraMode(DefaultCameraMode);
		}
	}
}

void UGCMS_CameraSystemComponent::Initialize(UCameraComponent* NewCameraComponent, USpringArmComponent* NewSpringArmComponent)
{
	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UGCMS_CameraModeStack>(this);
		check(CameraModeStack);
	}

	AssociatedCameraComponent = NewCameraComponent;
	AssociatedSprintArmComponent = NewSpringArmComponent;
}

void UGCMS_CameraSystemComponent::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("GCMS_CameraSystemComponent: %s"), *GetNameSafe(GetTargetActor())));

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Location: %s"), *AssociatedCameraComponent->GetComponentLocation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Rotation: %s"), *AssociatedCameraComponent->GetComponentRotation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   SprintArmLength: %f"), AssociatedSprintArmComponent->TargetArmLength));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   SprintArmSocketOffset: %s"), *AssociatedSprintArmComponent->SocketOffset.ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   SprintArmTargetOffset: %s"), *AssociatedSprintArmComponent->TargetOffset.ToCompactString()));

	DisplayDebugManager.DrawString(FString::Printf(TEXT("   FOV: %f"), AssociatedCameraComponent->FieldOfView));

	check(CameraModeStack);
	CameraModeStack->DrawDebug(Canvas);
}

void UGCMS_CameraSystemComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}
