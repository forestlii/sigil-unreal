// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilCameraSystemComponent.h"
#include "GameFramework/HUD.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "SigilCameraMode.h"
#include "SigilCameraModeStack.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilCameraSystemComponent)


USigilCameraSystemComponent::USigilCameraSystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
	PrimaryComponentTick.bCanEverTick = true;
	FieldOfViewOffset = 0.0f;
}

void USigilCameraSystemComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (DisplayInfo.IsDisplayOn(TEXT("CAMERA")))
	{
		if (const USigilCameraSystemComponent* CameraComponent = GetCameraSystemComponent(HUD->GetCurrentDebugTargetActor()))
		{
			CameraComponent->DrawDebug(Canvas);
		}
	}
}

void USigilCameraSystemComponent::OnRegister()
{
	Super::OnRegister();

	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<USigilCameraModeStack>(this);
		check(CameraModeStack);
	}
}

void USigilCameraSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CameraModeStack && CameraModeStack->IsStackActivate() && AssociatedCameraComponent && AssociatedSpringArmComponent)
	{
		UpdateCameraModes();
		FSigilCameraModeView CameraModeView;
		if (!CameraModeStack->EvaluateStack(DeltaTime, CameraModeView))
		{
			return;
		}

		// Keep player controller in sync with the latest view.
		if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
		{
			if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
			{
				PC->SetControlRotation(CameraModeView.ControlRotation);
			}
		}

		// One-frame FOV offset: consume it here and reset so callers must re-apply it every frame.
		AssociatedCameraComponent->FieldOfView = CameraModeView.FieldOfView + FieldOfViewOffset;
		FieldOfViewOffset = 0.0f;

		AssociatedSpringArmComponent->TargetArmLength = CameraModeView.SpringArmLength;

		AssociatedSpringArmComponent->SocketOffset = CameraModeView.SpringArmSocketOffset;
		AssociatedSpringArmComponent->TargetOffset = CameraModeView.SpringArmTargetOffset;
	}
}

UCameraComponent* USigilCameraSystemComponent::GetAssociatedCamera() const
{
	return AssociatedCameraComponent;
}

USpringArmComponent* USigilCameraSystemComponent::GetAssociatedSpringArm() const
{
	return AssociatedSpringArmComponent;
}

void USigilCameraSystemComponent::Activate(bool bReset)
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

void USigilCameraSystemComponent::Deactivate()
{
	Super::Deactivate();
	if (CameraModeStack)
	{
		CameraModeStack->DeactivateStack();
	}
}

void USigilCameraSystemComponent::UpdateCameraModes()
{
	check(CameraModeStack);

	if (CameraModeStack->IsStackActivate())
	{
		if (DetermineCameraModeDelegate.IsBound())
		{
			if (const TSubclassOf<USigilCameraMode> CameraMode = DetermineCameraModeDelegate.Execute())
			{
				CameraModeStack->PushCameraMode(CameraMode);
			}
		}
	}
}

void USigilCameraSystemComponent::PushCameraMode(TSubclassOf<USigilCameraMode> NewCameraMode)
{
	if (CameraModeStack->IsStackActivate())
	{
		if (NewCameraMode)
		{
			CameraModeStack->PushCameraMode(NewCameraMode);
		}
	}
}

void USigilCameraSystemComponent::PushDefaultCameraMode()
{
	if (CameraModeStack->IsStackActivate())
	{
		if (DefaultCameraMode)
		{
			CameraModeStack->PushCameraMode(DefaultCameraMode);
		}
	}
}

void USigilCameraSystemComponent::Initialize(UCameraComponent* NewCameraComponent, USpringArmComponent* NewSpringArmComponent)
{
	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<USigilCameraModeStack>(this);
		check(CameraModeStack);
	}

	AssociatedCameraComponent = NewCameraComponent;
	AssociatedSpringArmComponent = NewSpringArmComponent;
}

void USigilCameraSystemComponent::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("SigilCameraSystemComponent: %s"), *GetNameSafe(GetTargetActor())));

	DisplayDebugManager.SetDrawColor(FColor::White);
	if (AssociatedCameraComponent)
	{
		DisplayDebugManager.DrawString(FString::Printf(TEXT("   Location: %s"), *AssociatedCameraComponent->GetComponentLocation().ToCompactString()));
		DisplayDebugManager.DrawString(FString::Printf(TEXT("   Rotation: %s"), *AssociatedCameraComponent->GetComponentRotation().ToCompactString()));
		DisplayDebugManager.DrawString(FString::Printf(TEXT("   FOV: %f"), AssociatedCameraComponent->FieldOfView));
	}
	else
	{
		DisplayDebugManager.DrawString(TEXT("   Camera: <not initialized - call Initialize(Camera, SpringArm)>"));
	}
	if (AssociatedSpringArmComponent)
	{
		DisplayDebugManager.DrawString(FString::Printf(TEXT("   SpringArmLength: %f"), AssociatedSpringArmComponent->TargetArmLength));
		DisplayDebugManager.DrawString(FString::Printf(TEXT("   SpringArmSocketOffset: %s"), *AssociatedSpringArmComponent->SocketOffset.ToCompactString()));
		DisplayDebugManager.DrawString(FString::Printf(TEXT("   SpringArmTargetOffset: %s"), *AssociatedSpringArmComponent->TargetOffset.ToCompactString()));
	}
	else
	{
		DisplayDebugManager.DrawString(TEXT("   SpringArm: <not initialized>"));
	}

	if (CameraModeStack)
	{
		CameraModeStack->DrawDebug(Canvas);
	}
}

void USigilCameraSystemComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}
