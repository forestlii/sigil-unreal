// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Locomotions/SigilMainAnimInstance.h"
#include "AnimationWarpingLibrary.h"
#include "DrawDebugHelpers.h"
#include "SigilCharacterMovementSystemComponent.h"
#include "SigilMovementSystemComponent.h"
#include "KismetAnimationLibrary.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Locomotions/SigilAnimLayer.h"
#include "Locomotions/SigilAnimLayer_Additive.h"
#include "Locomotions/SigilAnimLayer_Overlay.h"
#include "Locomotions/SigilAnimLayer_States.h"
#include "Locomotions/SigilAnimLayer_View.h"
#include "Locomotions/SigilAnimLayer_SkeletalControls.h"
#include "Utility/SigilLog.h"
#include "Utility/SigilMath.h"
#include "Utility/SigilUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilMainAnimInstance)

USigilMainAnimInstance::USigilMainAnimInstance()
{
	RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
}

USigilMovementSystemComponent* USigilMainAnimInstance::GetMovementSystemComponent() const
{
	return MSC;
}


void USigilMainAnimInstance::RegisterStateNameToTagMapping(UAnimInstance* SourceAnimInstance, TArray<FSigilAnimStateNameToTag> Mapping)
{
	if (SourceAnimInstance && SourceAnimInstance->Blueprint_GetMainAnimInstance() == this && !Mapping.IsEmpty())
	{
		FSigilAnimStateNameToTagWrapper Wrapper;
		Wrapper.AnimStateNameToTagMapping = Mapping;
		RuntimeAnimStateNameToTagMappings.Emplace(SourceAnimInstance, Wrapper);
	}
}

void USigilMainAnimInstance::UnregisterStateNameToTagMapping(UAnimInstance* SourceAnimInstance)
{
	if (SourceAnimInstance && SourceAnimInstance->Blueprint_GetMainAnimInstance() == this && RuntimeAnimStateNameToTagMappings.Contains(SourceAnimInstance))
	{
		TArray<FGameplayTag> Tags;
		for (const FSigilAnimStateNameToTag& Mapping : RuntimeAnimStateNameToTagMappings[SourceAnimInstance].AnimStateNameToTagMapping)
		{
			Tags.Add(Mapping.Tag);
		}
		NodeRelevanceTags.RemoveTags(FGameplayTagContainer::CreateFromArray(Tags));
		RuntimeAnimStateNameToTagMappings.Remove(SourceAnimInstance);
	}
}

#pragma region Definition


void USigilMainAnimInstance::RefreshLayerSettings_Implementation()
{
	const FSigilMovementSetSetting& MSSetting = MSC->GetMovementSetSetting();

	const auto& States = MSSetting.bUseInstancedStatesSetting ? MSSetting.AnimLayerSetting_States : MSSetting.DA_AnimLayerSetting_States;

	SetAnimLayerBySetting(States, StateLayerInstance);

	const auto& Overlay = MSSetting.bUseInstancedOverlaySetting ? MSSetting.AnimLayerSetting_Overlay : MSSetting.DA_AnimLayerSetting_Overlay;

	SetAnimLayerBySetting(Overlay, OverlayLayerInstance);
	SetAnimLayerBySetting(MSSetting.AnimLayerSetting_View, ViewLayerInstance);
	SetAnimLayerBySetting(MSSetting.AnimLayerSetting_Additive, AdditiveLayerInstance);
	SetAnimLayerBySetting(MSSetting.AnimLayerSetting_SkeletalControls, SkeletonControlsLayerInstance);
}

void USigilMainAnimInstance::SetOffsetRootBoneRotationMode_Implementation(EOffsetRootBoneMode NewRotationMode)
{
	if (GeneralSetting.bEnableOffsetRootBoneRotation && RootState.RotationMode != NewRotationMode)
	{
		RootState.RotationMode  = NewRotationMode;
	}
}


EOffsetRootBoneMode USigilMainAnimInstance::GetOffsetRootBoneRotationMode_Implementation() const
{
	if (bAnyMontagePlaying) return EOffsetRootBoneMode::Release;
	return GeneralSetting.bEnableOffsetRootBoneRotation ? RootState.RotationMode : EOffsetRootBoneMode::Release;
}

void USigilMainAnimInstance::OnLocomotionModeChanged_Implementation(const FGameplayTag& Prev)
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

void USigilMainAnimInstance::OnRotationModeChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))
	UE_LOG(LogSigilMovement, VeryVerbose, TEXT("Refresh layer settings due to RotationMode Changed."))

	RotationMode = MSC->GetRotationMode();
	RotationModeContainer = MSC->GetRotationMode().GetSingleTagContainer();

	RefreshLayerSettings();

	bRotationModeChanged = true;

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		bRotationModeChanged = false;
	});
}

void USigilMainAnimInstance::OnMovementSetChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))
	UE_LOG(LogSigilMovement, VeryVerbose, TEXT("Refresh layer settings due to MovementSet Changed."))

	MovementSet = MSC->GetMovementSet();
	MovementSetContainer = MovementSet.GetSingleTagContainer();

	RefreshLayerSettings();

	bMovementSetChanged = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		bMovementSetChanged = false;
	});
}

void USigilMainAnimInstance::OnMovementStateChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))
	UE_LOG(LogSigilMovement, VeryVerbose, TEXT("Refresh layer settings due to MovementState Changed."))

	MovementState = MSC->GetMovementState();
	MovementStateContainer = MovementState.GetSingleTagContainer();

	RefreshLayerSettings();

	bMovementStateChanged = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		bMovementStateChanged = false;
	});
}

void USigilMainAnimInstance::OnOverlayModeChanged_Implementation(const FGameplayTag& Prev)
{
	check(IsInGameThread())
	check(IsValid(MSC))
	UE_LOG(LogSigilMovement, VeryVerbose, TEXT("Refresh layer settings due to OverlayMode Changed."))

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


void USigilMainAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PawnOwner = Cast<APawn>(GetOwningActor());

#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld() && !IsValid(PawnOwner))
	{
		// Use default objects for editor preview.

		PawnOwner = GetMutableDefault<APawn>();
		MSC = PawnOwner->FindComponentByClass<USigilMovementSystemComponent>();
	}
#endif
}

void USigilMainAnimInstance::NativeUninitializeAnimation()
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

void USigilMainAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	ensure(PawnOwner);

	MSC = PawnOwner->FindComponentByClass<USigilMovementSystemComponent>();

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
			const FSigilMovementSetSetting& MSSetting = MSC->GetMovementSetSetting();

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
		UE_LOG(LogSigilMovement, Error, TEXT("Missing Movement system component on actor(%s), This anim instance(%s) will not work properly!"), *PawnOwner->GetName(), *GetClass()->GetName());
	}
}

void USigilMainAnimInstance::NativeUpdateAnimation(const float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("USigilMainAnimInstance::NativeUpdateAnimation()"),
	                            STAT_USigilMainAnimInstance_NativeUpdateAnimation, STATGROUP_GMS)

	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValid(PawnOwner) || !IsValid(MSC))
	{
		return;
	}

	if (USigilCharacterMovementSystemComponent* CharacterMovementSystemComponent = Cast<USigilCharacterMovementSystemComponent>(MSC))
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

void USigilMainAnimInstance::NativeThreadSafeUpdateAnimation(const float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("USigilMainAnimInstance::NativeThreadSafeUpdateAnimation()"),
	                            STAT_USigilMainAnimInstance_NativeThreadSafeUpdateAnimation, STATGROUP_GMS)

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

void USigilMainAnimInstance::SetAnimLayerBySetting(const USigilAnimLayerSetting* LayerSetting, TObjectPtr<USigilAnimLayer>& LayerInstance)
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

	TSubclassOf<USigilAnimLayer> LayerClass = nullptr;
	if (!LayerSetting->GetOverrideAnimLayerClass(LayerClass))
	{
		bool bValidMapping = MSC->AnimGraphSetting->AnimLayerSettingToInstanceMapping.Contains(LayerSetting->GetClass()) && MSC->AnimGraphSetting->AnimLayerSettingToInstanceMapping[LayerSetting->
			GetClass()] != nullptr;

		if (!bValidMapping)
		{
			UE_LOG(LogSigilMovement, Error, TEXT("Can't find exising anim instance mapping for anim layer setting(%s) or mapped a invalid anim instance. Please check anim graph setting:%s %S"),
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
		LayerInstance = Cast<USigilAnimLayer>(GetLinkedAnimLayerInstanceByClass(LayerClass));
		if (LayerInstance)
		{
			LayerInstance->OnLinked();
		}
		else
		{
			UE_LOG(LogSigilMovement, Error, TEXT("Failed to link anim layer by class(%s), It will happen if this class doesn't implement any anim layer interface required on main anim instance. "),
			       *LayerClass->GetName())
		}
	}

	if (LayerInstance)
	{
		LayerInstance->ApplySetting(LayerSetting);
	}
}


void USigilMainAnimInstance::RefreshViewOnGameThread()
{
	check(IsInGameThread())

	const auto& View{MSC->GetViewState()};

	ViewState.Rotation = View.Rotation;
	ViewState.YawSpeed = View.YawSpeed;
}

void USigilMainAnimInstance::RefreshView(const float DeltaTime)
{
	// FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(ViewState.Rotation,RootState.RootTransform.Rotator());
	// ViewState.YawAngle = RotationDelta.Yaw;
	// ViewState.PitchAngle = RotationDelta.Pitch;
	ViewState.YawAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw - RootState.YawOffset));
	ViewState.PitchAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Pitch - LocomotionState.Rotation.Pitch));

	ViewState.PitchAmount = 0.5f - ViewState.PitchAngle / 180.0f;
}

void USigilMainAnimInstance::RefreshLocomotionOnGameThread()
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

void USigilMainAnimInstance::RefreshLocomotion(const float DeltaTime)
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

void USigilMainAnimInstance::RefreshBlock()
{
	bBlocked = UKismetMathLibrary::VSizeXY(InputDirection) > 0.1 && LocomotionState.Speed < 200.0f &&
		UKismetMathLibrary::InRange_FloatFloat(FVector::DotProduct(InputDirection.GetSafeNormal(0.0001), LocomotionState.Velocity.GetSafeNormal(0.0001)), -0.6f, 0.6, true, true);
}

void USigilMainAnimInstance::RefreshRelevanceOnGameThread()
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

	for (const TTuple<TObjectPtr<UAnimInstance>, FSigilAnimStateNameToTagWrapper>& Pair : RuntimeAnimStateNameToTagMappings)
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


void USigilMainAnimInstance::RefreshGrounded()
{
#if WITH_EDITOR
	if (!IsValid(GetWorld()) || !GetWorld()->IsGameWorld())
	{
		return;
	}
#endif
	if (LocomotionMode != SigilMovementModeTags::Grounded)
	{
		return;
	}

	RefreshBlock();
	RefreshGroundedLean();
}

void USigilMainAnimInstance::RefreshGroundedLean()
{
	const auto TargetLeanAmount{GetRelativeAccelerationAmount()};

	const auto DeltaTime{GetDeltaSeconds()};

	LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, TargetLeanAmount.Y,
	                                         DeltaTime, GeneralSetting.LeanInterpolationSpeed);

	LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, TargetLeanAmount.X,
	                                           DeltaTime, GeneralSetting.LeanInterpolationSpeed);
}

FVector2f USigilMainAnimInstance::GetRelativeAccelerationAmount() const
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


	return FVector2f{USigilMath::ClampMagnitude01(RelativeAcceleration / MaxAcceleration)};
}


void USigilMainAnimInstance::RefreshInAir()
{
#if WITH_EDITOR
	if (!IsValid(GetWorld()) || !GetWorld()->IsGameWorld())
	{
		return;
	}
#endif

	InAirState.bJumping = false;
	InAirState.bFalling = false;

	if (LocomotionMode != SigilMovementModeTags::InAir)
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

void USigilMainAnimInstance::RefreshGroundPrediction()
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

void USigilMainAnimInstance::RefreshInAirLean()
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

void USigilMainAnimInstance::RefreshOffsetRootBone_Implementation(FAnimUpdateContext& Context, FAnimNodeReference& Node)
{
	if (GetMovementSystemComponent())
	{
		RootState.Transform = UAnimationWarpingLibrary::GetOffsetRootTransform(Node);

		FRotator Rotator = FRotator(RootState.Transform.Rotator().Pitch, RootState.Transform.Rotator().Yaw + 90.0f, RootState.Transform.Rotator().Roll);
		RootState.RootTransform = UKismetMathLibrary::MakeTransform(RootState.Transform.GetTranslation(), Rotator, FVector::One());
		RootState.YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(Rotator, LocomotionState.Rotation).Yaw;

		if (RotationMode == SigilRotationModeTags::ViewDirection)
		{
			//apply constraint when aiming.
			const FSigilViewDirectionSetting& ViewDirSetting = GetMovementSystemComponent()->MovementStateSetting.ViewDirectionSetting;
			if (ViewDirSetting.DirectionMode == ESigilViewDirectionMode::Aiming)
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

float USigilMainAnimInstance::GetCurveValueClamped01(const FName& CurveName) const
{
	return USigilMath::Clamp01(GetCurveValue(CurveName));
}

UBlendProfile* USigilMainAnimInstance::GetNamedBlendProfile(const FName& BlendProfileName) const
{
	if (CurrentSkeleton)
	{
		return CurrentSkeleton->GetBlendProfile(BlendProfileName);
	}
	return nullptr;
}

ESigilMovementDirection USigilMainAnimInstance::SelectCardinalDirectionFromAngle(float Angle, float DeadZone, ESigilMovementDirection CurrentDirection, bool bUseCurrentDirection) const
{
	const float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;
	if (bUseCurrentDirection)
	{
		if (CurrentDirection == ESigilMovementDirection::Forward)
		{
			FwdDeadZone *= 2;
		}
		if (CurrentDirection == ESigilMovementDirection::Backward)
		{
			BwdDeadZone *= 2;
		}
	}

	if (AbsAngle <= 45 + FwdDeadZone)
	{
		return ESigilMovementDirection::Forward;
	}

	if (AbsAngle >= 135 - BwdDeadZone)
	{
		return ESigilMovementDirection::Backward;
	}
	if (Angle > 0)
	{
		return ESigilMovementDirection::Right;
	}

	return ESigilMovementDirection::Left;
}

ESigilMovementDirection_8Way USigilMainAnimInstance::SelectOctagonalDirectionFromAngle(float Angle, float DeadZone, ESigilMovementDirection_8Way CurrentDirection,
                                                                                     bool bUseCurrentDirection) const
{
	const float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;
	if (bUseCurrentDirection)
	{
		if (CurrentDirection == ESigilMovementDirection_8Way::Forward)
		{
			FwdDeadZone *= 2;
		}
		if (CurrentDirection == ESigilMovementDirection_8Way::Backward)
		{
			BwdDeadZone *= 2;
		}
	}

	if (AbsAngle <= 22.5f + FwdDeadZone)
	{
		return ESigilMovementDirection_8Way::Forward;
	}
	else if (AbsAngle >= 157.5f - BwdDeadZone)
	{
		return ESigilMovementDirection_8Way::Backward;
	}
	else if (Angle >= 22.5f && Angle < 67.5f)
	{
		return ESigilMovementDirection_8Way::ForwardRight;
	}
	else if (Angle >= 67.5f && Angle < 112.5f)
	{
		return ESigilMovementDirection_8Way::Right;
	}
	else if (Angle >= 112.5f && Angle < 157.5f)
	{
		return ESigilMovementDirection_8Way::BackwardRight;
	}
	else if (Angle >= -157.5f && Angle < -112.5f)
	{
		return ESigilMovementDirection_8Way::BackwardLeft;
	}
	else if (Angle >= -112.5f && Angle < -67.5f)
	{
		return ESigilMovementDirection_8Way::Left;
	}
	else
	{
		return ESigilMovementDirection_8Way::ForwardLeft;
	}
}

ESigilMovementDirection USigilMainAnimInstance::GetOppositeCardinalDirection(ESigilMovementDirection CurrentDirection) const
{
	switch (CurrentDirection)
	{
	case ESigilMovementDirection::Forward:
		return ESigilMovementDirection::Backward;
	case ESigilMovementDirection::Backward:
		return ESigilMovementDirection::Forward;
	case ESigilMovementDirection::Left:
		return ESigilMovementDirection::Right;
	case ESigilMovementDirection::Right:
		return ESigilMovementDirection::Left;
	default:
		return CurrentDirection;
	}
}

bool USigilMainAnimInstance::HasCoreStateChanges() const
{
	return bMovementSetChanged || bMovementStateChanged || bLocomotionModeChanged || bOverlayModeChanged || bRotationModeChanged;
}

bool USigilMainAnimInstance::CheckCoreStateChanges(bool bCheckLocomotionMode, bool bCheckMovementSet, bool bCheckRotationMode, bool bCheckMovementState, bool bCheckOverlayMode) const
{
	return (bCheckLocomotionMode && bLocomotionModeChanged) ||
		(bCheckMovementSet && bMovementSetChanged) ||
		(bCheckRotationMode && bRotationModeChanged) ||
		(bCheckMovementState && bMovementStateChanged) ||
		(bCheckOverlayMode && bOverlayModeChanged);
}
