// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "Locomotions/SigilLocomotionStructLibrary.h"
#include "SigilSecondaryAnimInstance.generated.h"

class APawn;
class USigilMovementSystemComponent;

/**
 * 供需要 SigilMovement 快照的次级 Mesh 使用的只读动画实例。
 */
UCLASS(BlueprintType)
class SIGILMOVEMENT_API USigilSecondaryAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUninitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FGameplayTag MovementSet;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FGameplayTag MovementState;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FGameplayTag LocomotionMode;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FGameplayTag RotationMode;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FGameplayTag OverlayMode;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FVector InputDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FGameplayTagContainer OwnedTags;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FSigilLocomotionState LocomotionState;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Sigil|Movement")
	FSigilViewState ViewState;

private:
	void RefreshSnapshotOnGameThread();
	void ResetSnapshot();
	void ReportMissingDependencyOnce(const TCHAR* DependencyName);

	TWeakObjectPtr<APawn> PawnOwner;
	TWeakObjectPtr<USigilMovementSystemComponent> MovementSystemComponent;
	bool bReportedMissingDependency{false};
};
