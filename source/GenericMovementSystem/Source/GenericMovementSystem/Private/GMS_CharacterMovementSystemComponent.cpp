// Copyright 2024 RedMoonGames All Rights Reserved.

#include "GMS_CharacterMovementSystemComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Locomotions/GMS_MainAnimInstance.h"
#include "Settings/GMS_SettingObjectLibrary.h"
#include "UniversalObjectLocators/AnimInstanceLocatorFragment.h"
#include "Utility/GMS_Constants.h"
#include "Utility/GMS_Log.h"
#include "Utility/GMS_Math.h"
#include "Utility/GMS_Utility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_CharacterMovementSystemComponent)

namespace GMS_MovementComponentConstants
{
	inline static constexpr auto TeleportDistanceThresholdSquared{FMath::Square(50.0f)};
}

UGMS_CharacterMovementSystemComponent::UGMS_CharacterMovementSystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	bReplicateUsingRegisteredSubObjectList = true;

	MovementModeToTagMapping = {
		{EMovementMode::MOVE_None, GMS_MovementModeTags::None},
		{EMovementMode::MOVE_Walking, GMS_MovementModeTags::Grounded},
		{EMovementMode::MOVE_NavWalking, GMS_MovementModeTags::Grounded},
		{EMovementMode::MOVE_Falling, GMS_MovementModeTags::InAir},
		{EMovementMode::MOVE_Swimming, GMS_MovementModeTags::Swimming},
		{EMovementMode::MOVE_Flying, GMS_MovementModeTags::Flying},
	};
}

void UGMS_CharacterMovementSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	Parameters.Condition = COND_SkipOwner;
}

void UGMS_CharacterMovementSystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		CharacterMovement = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent());
		// Set some default values here to ensure that the animation instance and the
		// camera component can read the most up-to-date values during their initialization.

		RotationMode = DesiredRotationMode;

		MovementState = DesiredMovementState;

		SetReplicatedViewRotation(OwnerPawn->GetViewRotation().GetNormalized(), false);

		ViewState.Rotation = ReplicatedViewRotation;
		ViewState.PreviousYawAngle = UE_REAL_TO_FLOAT(OwnerPawn->GetViewRotation().Yaw);

		const auto& ActorTransform{OwnerPawn->GetActorTransform()};

		LocomotionState.Location = ActorTransform.GetLocation();
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = OwnerPawn->GetActorRotation();
		LocomotionState.PreviousYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);

		RefreshTargetYawAngleUsingLocomotionRotation();

		LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
		LocomotionState.VelocityYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);

		// Make sure the mesh and animation blueprint are ticking after the character so they can access the most up-to-date character state.

		GetMesh()->AddTickPrerequisiteComponent(this);

		// Pass current movement settings to the movement component.
		//TODO

		AnimationInstance = GetMesh()->GetAnimInstance();
	}
}


void UGMS_CharacterMovementSystemComponent::BeginPlay()
{
	ensure(IsValid(AnimationInstance));

	ensureMsgf(!OwnerPawn->bUseControllerRotationPitch && !OwnerPawn->bUseControllerRotationYaw && !OwnerPawn->bUseControllerRotationRoll,
	           TEXT("These settings are not allowed and must be turned off!"));


	Super::BeginPlay();

	OwnerCharacter->MovementModeChangedDelegate.AddDynamic(this, &ThisClass::OnCharacterMovementModeChanged);
	OnCharacterMovementModeChanged(OwnerCharacter, OwnerCharacter->GetCharacterMovement()->GetGroundMovementMode(), 0);

	// Update states to use the initial desired values.

	RefreshRotationMode();

	RefreshMovementSetSetting();

	// InitiallyLoadMovementSets();
}

void UGMS_CharacterMovementSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(this, &ThisClass::OnCharacterMovementModeChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void UGMS_CharacterMovementSystemComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
}

void UGMS_CharacterMovementSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGMS_MovementSystemComponent::Tick()"), STAT_UGMS_MovementSystemComponent_Tick, STATGROUP_GMS)

	if (!IsValid(GetMovementDefinition()) || !IsValid(AnimationInstance) || !IsValid(ControlSetting))
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

	RefreshInput(DeltaTime);

	RefreshLocomotionEarly();

	RefreshView(DeltaTime);

	RefreshRotationMode();

	RefreshLocomotion(DeltaTime);

	RefreshDynamicMovementState();

	RefreshMovementState();

	RefreshGroundedRotation(DeltaTime);

	RefreshInAirRotation(DeltaTime);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RefreshLocomotionLate(DeltaTime);

	if (!OwnerCharacter->GetMesh()->bRecentlyRendered &&
		OwnerCharacter->GetMesh()->VisibilityBasedAnimTickOption > EVisibilityBasedAnimTickOption::AlwaysTickPose)
	{
		if (MainAnimInstance.IsValid())
		{
			//TODO 修复
			// LocomotionAnimInstance->MarkPendingUpdate();
		}
	}
}

void UGMS_CharacterMovementSystemComponent::OnCharacterMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	// Use the character movement mode to set the locomotion mode to the right value. This allows you to have a
	// custom set of movement modes but still use the functionality of the default character movement component.

	EMovementMode CharMovementMode = CharacterMovement->MovementMode;
	uint8 CharCustomMovementMode = CharacterMovement->CustomMovementMode;

	if (CharMovementMode != MOVE_Custom)
	{
		if (MovementModeToTagMapping.Contains(CharMovementMode) && MovementModeToTagMapping[CharMovementMode].IsValid())
		{
			SetLocomotionMode(MovementModeToTagMapping[CharMovementMode]);
		}
		else
		{
			UE_LOG(LogGMS, Error, TEXT("No locomotion mode mapping for MovementMode:%s"), *UEnum::GetDisplayValueAsText(CharMovementMode).ToString());
		}
	}
	else
	{
		if (CustomMovementModeToTagMapping.Contains(CharCustomMovementMode) && CustomMovementModeToTagMapping[CharCustomMovementMode].IsValid())
		{
			SetLocomotionMode(CustomMovementModeToTagMapping[CharCustomMovementMode]);
		}
		else
		{
			UE_LOG(LogGMS, Error, TEXT("No locomotion mode mapping for CustomMovementMode:%d"), CharCustomMovementMode);
		}
	}
}

void UGMS_CharacterMovementSystemComponent::ApplyMovementSetting()
{
	if (bAllowRefreshCharacterMovementSettings && IsValid(ControlSetting))
	{
		FGMS_MovementStateSetting TempMS;
		if (!ControlSetting->GetStateByTag(DesiredMovementState, TempMS))
		{
			TempMS = ControlSetting->MovementStates.Last();
		}

		CharacterMovement->MaxWalkSpeed = TempMS.Speed;
		CharacterMovement->MaxAcceleration = TempMS.Acceleration;
		CharacterMovement->BrakingDecelerationWalking = TempMS.BrakingDeceleration;
		CharacterMovement->MaxWalkSpeedCrouched = TempMS.Speed;
		ControlSetting->BroadcastJumpStates(ControlSetting->JumpStates);
		ControlSetting->BroadcastMovementStates(ControlSetting->MovementStates);
	}
}

void UGMS_CharacterMovementSystemComponent::RefreshInput(float DeltaTime)
{
	if (OwnerCharacter->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		SetInputDirection(CharacterMovement->GetCurrentAcceleration() / CharacterMovement->GetMaxAcceleration());
	}

	Super::RefreshInput(DeltaTime);
}

void UGMS_CharacterMovementSystemComponent::OnRotationModeChanged_Implementation(const FGameplayTag& PreviousRotationMode)
{
	Super::OnRotationModeChanged_Implementation(PreviousRotationMode);
}

void UGMS_CharacterMovementSystemComponent::RefreshView(const float DeltaTime)
{
	ViewState.PreviousYawAngle = UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw);

	if (OwnerPawn->IsLocallyControlled() || (OwnerPawn->IsReplicatingMovement() && OwnerPawn->GetLocalRole() >= ROLE_Authority && IsValid(OwnerPawn->GetController())))
	{
		// The character movement component already sends the view rotation to the
		// server if movement is replicated, so we don't have to do this ourselves.

		SetReplicatedViewRotation(OwnerPawn->GetViewRotation().GetNormalized(), !OwnerPawn->IsReplicatingMovement());
	}

	ViewState.YawAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw));

	ViewState.Rotation = ReplicatedViewRotation;
	// Set the yaw speed by comparing the current and previous view yaw angle, divided by
	// delta seconds. This represents the speed the camera is rotating from left to right.
	if (DeltaTime > UE_SMALL_NUMBER)
	{
		ViewState.YawSpeed = FMath::Abs(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - ViewState.PreviousYawAngle)) / DeltaTime;
	}
}

float UGMS_CharacterMovementSystemComponent::GetScaledCapsuleRadius() const
{
	return OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UGMS_CharacterMovementSystemComponent::GetScaledCapsuleHalfHeight() const
{
	return OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

float UGMS_CharacterMovementSystemComponent::GetMaxAcceleration() const
{
	return CharacterMovement->GetMaxAcceleration();
}

float UGMS_CharacterMovementSystemComponent::GetMaxBrakingDeceleration() const
{
	return CharacterMovement->GetMaxBrakingDeceleration();
}

float UGMS_CharacterMovementSystemComponent::GetWalkableFloorZ() const
{
	return CharacterMovement->GetWalkableFloorZ();
}

float UGMS_CharacterMovementSystemComponent::GetGravityZ() const
{
	return CharacterMovement->GetGravityZ();
}

USkeletalMeshComponent* UGMS_CharacterMovementSystemComponent::GetMesh() const
{
	return OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
}

void UGMS_CharacterMovementSystemComponent::RefreshLocomotionLocationAndRotation()
{
	const auto& ActorTransform{OwnerCharacter->GetActorTransform()};

	// If network smoothing is disabled, then return regular actor transform.

	if (CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Disabled)
	{
		LocomotionState.Location = ActorTransform.GetLocation();
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = GetOwner()->GetActorRotation();
	}
	else if (GetMesh()->IsUsingAbsoluteRotation())
	{
		LocomotionState.Location = ActorTransform.TransformPosition(GetMesh()->GetRelativeLocation() - OwnerCharacter->GetBaseTranslationOffset());
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = GetOwner()->GetActorRotation();
	}
	else
	{
		const auto SmoothTransform{
			ActorTransform * FTransform{
				GetMesh()->GetRelativeRotationCache().RotatorToQuat(GetMesh()->GetRelativeRotation()) * OwnerCharacter->GetBaseRotationOffset().Inverse(),
				GetMesh()->GetRelativeLocation() - OwnerCharacter->GetBaseTranslationOffset()
			}
		};

		LocomotionState.Location = SmoothTransform.GetLocation();
		LocomotionState.RotationQuaternion = SmoothTransform.GetRotation();
		LocomotionState.Rotation = LocomotionState.RotationQuaternion.Rotator();
	}
}

void UGMS_CharacterMovementSystemComponent::RefreshLocomotionEarly()
{
	RefreshLocomotionLocationAndRotation();

	LocomotionState.PreviousVelocity = LocomotionState.Velocity;
	LocomotionState.PreviousYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
}

void UGMS_CharacterMovementSystemComponent::RefreshLocomotion(const float DeltaTime)
{
	LocomotionState.Velocity = OwnerPawn->GetVelocity();

	// Determine if the character is moving by getting its speed. The speed equals the length
	// of the horizontal velocity, so it does not take vertical movement into account. If the
	// character is moving, update the last velocity rotation. This value is saved because it might
	// be useful to know the last orientation of a movement even after the character has stopped.

	LocomotionState.Speed = UE_REAL_TO_FLOAT(LocomotionState.Velocity.Size2D());

	static constexpr auto HasSpeedThreshold{1.0f};

	LocomotionState.bHasSpeed = LocomotionState.Speed >= HasSpeedThreshold;

	if (LocomotionState.bHasSpeed)
	{
		LocomotionState.VelocityYawAngle = UE_REAL_TO_FLOAT(UGMS_Math::DirectionToAngleXY(LocomotionState.Velocity));
	}

	if (DeltaTime > UE_SMALL_NUMBER)
	{
		LocomotionState.CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();
	}

	// Character is moving if has speed and current acceleration, or if the speed is greater than the moving speed threshold.

	LocomotionState.bMoving = (LocomotionState.bHasInput && LocomotionState.bHasSpeed) ||
		LocomotionState.Speed > ControlSetting->MovingSpeedThreshold;
}

void UGMS_CharacterMovementSystemComponent::RefreshDynamicMovementState()
{
	if (bAllowRefreshCharacterMovementSettings)
		return;

	if (IsValid(SpeedToMovementStateCurve) && OwnerPawn->HasAuthority())
	{
		int32 Index = UKismetMathLibrary::Round(SpeedToMovementStateCurve->GetFloatValue(LocomotionState.Speed));
		FGMS_MovementStateSetting TempSetting;
		if (ControlSetting->GetStateByIndex(Index, TempSetting))
		{
			SetDesiredMovement(TempSetting.Tag);
		}
		else
		{
			UE_LOG(LogGMS, Warning, TEXT("Found invalid index output from SpeedToMovementStateCurve, Index(%d) of movement definitions can't be found. dynamic adjust movement state failed! Actor:%s"),
			       Index, *GetOwner()->GetName());
		}
	}
}

void UGMS_CharacterMovementSystemComponent::RefreshLocomotionLate(const float DeltaTime)
{
	if (!LocomotionMode.IsValid())
	{
		RefreshLocomotionLocationAndRotation();
		RefreshTargetYawAngleUsingLocomotionRotation();
	}

	if (DeltaTime > UE_SMALL_NUMBER)
	{
		LocomotionState.YawSpeed = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(
			LocomotionState.Rotation.Yaw - LocomotionState.PreviousYawAngle)) / DeltaTime;
	}
}

void UGMS_CharacterMovementSystemComponent::RefreshGroundedRotation(const float DeltaTime)
{
	if (!EnableRotate)
		return;
	if (LocomotionMode != GMS_MovementModeTags::Grounded || GetGameplayTags().HasAny(GroundedRotationBlockingTags))
	{
		return;
	}

	if (OwnerCharacter->HasAnyRootMotion())
	{
		RefreshTargetYawAngleUsingLocomotionRotation();
		return;
	}

	if (!LocomotionState.bMoving)
	{
		RefreshGroundedNotMovingRotation(DeltaTime);
	}
	else
	{
		RefreshGroundedMovingRotation(DeltaTime);
	}
}

void UGMS_CharacterMovementSystemComponent::RefreshGroundedNotMovingRotation(float DeltaTime)
{
	ApplyRotationYawSpeedAnimationCurve(DeltaTime);

	if (RefreshCustomGroundedNotMovingRotation(DeltaTime))
	{
		return;
	}

	if (RotationMode == GMS_RotationModeTags::ViewDirection)
	{
		if (GetViewDirSetting().DirectionMode == EGMS_ViewDirectionMode::Default)
		{
			if (GetViewDirSetting().bRotateToViewDirectionWhileNotMoving)
			{
				SetRotationExtraSmooth(ViewState.Rotation.Yaw, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
				return;
			}
		}

		if (GetViewDirSetting().DirectionMode == EGMS_ViewDirectionMode::Aiming)
		{
			if (GetViewDirSetting().bRotateToViewDirectionWhileNotMoving)
			{
				SetRotationExtraSmooth(ViewState.Rotation.Yaw, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
				return;
			}
			else
			{
				FRotator NewActorRotation{GetOwner()->GetActorRotation()};

				SetTargetYawAngle(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw));
				//Limit rotation so turn in place can catch up.
				if (ConstrainAimingRotation(NewActorRotation, DeltaTime, true))
				{
					GetOwner()->SetActorRotation(NewActorRotation);
					RefreshLocomotionLocationAndRotation();
				}
				return;
			}
		}
	}

	if (RotationMode == GMS_RotationModeTags::VelocityDirection)
	{
		const FGMS_VelocityDirectionSetting& DirSetting = GetVelocityDirSetting();
		if (DirSetting.bEnableRotationWhenNotMoving)
		{
			if (DirSetting.DirectionMode == EGMS_VelocityDirectionMode::TurningCircle)
			{
				SetRotationInstant(DesiredVelocityYawAngle, ETeleportType::None);
				return;
			}
			SetRotationExtraSmooth(LocomotionState.TargetYawAngle, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
			return;
		}
	}

	RefreshTargetYawAngleUsingLocomotionRotation();
}

void UGMS_CharacterMovementSystemComponent::RefreshGroundedMovingRotation(float DeltaTime)
{
	// Moving.
	if (RefreshCustomGroundedMovingRotation(DeltaTime))
	{
		return;
	}

	if (RotationMode == GMS_RotationModeTags::VelocityDirection)
	{
		if (GetVelocityDirSetting().DirectionMode == EGMS_VelocityDirectionMode::TurningCircle)
		{
			SetRotationInstant(DesiredVelocityYawAngle, ETeleportType::None);
			return;
		}

		if (LocomotionState.bHasInput && GetVelocityDirSetting().DirectionMode == EGMS_VelocityDirectionMode::OrientToLastVelocityDirection)
		{
			// Rotate to the last target yaw angle when not moving (relative to the movement base or not).
			float TargetYawAngle = LocomotionState.VelocityYawAngle;

			SetRotationExtraSmooth(TargetYawAngle, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
			return;
		}

		if (LocomotionState.bHasInput && GetVelocityDirSetting().DirectionMode == EGMS_VelocityDirectionMode::OrientToInputDirection)
		{
			SetRotationExtraSmooth(LocomotionState.InputYawAngle, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
			return;
		}
	}

	if (RotationMode == GMS_RotationModeTags::ViewDirection)
	{
		if (GetViewDirSetting().DirectionMode == EGMS_ViewDirectionMode::Default)
		{
			//TODO maybe use root yaw offset here?
			float RotationYawOffset = AnimationInstance->GetCurveValue(UGMS_Constants::RotationYawOffsetCurveName());

			float TargetYawAngle{UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw + RotationYawOffset)};

			SetRotationExtraSmooth(TargetYawAngle, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
			return;
		}

		if (GetViewDirSetting().DirectionMode == EGMS_ViewDirectionMode::Aiming)
		{
			//TODO Should use this if using root yaw offset?
			FRotator NewActorRotation{GetOwner()->GetActorRotation()};

			SetTargetYawAngleSmooth(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, CalculateTargetYawRotationSpeed());

			NewActorRotation.Yaw = UGMS_Math::ExponentialDecayAngle(
				UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewActorRotation.Yaw)), LocomotionState.SmoothTargetYawAngle, DeltaTime, CalculateGroundedRotationInterpolationSpeed());

			if (ConstrainAimingRotation(NewActorRotation, DeltaTime))
			{
				// Cancel the extra smooth rotation, otherwise the actor will rotate too weirdly.
				LocomotionState.SmoothTargetYawAngle = LocomotionState.TargetYawAngle;
			}

			OwnerPawn->SetActorRotation(NewActorRotation);

			RefreshLocomotionLocationAndRotation();
			return;
		}
	}

	RefreshTargetYawAngleUsingLocomotionRotation();
}


bool UGMS_CharacterMovementSystemComponent::ConstrainAimingRotation(FRotator& ActorRotation, float DeltaTime, bool bApplySecondaryConstraint)
{
	// Limit the actor's rotation when aiming to prevent situations where the lower body noticeably
	// fails to keep up with the rotation of the upper body when the camera is rotating very fast.

	float ViewRelativeAngle{FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - ActorRotation.Yaw))};

	if (FMath::Abs(ViewRelativeAngle) <= GetViewDirSetting().MinAimingYawAngleLimit + UE_KINDA_SMALL_NUMBER)
	{
		LocomotionState.AimingYawAngleLimit = GetViewDirSetting().MinAimingYawAngleLimit;
		return false;
	}

	ViewRelativeAngle = UGMS_Math::RemapAngleForCounterClockwiseRotation(ViewRelativeAngle);

	// Secondary constraint. Simply increases the actor's rotation speed. Typically only used when the actor is standing still.

	if (bApplySecondaryConstraint)
	{
		static constexpr auto RotationInterpolationSpeed{20.0f};

		// Interpolate the angle only to the point where the constraints no longer apply to ensure a smoother completion of the rotation.

		const auto TargetViewRelativeAngle{
			FMath::Clamp(ViewRelativeAngle, -GetViewDirSetting().MinAimingYawAngleLimit,
			             GetViewDirSetting().MinAimingYawAngleLimit)
		};

		const auto DeltaAngle{FRotator3f::NormalizeAxis(TargetViewRelativeAngle - ViewRelativeAngle)};
		const auto InterpolationAmount{UGMS_Math::ExponentialDecay(DeltaTime, RotationInterpolationSpeed)};

		ViewRelativeAngle = FRotator3f::NormalizeAxis(ViewRelativeAngle + DeltaAngle * InterpolationAmount);
	}

	// Primary constraint. Prevents the actor from rotating beyond a certain angle relative to the camera.

	if (FMath::Abs(ViewRelativeAngle) > LocomotionState.AimingYawAngleLimit + UE_KINDA_SMALL_NUMBER)
	{
		ViewRelativeAngle = FMath::Clamp(ViewRelativeAngle, -LocomotionState.AimingYawAngleLimit, LocomotionState.AimingYawAngleLimit);
	}
	else
	{
		LocomotionState.AimingYawAngleLimit = FMath::Max(FMath::Abs(ViewRelativeAngle), GetViewDirSetting().MinAimingYawAngleLimit);
	}

	const auto PreviousActorYawAngle{ActorRotation.Yaw};

	ActorRotation.Yaw = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - ViewRelativeAngle));

	//有修正.
	return !FMath::IsNearlyEqual(PreviousActorYawAngle, ActorRotation.Yaw);
}

bool UGMS_CharacterMovementSystemComponent::ApplyRotationYawSpeedAnimationCurve(float DeltaTime)
{
	if (!EnableRotate)
		return false;
	// Use curve to drive actor rotation only if no root bone rotation. 仅在没有使用根骨旋转时，采用曲线驱动Actor旋转。
	if (UGMS_MainAnimInstance* AnimInst = Cast<UGMS_MainAnimInstance>(MainAnimInstance))
	{
		if (AnimInst->GetOffsetRootBoneRotationMode() != EOffsetRootBoneMode::Release)
		{
			return false;
		}
	}
	const float CurveValue = AnimationInstance->GetCurveValue(UGMS_Constants::RotationYawSpeedCurveName());
	const float DeltaYawAngle{CurveValue * DeltaTime};

	if (FMath::Abs(DeltaYawAngle) > UE_SMALL_NUMBER)
	{
		auto NewRotation{GetOwner()->GetActorRotation()};
		NewRotation.Yaw += DeltaYawAngle;

		GetOwner()->SetActorRotation(NewRotation);

		RefreshLocomotionLocationAndRotation();
		RefreshTargetYawAngleUsingLocomotionRotation();
		return true;
	}
	return false;
}

bool UGMS_CharacterMovementSystemComponent::RefreshCustomGroundedMovingRotation_Implementation(float DeltaTime)
{
	return false;
}

bool UGMS_CharacterMovementSystemComponent::RefreshCustomGroundedNotMovingRotation_Implementation(float DeltaTime)
{
	return false;
}

float UGMS_CharacterMovementSystemComponent::CalculateGroundedRotationInterpolationSpeed() const
{
	return GetMovementStateSetting().RotationInterpolationSpeed;
}

float UGMS_CharacterMovementSystemComponent::CalculateTargetYawRotationSpeed() const
{
	return GetMovementStateSetting().TargetYawAngleRotationSpeed;
}

void UGMS_CharacterMovementSystemComponent::RefreshInAirRotation(const float DeltaTime)
{
	if (!EnableRotate)
		return;
	if (LocomotionMode != GMS_MovementModeTags::InAir || GetGameplayTags().HasAny(InAirRotationBlockingTags))
	{
		return;
	}

	if (RotationMode == GMS_RotationModeTags::VelocityDirection || RotationMode == GMS_RotationModeTags::ViewDirection)
	{
		switch (ControlSetting->InAirRotationMode)
		{
		case EGMS_InAirRotationMode::RotateToVelocityOnJump:
			if (LocomotionState.bMoving)
			{
				SetRotationSmooth(LocomotionState.VelocityYawAngle, DeltaTime, ControlSetting->InAirRotationInterpolationSpeed);
			}
			else
			{
				RefreshTargetYawAngleUsingLocomotionRotation();
			}
			break;

		case EGMS_InAirRotationMode::KeepRelativeRotation:
			SetRotationSmooth(
				FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw) - LocomotionState.ViewRelativeTargetYawAngle),
				DeltaTime, ControlSetting->InAirRotationInterpolationSpeed);
			break;

		default:
			RefreshTargetYawAngleUsingLocomotionRotation();
			break;
		}
	}
	else
	{
		RefreshTargetYawAngleUsingLocomotionRotation();
	}
}

void UGMS_CharacterMovementSystemComponent::SetRotationInstant_Implementation(const float TargetYawAngle, const ETeleportType Teleport)
{
	if (!EnableRotate)
		return ;
	SetTargetYawAngle(TargetYawAngle);

	auto NewRotation{GetOwner()->GetActorRotation()};
	NewRotation.Yaw = TargetYawAngle;

	GetOwner()->SetActorRotation(NewRotation, Teleport);

	RefreshLocomotionLocationAndRotation();
}

void UGMS_CharacterMovementSystemComponent::SetRotationSmooth_Implementation(const float TargetYawAngle, const float DeltaTime, const float RotationInterpolationSpeed)
{
	if (!EnableRotate)
		return;
	SetTargetYawAngle(TargetYawAngle);

	auto NewRotation{OwnerPawn->GetActorRotation()};
	NewRotation.Yaw = UGMS_Math::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewRotation.Yaw)),
	                                                   TargetYawAngle, DeltaTime, RotationInterpolationSpeed);

	OwnerPawn->SetActorRotation(NewRotation);

	RefreshLocomotionLocationAndRotation();
}

void UGMS_CharacterMovementSystemComponent::SetRotationExtraSmooth_Implementation(const float TargetYawAngle, const float DeltaTime,
                                                                                  const float RotationInterpolationSpeed, const float TargetYawAngleRotationSpeed)
{
	if (!EnableRotate)
		return;
	LocomotionState.TargetYawAngle = TargetYawAngle;

	RefreshViewRelativeTargetYawAngle();

	// Interpolate target yaw angle for extra smooth rotation.

	LocomotionState.SmoothTargetYawAngle = UGMS_Math::InterpolateAngleConstant(LocomotionState.SmoothTargetYawAngle, TargetYawAngle,
	                                                                           DeltaTime, TargetYawAngleRotationSpeed);

	auto NewRotation{OwnerPawn->GetActorRotation()};
	NewRotation.Yaw = UGMS_Math::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewRotation.Yaw)),
	                                                   LocomotionState.SmoothTargetYawAngle, DeltaTime, RotationInterpolationSpeed);

	OwnerPawn->SetActorRotation(NewRotation);

	RefreshLocomotionLocationAndRotation();
}

void UGMS_CharacterMovementSystemComponent::RefreshTargetYawAngleUsingLocomotionRotation()
{
	SetTargetYawAngle(UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw));
}

void UGMS_CharacterMovementSystemComponent::SetTargetYawAngle(const float TargetYawAngle)
{
	LocomotionState.TargetYawAngle = FRotator3f::NormalizeAxis(TargetYawAngle);

	RefreshViewRelativeTargetYawAngle();

	LocomotionState.SmoothTargetYawAngle = LocomotionState.TargetYawAngle;
}

void UGMS_CharacterMovementSystemComponent::SetTargetYawAngleSmooth(float TargetYawAngle, float DeltaTime, float RotationSpeed)
{
	LocomotionState.TargetYawAngle = FRotator3f::NormalizeAxis(TargetYawAngle);

	LocomotionState.SmoothTargetYawAngle = UGMS_Math::InterpolateAngleConstant(
		LocomotionState.SmoothTargetYawAngle, LocomotionState.TargetYawAngle, DeltaTime, RotationSpeed);

	RefreshViewRelativeTargetYawAngle();
}

void UGMS_CharacterMovementSystemComponent::RefreshViewRelativeTargetYawAngle()
{
	if (!EnableRotate)
		return;
	LocomotionState.ViewRelativeTargetYawAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(
		ViewState.Rotation.Yaw - LocomotionState.TargetYawAngle));
}

FGMS_PredictGroundMovementPivotLocationParams UGMS_CharacterMovementSystemComponent::GetPredictGroundMovementPivotLocationParams() const
{
	FGMS_PredictGroundMovementPivotLocationParams Params;
	if (CharacterMovement)
	{
		Params.Acceleration = CharacterMovement->GetCurrentAcceleration();
		Params.Velocity = CharacterMovement->GetLastUpdateVelocity();
		Params.GroundFriction = CharacterMovement->GroundFriction;
	}
	return Params;
}

FGMS_PredictGroundMovementStopLocationParams UGMS_CharacterMovementSystemComponent::GetPredictGroundMovementStopLocationParams() const
{
	FGMS_PredictGroundMovementStopLocationParams Params;
	if (CharacterMovement)
	{
		Params.Velocity = CharacterMovement->GetLastUpdateVelocity();
		Params.bUseSeparateBrakingFriction = CharacterMovement->bUseSeparateBrakingFriction;
		Params.BrakingFriction = CharacterMovement->BrakingFriction;
		Params.GroundFriction = CharacterMovement->GroundFriction;
		Params.BrakingFrictionFactor = CharacterMovement->BrakingFrictionFactor;
		Params.BrakingDecelerationWalking = CharacterMovement->BrakingDecelerationWalking;
	}
	return Params;
}
