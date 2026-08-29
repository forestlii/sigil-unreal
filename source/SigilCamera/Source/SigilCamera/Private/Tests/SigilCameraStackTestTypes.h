// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "SigilCameraMode.h"
#include "SigilCameraSystemComponent.h"
#include "SigilCameraStackTestTypes.generated.h"

UCLASS()
class USigilCameraStackTestComponent final : public USigilCameraSystemComponent
{
	GENERATED_BODY()

public:
	void TickForTest(const float DeltaTime)
	{
		TickComponent(DeltaTime, LEVELTICK_All, nullptr);
	}

	float GetFieldOfViewOffsetForTest() const
	{
		return FieldOfViewOffset;
	}
};

UCLASS()
class USigilCameraStackTestMode final : public USigilCameraMode
{
	GENERATED_BODY()

protected:
	virtual void UpdateView(float DeltaTime) override
	{
		View.ControlRotation = FRotator(10.0f, 20.0f, 30.0f);
		View.FieldOfView = 90.0f;
		View.SpringArmLength = 420.0f;
		View.SpringArmSocketOffset = FVector(11.0f, 12.0f, 13.0f);
		View.SpringArmTargetOffset = FVector(21.0f, 22.0f, 23.0f);
	}
};
