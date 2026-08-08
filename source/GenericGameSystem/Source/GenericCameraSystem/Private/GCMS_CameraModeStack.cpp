// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GCMS_CameraModeStack.h"

#include "Engine/Canvas.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCMS_CameraModeStack)

//////////////////////////////////////////////////////////////////////////
// UGCMS_CameraModeStack
//////////////////////////////////////////////////////////////////////////
UGCMS_CameraModeStack::UGCMS_CameraModeStack()
{
	bIsActive = true;
}

void UGCMS_CameraModeStack::ActivateStack()
{
	if (!bIsActive)
	{
		bIsActive = true;

		// Notify camera modes that they are being activated.
		for (UGCMS_CameraMode* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnActivation();
		}
	}
}

void UGCMS_CameraModeStack::DeactivateStack()
{
	if (bIsActive)
	{
		bIsActive = false;

		// Notify camera modes that they are being deactivated.
		for (UGCMS_CameraMode* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnDeactivation();
		}
	}
}

void UGCMS_CameraModeStack::PushCameraMode(TSubclassOf<UGCMS_CameraMode> CameraModeClass)
{
	if (!CameraModeClass)
	{
		return;
	}

	// get camera from pool.
	UGCMS_CameraMode* NewCameraMode = GetCameraModeInstance(CameraModeClass);
	check(NewCameraMode);

	int32 StackSize = CameraModeStack.Num();

	if ((StackSize > 0) && (CameraModeStack[0] == NewCameraMode))
	{
		// Already top of stack.
		return;
	}

	// See if it's already in the stack and remove it.
	// Figure out how much it was contributing to the stack.
	int32 ExistingStackIndex = INDEX_NONE;
	float ExistingStackContribution = 1.0f;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		if (CameraModeStack[StackIndex] == NewCameraMode)
		{
			ExistingStackIndex = StackIndex;
			ExistingStackContribution *= NewCameraMode->GetBlendWeight();
			break;
		}
		else
		{
			ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
		}
	}

	// existing in stack, remove it before add.
	if (ExistingStackIndex != INDEX_NONE)
	{
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}
	else
	{
		ExistingStackContribution = 0.0f;
	}

	// Decide what initial weight to start with.
	const bool bShouldBlend = ((NewCameraMode->GetBlendTime() > 0.0f) && (StackSize > 0));
	const float BlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

	NewCameraMode->SetBlendWeight(BlendWeight);

	// Add new entry to top of stack.
	CameraModeStack.Insert(NewCameraMode, 0);

	// Make sure stack bottom is always weighted 100%.
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	// Let the camera mode know if it's being added to the stack.
	if (ExistingStackIndex == INDEX_NONE)
	{
		NewCameraMode->OnActivation();
	}
}

bool UGCMS_CameraModeStack::EvaluateStack(float DeltaTime, FGCMS_CameraModeView& OutCameraModeView)
{
	if (!bIsActive)
	{
		return false;
	}

	//Update camera modes. 
	UpdateStack(DeltaTime);

	//Blend values from camera modes.
	BlendStack(OutCameraModeView);

	return true;
}

UGCMS_CameraMode* UGCMS_CameraModeStack::GetCameraModeInstance(TSubclassOf<UGCMS_CameraMode> CameraModeClass)
{
	check(CameraModeClass);

	// First see if we already created one.
	for (UGCMS_CameraMode* CameraMode : CameraModeInstances)
	{
		if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass))
		{
			return CameraMode;
		}
	}

	// Not found, so we need to create it.
	UGCMS_CameraMode* NewCameraMode = NewObject<UGCMS_CameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);

	CameraModeInstances.Add(NewCameraMode);

	return NewCameraMode;
}

void UGCMS_CameraModeStack::UpdateStack(float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}

	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		UGCMS_CameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		CameraMode->UpdateCameraMode(DeltaTime);

		if (CameraMode->GetBlendWeight() >= 1.0f)
		{
			// Everything below this mode is now irrelevant and can be removed.
			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);
			break;
		}
	}

	if (RemoveCount > 0)
	{
		// Let the camera modes know they being removed from the stack.
		for (int32 StackIndex = RemoveIndex; StackIndex < StackSize; ++StackIndex)
		{
			UGCMS_CameraMode* CameraMode = CameraModeStack[StackIndex];
			check(CameraMode);

			CameraMode->OnDeactivation();
		}

		CameraModeStack.RemoveAt(RemoveIndex, RemoveCount);
	}
}

void UGCMS_CameraModeStack::BlendStack(FGCMS_CameraModeView& OutCameraModeView) const
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}

	// Start at the bottom and blend up the stack
	const UGCMS_CameraMode* CameraMode = CameraModeStack[StackSize - 1];
	check(CameraMode);

	OutCameraModeView = CameraMode->GetCameraModeView();

	for (int32 StackIndex = (StackSize - 2); StackIndex >= 0; --StackIndex)
	{
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
	}
}

void UGCMS_CameraModeStack::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("   --- Camera Modes (Begin) ---")));

	for (const UGCMS_CameraMode* CameraMode : CameraModeStack)
	{
		check(CameraMode);
		CameraMode->DrawDebug(Canvas);
	}

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   --- Camera Modes (End) ---")));
}

void UGCMS_CameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	if (CameraModeStack.Num() == 0)
	{
		OutWeightOfTopLayer = 1.0f;
		OutTagOfTopLayer = FGameplayTag();
		return;
	}
	else
	{
		UGCMS_CameraMode* TopEntry = CameraModeStack.Last();
		check(TopEntry);
		OutWeightOfTopLayer = TopEntry->GetBlendWeight();
		OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
	}
}
