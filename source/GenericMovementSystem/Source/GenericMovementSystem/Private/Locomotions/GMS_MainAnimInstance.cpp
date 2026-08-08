// Copyright 2024 RedMoonGames All Rights Reserved.

#include "Locomotions/GMS_MainAnimInstance.h"
#include "AnimationWarpingLibrary.h"
#include "DrawDebugHelpers.h"
#include "GMS_CharacterMovementSystemComponent.h"
#include "GMS_MovementSystemComponent.h"
#include "KismetAnimationLibrary.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Locomotions/GMS_AnimLayer.h"
#include "Locomotions/GMS_AnimLayer_Additive.h"
#include "Locomotions/GMS_AnimLayer_Overlay.h"
#include "Locomotions/GMS_AnimLayer_States.h"
#include "Locomotions/GMS_AnimLayer_View.h"
#include "Locomotions/GMS_AnimLayer_SkeletalControls.h"
#include "Utility/GMS_Log.h"
#include "Utility/GMS_Math.h"
#include "Utility/GMS_Utility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_MainAnimInstance)

UGMS_MainAnimInstance::UGMS_MainAnimInstance()
{
	RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
}

UGMS_MovementSystemComponent* UGMS_MainAnimInstance::GetMovementSystemComponent() const
{
	return MSC;
}


void UGMS_MainAnimInstance::RegisterStateNameToTagMapping(UAnimInstance* SourceAnimInstance, TArray<FGMS_AnimStateNameToTag> Mapping)
{
	if (SourceAnimInstance && SourceAnimInstance->Blueprint_GetMainAnimInstance() == this && !Mapping.IsEmpty())
	{
		FGMS_AnimStateNameToTagWrapper Wrapper;
		Wrapper.AnimStateNameToTagMapping = Mapping;
		RuntimeAnimStateNameToTagMappings.Emplace(SourceAnimInstance, Wrapper);
	}
}

void UGMS_MainAnimInstance::UnregisterStateNameToTagMapping(UAnimInstance* SourceAnimInstance)
{
	if (SourceAnimInstance && SourceAnimInstance->Blueprint_GetMainAnimInstance() == this && RuntimeAnimStateNameToTagMappings.Contains(SourceAnimInstance))
	{
		TArray<FGameplayTag> Tags;
		for (const FGMS_AnimStateNameToTag& Mapping : RuntimeAnimStateNameToTagMappings[SourceAnimInstance].AnimStateNameToTagMapping)
		{
			Tags.Add(Mapping.Tag);
		}
		NodeRelevanceTags.RemoveTags(FGameplayTagContainer::CreateFromArray(Tags));
		RuntimeAnimStateNameToTagMappings.Remove(SourceAnimInstance);
	}
}

#pragma region Definition


void UGMS_MainAnimInstance::RefreshLayerSettings_Implementation()
{
	const FGMS_MovementSetSetting& MSSetting = MSC->GetMovementSetSetting();

	const auto& States = MSSetting.bUseInstancedStatesSetting ? MSSetting.AnimLayerSetting_States : MSSetting.DA_AnimLayerSetting_States;

	SetAnimLayerBySetting(States, StateLayerInstance);

	const auto& Overlay = MSSetting.bUseInstancedOverlaySetting ? MSSetting.AnimLayerSetting_Overlay : MSSetting.DA_AnimLayerSetting_Overlay;

	SetAnimLayerBySetting(Overlay, OverlayLayerInstance);
	SetAnimLayerBySetting(MSSetting.AnimLayerSetting_View, ViewLayerInstance);
	SetAnimLayerBySetting(MSSetting.AnimLayerSetting_Additive, AdditiveLayerInstance);
	SetAnimLayerBySetting(MSSetting.AnimLayerSetting_SkeletalControls, SkeletonControlsLayerInstance);
}

void UGMS_MainAnimInstance::SetOffsetRootBoneRotationMode_Implementation(EOffsetRootBoneMode NewRotationMode)
{
	if (GeneralSetting.bEnableOffsetRootBoneRotation && RootState.RotationMode != NewRotationMode)
	{
		RootState.RotationMode  = NewRotationMode;
	}
}


EOffsetRootBoneMode UGMS_MainAnimInstance::GetOffsetRootBoneRotationMode_Implementation() const
{
	if (bAnyMontagePlaying) return EOffsetRootBoneMode::Release;
	return GeneralSetting.bEnableOffsetRootBoneRotation ? RootState.RotationMode : EOffsetRootBoneMode::Release;
}

void UGMS_MainAnimInstance::OnLocomotionModeChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))

	LocomotionMode = MSC->GetLocomotionMode();
	LocomotionModeContainer = LocomotionMode.GetSingleTagContainer();

	bLocomotionModeChanged = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		bLocomotionModeChanged = false;
	});
}

void UGMS_MainAnimInstance::OnRotationModeChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))
	UE_LOG(LogGMS, VeryVerbose, TEXT("Refresh layer settings due to RotationMode Changed."))

	RotationMode = MSC->GetRotationMode();
	RotationModeContainer = MSC->GetRotationMode().GetSingleTagContainer();

	RefreshLayerSettings();

	bRotationModeChanged = true;

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		bRotationModeChanged = false;
	});
}

void UGMS_MainAnimInstance::OnMovementSetChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))
	UE_LOG(LogGMS, VeryVerbose, TEXT("Refresh layer settings due to MovementSet Changed."))

	MovementSet = MSC->GetMovementSet();
	MovementSetContainer = MovementSet.GetSingleTagContainer();

	RefreshLayerSettings();

	bMovementSetChanged = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		bMovementSetChanged = false;
	});
}

void UGMS_MainAnimInstance::OnMovementStateChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))
	UE_LOG(LogGMS, VeryVerbose, TEXT("Refresh layer settings due to MovementState Changed."))

	MovementState = MSC->GetMovementState();
	MovementStateContainer = MovementState.GetSingleTagContainer();

	RefreshLayerSettings();

	bMovementStateChanged = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		bMovementStateChanged = false;
	});
}

void UGMS_MainAnimInstance::OnOverlayModeChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))
	UE_LOG(LogGMS, VeryVerbose, TEXT("Refresh layer settings due to OverlayMode Changed."))

	OverlayMode = MSC->GetOverlayMode();
	OverlayModeContainer = MSC->GetOverlayMode().GetSingleTagContainer();

	RefreshLayerSettings();

	bOverlayModeChanged = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		bOverlayModeChanged = false;
	});
}

#pragma endregion  Definition


void UGMS_MainAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PawnOwner = Cast<APawn>(GetOwningActor());

#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld() && !IsValid(PawnOwner))
	{
		// Use default objects for editor preview.

		PawnOwner = GetMutableDefault<APawn>();
		MSC = PawnOwner->FindComponentByClass<UGMS_MovementSystemComponent>();
	}
#endif
}

void UGMS_MainAnimInstance::NativeUninitializeAnimation()
{
	if (IsValid(MSC))
	{
		MSC->OnLocomotionModeChangedEvent.RemoveDynamic(this, &ThisClass::OnLocomotionModeChanged);
		MSC->OnRotationModeChangedEvent.RemoveDynamic(this, &ThisClass::OnRotationModeChanged);
		MSC->OnMovementSetChangedEvent.RemoveDynamic(this, &ThisClass::OnMovementSetChanged);
		MSC->OnMovementStateChangedEvent.RemoveDynamic(this, &ThisClass::OnMovementStateChanged);
		MSC->OnOverlayModeChangedEvent.RemoveDynamic(this, &ThisClass::OnOverlayModeChanged);
	}
	if (InitialTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InitialTimerHandle);
	}
	Super::NativeUninitializeAnimation();
}

void UGMS_MainAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	ensure(PawnOwner);

	MSC = PawnOwner->FindComponentByClass<UGMS_MovementSystemComponent>();

	ensure(MSC);

	if (IsValid(MSC))
	{
		MSC->MainAnimInstance = this;
		MSC->OnLocomotionModeChangedEvent.AddDynamic(this, &ThisClass::OnLocomotionModeChanged);
		MSC->OnRotationModeChangedEvent.AddDynamic(this, &ThisClass::OnRotationModeChanged);
		MSC->OnMovementSetChangedEvent.AddDynamic(this, &ThisClass::OnMovementSetChanged);
		MSC->OnMovementStateChangedEvent.AddDynamic(this, &ThisClass::OnMovementStateChanged);
		MSC->OnOverlayModeChangedEvent.AddDynamic(this, &ThisClass::OnOverlayModeChanged);

		//Grab latest info and intialize.
		FTimerDelegate Delegate = FTimerDelegate::CreateLambda([this]()
		{
			InitialTimerHandle.Invalidate();
			const FGMS_MovementSetSetting& MSSetting = MSC->GetMovementSetSetting();

			LocomotionMode = MSC->GetLocomotionMode();
			LocomotionModeContainer = LocomotionMode.GetSingleTagContainer();

			MovementSet = MSC->GetMovementSet();
			MovementSetContainer = MovementSet.GetSingleTagContainer();

			MovementState = MSC->GetMovementState();
			MovementStateContainer = MovementState.GetSingleTagContainer();

			RotationMode = MSC->GetRotationMode();
			RotationModeContainer = MSC->GetRotationMode().GetSingleTagContainer();

			OverlayMode = MSC->GetOverlayMode();
			OverlayModeContainer = MSC->GetOverlayMode().GetSingleTagContainer();

			RefreshLayerSettings();
		});

		GetWorld()->GetTimerManager().SetTimer(InitialTimerHandle, Delegate, 0.2f, false);
	}
	else
	{
		UE_LOG(LogGMS, Error, TEXT("Missing Movement system component on actor(%s), This anim instance(%s) will not work properly!"), *PawnOwner->GetName(), *GetClass()->GetName());
	}
}

void UGMS_MainAnimInstance::NativeUpdateAnimation(const float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGMS_MainAnimInstance::NativeUpdateAnimation()"),
	                            STAT_UGMS_MainAnimInstance_NativeUpdateAnimation, STATGROUP_GMS)

	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValid(PawnOwner) || !IsValid(MSC))
	{
		return;
	}

	if (UGMS_CharacterMovementSystemComponent* CharacterMovementSystemComponent = Cast<UGMS_CharacterMovementSystemComponent>(MSC))
	{
		if (!IsValid(CharacterMovementSystemComponent->GetCharacterMovement()))
			return;
	}

	LocomotionMode = MSC->GetLocomotionMode();
	LocomotionModeContainer = LocomotionMode.GetSingleTagContainer();

	MovementSet = MSC->GetMovementSet();
	MovementSetContainer = MovementSet.GetSingleTagContainer();

	MovementState = MSC->GetMovementState();
	MovementStateContainer = MovementState.GetSingleTagContainer();

	RotationMode = MSC->GetRotationMode();
	RotationModeContainer = MSC->GetRotationMode().GetSingleTagContainer();

	OverlayMode = MSC->GetOverlayMode();
	OverlayModeContainer = MSC->GetOverlayMode().GetSingleTagContainer();

	OwnedTags = MSC->GetGameplayTags();

	GeneralSetting = MSC->GetMovementSetSetting().AnimDataSetting_General;

	RefreshViewOnGameThread();
	RefreshLocomotionOnGameThread();
	RefreshRelevanceOnGameThread();
	
	bAnyMontagePlaying = IsAnyMontagePlaying();
}

void UGMS_MainAnimInstance::NativeThreadSafeUpdateAnimation(const float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGMS_MainAnimInstance::NativeThreadSafeUpdateAnimation()"),
	                            STAT_UGMS_MainAnimInstance_NativeThreadSafeUpdateAnimation, STATGROUP_GMS)

	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	if (!IsValid(PawnOwner) || !IsValid(MSC))
	{
		return;
	}

	RefreshLocomotion(DeltaTime);
	RefreshGrounded();
	RefreshInAir();
	RefreshView(DeltaTime);
}

void UGMS_MainAnimInstance::SetAnimLayerBySetting(const UGMS_AnimLayerSetting* LayerSetting, TObjectPtr<UGMS_AnimLayer>& LayerInstance)
{
	check(IsInGameThread() && IsValid(MSC) && IsValid(MSC->AnimGraphSetting))

	//invalid setting
	if (!IsValid(LayerSetting))
	{
		if (IsValid(LayerInstance))
		{
			UnlinkAnimClassLayers(LayerInstance->GetClass());
			LayerInstance->OnUnlinked();
			LayerInstance = nullptr;
		}
		return;
	}

	TSubclassOf<UGMS_AnimLayer> LayerClass = nullptr;
	if (!LayerSetting->GetOverrideAnimLayerClass(LayerClass))
	{
		bool bValidMapping = MSC->AnimGraphSetting->AnimLayerSettingToInstanceMapping.Contains(LayerSetting->GetClass()) && MSC->AnimGraphSetting->AnimLayerSettingToInstanceMapping[LayerSetting->
			GetClass()] != nullptr;

		if (!bValidMapping)
		{
			UE_LOG(LogGMS, Error, TEXT("Can't find exising anim instance mapping for anim layer setting(%s) or mapped a invalid anim instance. Please check anim graph setting:%s %S"),
			       *LayerSetting->GetClass()->GetName(), *MSC->AnimGraphSetting->GetName(), __FUNCTION__)
			if (IsValid(LayerInstance))
			{
				UnlinkAnimClassLayers(LayerInstance->GetClass());
				LayerInstance->OnUnlinked();
				LayerInstance = nullptr;
			}
			return;
		}

		LayerClass = MSC->AnimGraphSetting->AnimLayerSettingToInstanceMapping[LayerSetting->GetClass()];
	}

	if (IsValid(LayerInstance) && LayerClass != LayerInstance->GetClass())
	{
		UnlinkAnimClassLayers(LayerInstance->GetClass());
		LayerInstance->OnUnlinked();
		LayerInstance = nullptr;
	}

	if (!IsValid(LayerInstance))
	{
		LinkAnimClassLayers(LayerClass);
		LayerInstance = Cast<UGMS_AnimLayer>(GetLinkedAnimLayerInstanceByClass(LayerClass));
		if (LayerInstance)
		{
			LayerInstance->OnLinked();
		}
		else
		{
			UE_LOG(LogGMS, Error, TEXT("Failed to link anim layer by class(%s), It will happen if this class doesn't implement any anim layer interface required on main anim instance. "),
			       *LayerClass->GetName())
		}
	}

	if (LayerInstance)
	{
		LayerInstance->ApplySetting(LayerSetting);
	}
}


void UGMS_MainAnimInstance::RefreshViewOnGameThread()
{
	check(IsInGameThread())

	const auto& View{MSC->GetViewState()};

	ViewState.Rotation = View.Rotation;
	ViewState.YawSpeed = View.YawSpeed;
}

void UGMS_MainAnimInstance::RefreshView(const float DeltaTime)
{
	// FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(ViewState.Rotation,RootState.RootTransform.Rotator());
	// ViewState.YawAngle = RotationDelta.Yaw;
	// ViewState.PitchAngle = RotationDelta.Pitch;
	ViewState.YawAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw - RootState.YawOffset));
	ViewState.PitchAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Pitch - LocomotionState.Rotation.Pitch));

	ViewState.PitchAmount = 0.5f - ViewState.PitchAngle / 180.0f;
}

void UGMS_MainAnimInstance::RefreshLocomotionOnGameThread()
{
	check(IsInGameThread())

	GameThreadState = MSC->GetLocomotionState();
	InputDirection = MSC->GetInputDirection();

	LocomotionState.MaxAcceleration = MSC->GetMaxAcceleration();
	LocomotionState.MaxBrakingDeceleration = MSC->GetMaxBrakingDeceleration();
	LocomotionState.WalkableFloorZ = MSC->GetWalkableFloorZ();

	LocomotionState.Scale = UE_REAL_TO_FLOAT(GetSkelMeshComponent()->GetComponentScale().Z);

	LocomotionState.CapsuleRadius = MSC->GetScaledCapsuleRadius();
	LocomotionState.CapsuleHalfHeight = MSC->GetScaledCapsuleHalfHeight();
}

void UGMS_MainAnimInstance::RefreshLocomotion(const float DeltaTime)
{
	// update location data
	LocomotionState.PreviousDisplacement = (GetOwningActor()->GetActorLocation() - LocomotionState.Location).Size2D();
	LocomotionState.Location = GetOwningActor()->GetActorLocation();
	LocomotionState.DisplacementSpeed = UKismetMathLibrary::SafeDivide(LocomotionState.PreviousDisplacement, DeltaTime);

	if (bFirstUpdate)
	{
		LocomotionState.PreviousDisplacement = 0.0f;
		LocomotionState.DisplacementSpeed = 0.0f;
	}

	// update rotation data
	LocomotionState.Rotation = GameThreadState.Rotation;
	LocomotionState.RotationQuaternion = GameThreadState.RotationQuaternion;

	// update velocity data

	LocomotionState.bHasInput = GameThreadState.bHasInput;

	LocomotionState.Speed = GameThreadState.Speed;
	LocomotionState.Velocity = GameThreadState.Velocity;

	LocomotionState.TargetYawAngle = GameThreadState.TargetYawAngle;

	bool bWasMovingLastUpdate = !LocomotionState.LocalVelocity2D.IsZero();

	LocomotionState.LocalVelocity2D = GameThreadState.RotationQuaternion.UnrotateVector({LocomotionState.Velocity.X, LocomotionState.Velocity.Y, 0.0f});

	LocomotionState.bHasVelocity = !FMath::IsNearlyZero(LocomotionState.LocalVelocity2D.SizeSquared2D());

	// auto Dir = bBlocked ? InputDirection.GetSafeNormal2D() : LocomotionState.Velocity.GetSafeNormal2D();

	LocomotionState.LocalVelocityYawAngle = UKismetAnimationLibrary::CalculateDirection(LocomotionState.Velocity.GetSafeNormal2D(), LocomotionState.Rotation);

	LocomotionState.LocalVelocityYawAngleWithOffset = LocomotionState.LocalVelocityYawAngle - RootState.YawOffset;

	//take root yaw offset in account. 考虑到Offset的方向
	LocomotionState.LocalVelocityDirection = SelectCardinalDirectionFromAngle(LocomotionState.LocalVelocityYawAngleWithOffset, 10, LocomotionState.LocalVelocityDirection,
	                                                                          bWasMovingLastUpdate);

	LocomotionState.LocalVelocityDirectionNoOffset = SelectCardinalDirectionFromAngle(LocomotionState.LocalVelocityYawAngle, 10, LocomotionState.LocalVelocityDirectionNoOffset,
	                                                                                  bWasMovingLastUpdate);

	LocomotionState.LocalVelocityOctagonalDirection = SelectOctagonalDirectionFromAngle(LocomotionState.LocalVelocityYawAngleWithOffset, 10, LocomotionState.LocalVelocityOctagonalDirection,
	                                                                                    bWasMovingLastUpdate);

	LocomotionState.VelocityAcceleration = (GameThreadState.Velocity - GameThreadState.PreviousVelocity) / DeltaTime;

	LocomotionState.LocalAcceleration2D = UKismetMathLibrary::LessLess_VectorRotator({InputDirection.X, InputDirection.Y, 0.0f}, LocomotionState.Rotation);
	// LocomotionState.LocalAcceleration2D = UKismetMathLibrary::LessLess_VectorRotator({GameThreadState.CurrentAcceleration.X, GameThreadState.CurrentAcceleration.Y, 0.0f}, LocomotionState.Rotation);

	LocomotionState.bMoving = GameThreadState.bMoving;

	bFirstUpdate = false;
}

void UGMS_MainAnimInstance::RefreshBlock()
{
	bBlocked = UKismetMathLibrary::VSizeXY(InputDirection) > 0.1 && LocomotionState.Speed < 200.0f &&
		UKismetMathLibrary::InRange_FloatFloat(FVector::DotProduct(InputDirection.GetSafeNormal(0.0001), LocomotionState.Velocity.GetSafeNormal(0.0001)), -0.6f, 0.6, true, true);
}

void UGMS_MainAnimInstance::RefreshRelevanceOnGameThread()
{
	if (!IsValid(this))
	{
		return;
	}

	FGameplayTagContainer TagsToAdd;

	for (int i = 0; i < AnimStateNameToTagMapping.Num(); ++i)
	{
		if (AnimStateNameToTagMapping[i].State.IsRelevant(*this))
		{
			TagsToAdd.AddTag(AnimStateNameToTagMapping[i].Tag);
		}
	}

	for (const TTuple<TObjectPtr<UAnimInstance>, FGMS_AnimStateNameToTagWrapper>& Pair : RuntimeAnimStateNameToTagMappings)
	{
		if (IsValid(Pair.Key))
		{
			for (int i = 0; i < Pair.Value.AnimStateNameToTagMapping.Num(); ++i)
			{
				if (Pair.Value.AnimStateNameToTagMapping[i].State.IsRelevant(*Pair.Key))
				{
					TagsToAdd.AddTag(Pair.Value.AnimStateNameToTagMapping[i].Tag);
				}
			}
		}
	}

	NodeRelevanceTags = TagsToAdd;
}


void UGMS_MainAnimInstance::RefreshGrounded()
{
#if WITH_EDITOR
	if (!IsValid(GetWorld()) || !GetWorld()->IsGameWorld())
	{
		return;
	}
#endif
	if (LocomotionMode != GMS_MovementModeTags::Grounded)
	{
		return;
	}

	RefreshBlock();
	RefreshGroundedLean();
}

void UGMS_MainAnimInstance::RefreshGroundedLean()
{
	const auto TargetLeanAmount{GetRelativeAccelerationAmount()};

	const auto DeltaTime{GetDeltaSeconds()};

	LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, TargetLeanAmount.Y,
	                                         DeltaTime, GeneralSetting.LeanInterpolationSpeed);

	LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, TargetLeanAmount.X,
	                                           DeltaTime, GeneralSetting.LeanInterpolationSpeed);
}

FVector2f UGMS_MainAnimInstance::GetRelativeAccelerationAmount() const
{
	// This value represents the current amount of acceleration / deceleration relative to the
	// character rotation. It is normalized to a range of -1 to 1 so that -1 equals the max
	// braking deceleration and 1 equals the max acceleration of the character movement component.

	const auto MaxAcceleration{
		(InputDirection | LocomotionState.Velocity) >= 0.0f
			? LocomotionState.MaxAcceleration
			: LocomotionState.MaxBrakingDeceleration
	};

	// const FVector3f RelativeAcceleration{
	// 	LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.VelocityAcceleration)
	// };

	// relative to root bone transform.
	const FVector3f RelativeAcceleration{
		RootState.RootTransform.GetRotation().UnrotateVector(LocomotionState.VelocityAcceleration)
	};


	return FVector2f{UGMS_Math::ClampMagnitude01(RelativeAcceleration / MaxAcceleration)};
}


void UGMS_MainAnimInstance::RefreshInAir()
{
#if WITH_EDITOR
	if (!IsValid(GetWorld()) || !GetWorld()->IsGameWorld())
	{
		return;
	}
#endif

	InAirState.bJumping = false;
	InAirState.bFalling = false;

	if (LocomotionMode != GMS_MovementModeTags::InAir)
	{
		// InAirState.VerticalSpeed = 0.0f;
		return;
	}

	// A separate variable for vertical speed is used to determine at what speed the character landed on the ground.

	if (LocomotionState.Velocity.Z > 0)
	{
		InAirState.bJumping = true;
		InAirState.TimeToJumpApex = (0 - LocomotionState.Velocity.Z) / MSC->GetGravityZ();
		InAirState.FallingTime = 0;
	}
	else
	{
		InAirState.bFalling = true;
		InAirState.TimeToJumpApex = 0;
		InAirState.FallingTime += GetDeltaSeconds();
	}

	InAirState.VerticalSpeed = UE_REAL_TO_FLOAT(LocomotionState.Velocity.Z);

	RefreshGroundPrediction();

	RefreshInAirLean();
}

void UGMS_MainAnimInstance::RefreshGroundPrediction()
{
	if (!bEnableGroundPrediction)
	{
		return;
	}

	static constexpr auto VerticalVelocityThreshold{-200.0f};

	if (InAirState.VerticalSpeed > VerticalVelocityThreshold)
	{
		InAirState.bValidGround = false;
		InAirState.GroundDistance = -1.0f;
		return;
	}

	const auto SweepStartLocation{LocomotionState.Location};

	static constexpr auto MinVerticalVelocity{-4000.0f};
	static constexpr auto MaxVerticalVelocity{-200.0f};

	auto VelocityDirection{LocomotionState.Velocity};
	VelocityDirection.Z = FMath::Clamp(VelocityDirection.Z, MinVerticalVelocity, MaxVerticalVelocity);
	VelocityDirection.Normalize();

	static constexpr auto MinSweepDistance{150.0f};
	static constexpr auto MaxSweepDistance{2000.0f};

	const auto SweepVector{
		VelocityDirection * FMath::GetMappedRangeValueClamped(FVector2f{MaxVerticalVelocity, MinVerticalVelocity},
		                                                      {MinSweepDistance, MaxSweepDistance},
		                                                      InAirState.VerticalSpeed) * LocomotionState.Scale
	};

	FHitResult Hit;
	GetWorld()->SweepSingleByChannel(Hit, SweepStartLocation, SweepStartLocation + SweepVector,
	                                 FQuat::Identity, GeneralSetting.GroundPredictionSweepChannel,
	                                 FCollisionShape::MakeCapsule(LocomotionState.CapsuleRadius, LocomotionState.CapsuleHalfHeight),
	                                 {__FUNCTION__, false, PawnOwner}, GeneralSetting.GroundPredictionSweepResponses);

	const auto bGroundValid{Hit.IsValidBlockingHit() && Hit.ImpactNormal.Z >= LocomotionState.WalkableFloorZ};

	InAirState.bValidGround = bGroundValid;
	InAirState.GroundDistance = Hit.Distance;
}

void UGMS_MainAnimInstance::RefreshInAirLean()
{
	if (GeneralSetting.InAirLeanAmountCurve == nullptr)
		return;

	// Use the relative velocity direction and amount to determine how much the character should lean
	// while in air. The lean amount curve gets the vertical velocity and is used as a multiplier to
	// smoothly reverse the leaning direction when transitioning from moving upwards to moving downwards.

	static constexpr auto ReferenceSpeed{350.0f};

	const auto RelativeVelocity{
		FVector3f{LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Velocity)} /
		ReferenceSpeed * GeneralSetting.InAirLeanAmountCurve->GetFloatValue(InAirState.VerticalSpeed)
	};

	const auto DeltaTime{GetDeltaSeconds()};

	LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, RelativeVelocity.Y,
	                                         DeltaTime, GeneralSetting.LeanInterpolationSpeed);

	LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, RelativeVelocity.X,
	                                           DeltaTime, GeneralSetting.LeanInterpolationSpeed);
}

void UGMS_MainAnimInstance::RefreshOffsetRootBone_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	if (GetMovementSystemComponent())
	{
		RootState.Transform = UAnimationWarpingLibrary::GetOffsetRootTransform(Node);
		// if (!GeneralSetting.bEnableOffsetRootBoneRotation || GetOffsetRootBoneRotationMode() == EOffsetRootBoneMode::Release)
		// {
		// 	RootState.RootTransform.SetLocation(LocomotionState.Location);
		// 	RootState.RootTransform.SetRotation(LocomotionState.RotationQuaternion);
		//
		// 	// Smooth blendout to 0.
		// 	// RootState.YawOffset = UKismetMathLibrary::FloatSpringInterp(RootState.YawOffset,0,RootState.SprintState,80,1.0,GetDeltaSeconds(),1,0.5,false);
		// 	// RootState.YawOffset = UKismetMathLibrary::FInterpTo(RootState.YawOffset,0,Context.GetContext()->GetDeltaTime(),50);
		// 	RootState.YawOffset = 0.0f;
		// }
		// else
		// {
		// 	FRotator Rotator = FRotator(RootState.Transform.Rotator().Pitch, RootState.Transform.Rotator().Yaw + 90.0f, RootState.Transform.Rotator().Roll);
		// 	RootState.RootTransform = UKismetMathLibrary::MakeTransform(RootState.Transform.GetTranslation(), Rotator, FVector::One());
		// 	RootState.YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(Rotator, LocomotionState.Rotation).Yaw;
		// }

		FRotator Rotator = FRotator(RootState.Transform.Rotator().Pitch, RootState.Transform.Rotator().Yaw + 90.0f, RootState.Transform.Rotator().Roll);
		RootState.RootTransform = UKismetMathLibrary::MakeTransform(RootState.Transform.GetTranslation(), Rotator, FVector::One());
		RootState.YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(Rotator, LocomotionState.Rotation).Yaw;

		if (RotationMode == GMS_RotationModeTags::ViewDirection)
		{
			//apply constraint when aiming.
			const FGMS_ViewDirectionSetting& ViewDirSetting = GetMovementSystemComponent()->MovementStateSetting.ViewDirectionSetting;
			if (ViewDirSetting.DirectionMode == EGMS_ViewDirectionMode::Aiming)
			{
				if (FMath::Abs(RootState.YawOffset) <= ViewDirSetting.MinAimingYawAngleLimit + UE_KINDA_SMALL_NUMBER)
				{
					RootState.MaxRotationError = ViewDirSetting.MinAimingYawAngleLimit;
				}
			}
			else
			{
				// no limit.	
				RootState.MaxRotationError = -1.0f;
			}
		}
		else
		{
			// no limit.	
			RootState.MaxRotationError = -1.0f;
		}
	}
}

float UGMS_MainAnimInstance::GetCurveValueClamped01(const FName& CurveName) const
{
	return UGMS_Math::Clamp01(GetCurveValue(CurveName));
}

UBlendProfile* UGMS_MainAnimInstance::GetNamedBlendProfile(const FName& BlendProfileName) const
{
	if (CurrentSkeleton)
	{
		return CurrentSkeleton->GetBlendProfile(BlendProfileName);
	}
	return nullptr;
}

EGMS_MovementDirection UGMS_MainAnimInstance::SelectCardinalDirectionFromAngle(float Angle, float DeadZone, EGMS_MovementDirection CurrentDirection, bool bUseCurrentDirection) const
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

EGMS_MovementDirection_8Way UGMS_MainAnimInstance::SelectOctagonalDirectionFromAngle(float Angle, float DeadZone, EGMS_MovementDirection_8Way CurrentDirection,
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

EGMS_MovementDirection UGMS_MainAnimInstance::GetOppositeCardinalDirection(EGMS_MovementDirection CurrentDirection) const
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

bool UGMS_MainAnimInstance::HasCoreStateChanges() const
{
	return bMovementSetChanged || bMovementStateChanged || bLocomotionModeChanged || bOverlayModeChanged || bRotationModeChanged;
}

bool UGMS_MainAnimInstance::CheckCoreStateChanges(bool bCheckLocomotionMode, bool bCheckMovementSet, bool bCheckRotationMode, bool bCheckMovementState, bool bCheckOverlayMode) const
{
	return (bCheckLocomotionMode && bLocomotionModeChanged) ||
		(bCheckMovementSet && bMovementSetChanged) ||
		(bCheckRotationMode && bRotationModeChanged) ||
		(bCheckMovementState && bMovementStateChanged) ||
		(bCheckOverlayMode && bOverlayModeChanged);
}
