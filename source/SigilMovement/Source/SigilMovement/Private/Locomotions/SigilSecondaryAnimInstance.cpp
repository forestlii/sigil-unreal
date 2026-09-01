// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Locomotions/SigilSecondaryAnimInstance.h"

#include "GameFramework/Pawn.h"
#include "SigilMovementSystemComponent.h"
#include "Utility/SigilLog.h"

void USigilSecondaryAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	RefreshSnapshotOnGameThread();
}

void USigilSecondaryAnimInstance::NativeUninitializeAnimation()
{
	PawnOwner.Reset();
	MovementSystemComponent.Reset();
	ResetSnapshot();
	Super::NativeUninitializeAnimation();
}

void USigilSecondaryAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	RefreshSnapshotOnGameThread();
}

void USigilSecondaryAnimInstance::NativeThreadSafeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}

void USigilSecondaryAnimInstance::RefreshSnapshotOnGameThread()
{
	check(IsInGameThread());

	if (!PawnOwner.IsValid())
	{
		PawnOwner = Cast<APawn>(GetOwningActor());
	}
	if (!PawnOwner.IsValid())
	{
		ResetSnapshot();
		ReportMissingDependencyOnce(TEXT("Pawn owner"));
		return;
	}

	if (!MovementSystemComponent.IsValid())
	{
		MovementSystemComponent = PawnOwner->FindComponentByClass<USigilMovementSystemComponent>();
	}
	if (!MovementSystemComponent.IsValid())
	{
		ResetSnapshot();
		ReportMissingDependencyOnce(TEXT("Sigil movement component"));
		return;
	}

	MovementSet = MovementSystemComponent->GetMovementSet();
	MovementState = MovementSystemComponent->GetMovementState();
	LocomotionMode = MovementSystemComponent->GetLocomotionMode();
	RotationMode = MovementSystemComponent->GetRotationMode();
	OverlayMode = MovementSystemComponent->GetOverlayMode();
	InputDirection = MovementSystemComponent->GetInputDirection();
	OwnedTags = MovementSystemComponent->GetGameplayTags();
	LocomotionState = MovementSystemComponent->GetLocomotionState();
	ViewState = MovementSystemComponent->GetViewState();
}

void USigilSecondaryAnimInstance::ResetSnapshot()
{
	MovementSet = FGameplayTag::EmptyTag;
	MovementState = FGameplayTag::EmptyTag;
	LocomotionMode = FGameplayTag::EmptyTag;
	RotationMode = FGameplayTag::EmptyTag;
	OverlayMode = FGameplayTag::EmptyTag;
	InputDirection = FVector::ZeroVector;
	OwnedTags.Reset();
	LocomotionState = FSigilLocomotionState{};
	ViewState = FSigilViewState{};
}

void USigilSecondaryAnimInstance::ReportMissingDependencyOnce(const TCHAR* DependencyName)
{
	if (bReportedMissingDependency)
	{
		return;
	}

	bReportedMissingDependency = true;
	UE_LOG(
		LogSigilMovement,
		Warning,
		TEXT("Secondary anim instance '%s' has no %s; using an empty movement snapshot."),
		*GetName(),
		DependencyName);
}
