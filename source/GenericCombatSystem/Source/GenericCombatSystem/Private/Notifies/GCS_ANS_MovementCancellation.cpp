// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Notifies/GCS_ANS_MovementCancellation.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGCS_ANS_MovementCancellation::UGCS_ANS_MovementCancellation(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	bIsNativeBranchingPoint = true;
}

void UGCS_ANS_MovementCancellation::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyBegin(BranchingPointPayload);

	IsRootMotionDisabled = false;

	if (USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent)
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			if (FAnimMontageInstance* MontageInstance = AnimInstance->GetMontageInstanceForID(BranchingPointPayload.MontageInstanceID))
			{
				if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
				{
					if (Character->GetCharacterMovement()->GetCurrentAcceleration().SizeSquared2D() > 10.0)
					{
						MontageInstance->PushDisableRootMotion();

						IsRootMotionDisabled = true;
					}
				}
			}
		}
	}
}

void UGCS_ANS_MovementCancellation::BranchingPointNotifyTick(FBranchingPointNotifyPayload& BranchingPointPayload, float FrameDeltaTime)
{
	Super::BranchingPointNotifyTick(BranchingPointPayload, FrameDeltaTime);
	if (USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent)
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			if (FAnimMontageInstance* MontageInstance = AnimInstance->GetMontageInstanceForID(BranchingPointPayload.MontageInstanceID))
			{
				if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner())) {
					if (Character->GetCharacterMovement()->GetCurrentAcceleration().SizeSquared2D() > 10.0) {
						if (!IsRootMotionDisabled) {
							MontageInstance->PushDisableRootMotion();

							IsRootMotionDisabled = true;
						}
					}
					else if (IsRootMotionDisabled) {
						MontageInstance->PopDisableRootMotion();

						IsRootMotionDisabled = false;
					}
				}
			}
		}
	}
}

void UGCS_ANS_MovementCancellation::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyEnd(BranchingPointPayload);
	if (USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent)
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			if (FAnimMontageInstance* MontageInstance = AnimInstance->GetMontageInstanceForID(BranchingPointPayload.MontageInstanceID))
			{
				if (IsRootMotionDisabled) {
					IsRootMotionDisabled = false;

					MontageInstance->PopDisableRootMotion();
				}
			}
		}
	}
}

bool UGCS_ANS_MovementCancellation::IsMoving_Implementation(USkeletalMeshComponent* MeshComp) const
{
	return false;
}

#if WITH_EDITOR


bool UGCS_ANS_MovementCancellation::CanBePlaced(UAnimSequenceBase* Animation) const
{
	return (Animation && Animation->IsA(UAnimMontage::StaticClass()));
}
#endif
