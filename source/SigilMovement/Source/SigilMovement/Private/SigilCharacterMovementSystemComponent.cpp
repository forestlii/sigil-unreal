// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilCharacterMovementSystemComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Locomotions/SigilMainAnimInstance.h"
#include "Settings/SigilSettingObjectLibrary.h"
#include "Utility/SigilConstants.h"
#include "Utility/SigilLog.h"
#include "Utility/SigilMath.h"
#include "Utility/SigilUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilCharacterMovementSystemComponent)

namespace SigilMovementComponentConstants
{
	inline static constexpr auto TeleportDistanceThresholdSquared{FMath::Square(50.0f)};
}

USigilCharacterMovementSystemComponent::USigilCharacterMovementSystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	bReplicateUsingRegisteredSubObjectList = true;

	MovementModeToTagMapping = {
		{EMovementMode::MOVE_None, SigilMovementModeTags::None},
		{EMovementMode::MOVE_Walking, SigilMovementModeTags::Grounded},
		{EMovementMode::MOVE_NavWalking, SigilMovementModeTags::Grounded},
		{EMovementMode::MOVE_Falling, SigilMovementModeTags::InAir},
		{EMovementMode::MOVE_Swimming, SigilMovementModeTags::Swimming},
		{EMovementMode::MOVE_Flying, SigilMovementModeTags::Flying},
	};
}

void USigilCharacterMovementSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	Parameters.Condition = COND_SkipOwner;
}

void USigilCharacterMovementSystemComponent::SetRuntimeInitializationMode(
	const ESigilMovementRuntimeInitializationMode NewMode)
{
	RuntimeInitializationMode = NewMode;
}

ESigilMovementRuntimeInitializationMode USigilCharacterMovementSystemComponent::GetRuntimeInitializationMode() const
{
	return RuntimeInitializationMode;
}

bool USigilCharacterMovementSystemComponent::IsConfiguredRuntimeActive() const
{
	return bConfiguredRuntimeActive;
}

bool USigilCharacterMovementSystemComponent::SetRotationAuthority(
	const ESigilMovementRotationAuthority NewAuthority)
{
	if (!IsSupportedRotationAuthority(NewAuthority))
	{
		return false;
	}

	if (bRotationAuthorityLocked)
	{
		return RotationAuthority == NewAuthority;
	}

	if (!IsValid(OwnerCharacter))
	{
		RotationAuthority = NewAuthority;
		return true;
	}

	if (!ApplyRotationAuthority(NewAuthority))
	{
		return false;
	}

	RotationAuthority = NewAuthority;
	return true;
}

ESigilMovementRotationAuthority USigilCharacterMovementSystemComponent::GetRotationAuthority() const
{
	return RotationAuthority;
}

bool USigilCharacterMovementSystemComponent::IsSupportedRotationAuthority(
	const ESigilMovementRotationAuthority Authority)
{
	switch (Authority)
	{
	case ESigilMovementRotationAuthority::SigilMovement:
	case ESigilMovementRotationAuthority::Controller:
	case ESigilMovementRotationAuthority::MovementDirection:
	case ESigilMovementRotationAuthority::External:
		return true;
	default:
		return false;
	}
}

bool USigilCharacterMovementSystemComponent::ApplyRotationAuthority(
	const ESigilMovementRotationAuthority Authority)
{
	if (!IsValid(OwnerPawn) || !IsValid(OwnerCharacter) || !IsValid(CharacterMovement))
	{
		return false;
	}

	switch (Authority)
	{
	case ESigilMovementRotationAuthority::SigilMovement:
		OwnerPawn->bUseControllerRotationPitch = false;
		OwnerPawn->bUseControllerRotationYaw = false;
		OwnerPawn->bUseControllerRotationRoll = false;
		CharacterMovement->bOrientRotationToMovement = false;
		CharacterMovement->bUseControllerDesiredRotation = false;
		SetEnableRotate(true);
		break;

	case ESigilMovementRotationAuthority::Controller:
		OwnerPawn->bUseControllerRotationPitch = false;
		OwnerPawn->bUseControllerRotationYaw = true;
		OwnerPawn->bUseControllerRotationRoll = false;
		CharacterMovement->bOrientRotationToMovement = false;
		CharacterMovement->bUseControllerDesiredRotation = false;
		SetEnableRotate(false);
		break;

	case ESigilMovementRotationAuthority::MovementDirection:
		OwnerPawn->bUseControllerRotationPitch = false;
		OwnerPawn->bUseControllerRotationYaw = false;
		OwnerPawn->bUseControllerRotationRoll = false;
		CharacterMovement->bOrientRotationToMovement = true;
		CharacterMovement->bUseControllerDesiredRotation = false;
		SetEnableRotate(false);
		break;

	case ESigilMovementRotationAuthority::External:
		SetEnableRotate(false);
		break;

	default:
		return false;
	}

	return true;
}

bool USigilCharacterMovementSystemComponent::TryActivateConfiguredRuntime()
{
	if (bConfiguredRuntimeActive)
	{
		return true;
	}
	if (!IsValid(AnimGraphSetting))
	{
		return false;
	}

	if (!ApplyRotationAuthority(RotationAuthority))
	{
		return false;
	}

	AnimationInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!IsValid(AnimationInstance))
	{
		return false;
	}

	if (RotationAuthority == ESigilMovementRotationAuthority::SigilMovement
		&& (OwnerPawn->bUseControllerRotationPitch
			|| OwnerPawn->bUseControllerRotationYaw
			|| OwnerPawn->bUseControllerRotationRoll))
	{
		return false;
	}

	StartConfiguredRuntime();
	return bConfiguredRuntimeActive;
}

void USigilCharacterMovementSystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		CharacterMovement = Cast<UCharacterMovementComponent>(OwnerCharacter->GetMovementComponent());
		ApplyRotationAuthority(RotationAuthority);
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


void USigilCharacterMovementSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	const bool bRotationAuthorityApplied =
		ApplyRotationAuthority(RotationAuthority);
	bRotationAuthorityLocked = true;
	ensure(bRotationAuthorityApplied);

	if (RuntimeInitializationMode == ESigilMovementRuntimeInitializationMode::DeferredUntilConfigured)
	{
		if (!MovementDefinitions.IsEmpty())
		{
			RefreshMovementSetSetting();
		}
		return;
	}

	ensure(IsValid(AnimationInstance));
	if (RotationAuthority == ESigilMovementRotationAuthority::SigilMovement)
	{
		ensureMsgf(
			!OwnerPawn->bUseControllerRotationPitch
			&& !OwnerPawn->bUseControllerRotationYaw
			&& !OwnerPawn->bUseControllerRotationRoll,
			TEXT("Controller rotation must be disabled while SigilMovement owns rotation."));
	}

	StartConfiguredRuntime();
}

void USigilCharacterMovementSystemComponent::StartConfiguredRuntime()
{
	if (bConfiguredRuntimeActive)
	{
		return;
	}

	OwnerCharacter->MovementModeChangedDelegate.AddUniqueDynamic(
		this,
		&ThisClass::OnCharacterMovementModeChanged);
	OnCharacterMovementModeChanged(
		OwnerCharacter,
		OwnerCharacter->GetCharacterMovement()->GetGroundMovementMode(),
		0);

	// Update states to use the initial desired values.

	RefreshRotationMode();

	RefreshMovementSetSetting();
	bConfiguredRuntimeActive = true;

	// InitiallyLoadMovementSets();
}

void USigilCharacterMovementSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->MovementModeChangedDelegate.RemoveDynamic(this, &ThisClass::OnCharacterMovementModeChanged);
	}
	bConfiguredRuntimeActive = false;
	Super::EndPlay(EndPlayReason);
}

void USigilCharacterMovementSystemComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
}

void USigilCharacterMovementSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("USigilMovementSystemComponent::Tick()"), STAT_USigilMovementSystemComponent_Tick, STATGROUP_GMS)

	if (RuntimeInitializationMode == ESigilMovementRuntimeInitializationMode::DeferredUntilConfigured
		&& !bConfiguredRuntimeActive)
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

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

void USigilCharacterMovementSystemComponent::OnCharacterMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
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
			UE_LOG(LogSigilMovement, Error, TEXT("No locomotion mode mapping for MovementMode:%s"), *UEnum::GetDisplayValueAsText(CharMovementMode).ToString());
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
			UE_LOG(LogSigilMovement, Error, TEXT("No locomotion mode mapping for CustomMovementMode:%d"), CharCustomMovementMode);
		}
	}
}

void USigilCharacterMovementSystemComponent::ApplyMovementSetting()
{
	if (bAllowRefreshCharacterMovementSettings
		&& IsValid(ControlSetting)
		&& IsValid(CharacterMovement))
	{
		FSigilMovementStateSetting TempMS;
		if (!ControlSetting->GetStateByTag(DesiredMovementState, TempMS))
		{
			TempMS = ControlSetting->MovementStates.Last();
		}

		CharacterMovement->MaxWalkSpeed = TempMS.Speed;
		CharacterMovement->MaxAcceleration = TempMS.Acceleration;
		CharacterMovement->BrakingDecelerationWalking = TempMS.BrakingDeceleration;
		CharacterMovement->MaxWalkSpeedCrouched = TempMS.CrouchedSpeed;
		CharacterMovement->GetNavAgentPropertiesRef().bCanCrouch =
			ControlSetting->bCanCrouch;
		ControlSetting->BroadcastJumpStates(ControlSetting->JumpStates);
		ControlSetting->BroadcastMovementStates(ControlSetting->MovementStates);
	}
}

void USigilCharacterMovementSystemComponent::OnMovementSetChanged_Implementation(
	const FGameplayTag& PreviousMovementSet)
{
	if (RuntimeInitializationMode == ESigilMovementRuntimeInitializationMode::DeferredUntilConfigured
		&& !bConfiguredRuntimeActive
		&& MovementDefinitions.IsEmpty())
	{
		OnMovementSetChangedEvent.Broadcast(PreviousMovementSet);
		return;
	}

	Super::OnMovementSetChanged_Implementation(PreviousMovementSet);
}

void USigilCharacterMovementSystemComponent::RefreshInput(float DeltaTime)
{
	if (OwnerCharacter->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		SetInputDirection(CharacterMovement->GetCurrentAcceleration() / CharacterMovement->GetMaxAcceleration());
	}

	Super::RefreshInput(DeltaTime);
}

void USigilCharacterMovementSystemComponent::OnRotationModeChanged_Implementation(const FGameplayTag& PreviousRotationMode)
{
	Super::OnRotationModeChanged_Implementation(PreviousRotationMode);
}

void USigilCharacterMovementSystemComponent::RefreshView(const float DeltaTime)
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

float USigilCharacterMovementSystemComponent::GetScaledCapsuleRadius() const
{
	return OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float USigilCharacterMovementSystemComponent::GetScaledCapsuleHalfHeight() const
{
	return OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

float USigilCharacterMovementSystemComponent::GetMaxAcceleration() const
{
	return CharacterMovement->GetMaxAcceleration();
}

float USigilCharacterMovementSystemComponent::GetMaxBrakingDeceleration() const
{
	return CharacterMovement->GetMaxBrakingDeceleration();
}

float USigilCharacterMovementSystemComponent::GetWalkableFloorZ() const
{
	return CharacterMovement->GetWalkableFloorZ();
}

float USigilCharacterMovementSystemComponent::GetGravityZ() const
{
	return CharacterMovement->GetGravityZ();
}

USkeletalMeshComponent* USigilCharacterMovementSystemComponent::GetMesh() const
{
	return OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
}

void USigilCharacterMovementSystemComponent::RefreshLocomotionLocationAndRotation()
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

void USigilCharacterMovementSystemComponent::RefreshLocomotionEarly()
{
	RefreshLocomotionLocationAndRotation();

	LocomotionState.PreviousVelocity = LocomotionState.Velocity;
	LocomotionState.PreviousYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
}

void USigilCharacterMovementSystemComponent::RefreshLocomotion(const float DeltaTime)
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
		LocomotionState.VelocityYawAngle = UE_REAL_TO_FLOAT(USigilMath::DirectionToAngleXY(LocomotionState.Velocity));
	}

	if (DeltaTime > UE_SMALL_NUMBER)
	{
		LocomotionState.CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();
	}

	// Character is moving if has speed and current acceleration, or if the speed is greater than the moving speed threshold.

	LocomotionState.bMoving = (LocomotionState.bHasInput && LocomotionState.bHasSpeed) ||
		LocomotionState.Speed > ControlSetting->MovingSpeedThreshold;
}

void USigilCharacterMovementSystemComponent::RefreshDynamicMovementState()
{
	if (bAllowRefreshCharacterMovementSettings)
		return;

	if (IsValid(SpeedToMovementStateCurve) && OwnerPawn->HasAuthority())
	{
		int32 Index = UKismetMathLibrary::Round(SpeedToMovementStateCurve->GetFloatValue(LocomotionState.Speed));
		FSigilMovementStateSetting TempSetting;
		if (ControlSetting->GetStateByIndex(Index, TempSetting))
		{
			SetDesiredMovement(TempSetting.Tag);
		}
		else
		{
			UE_LOG(LogSigilMovement, Warning, TEXT("Found invalid index output from SpeedToMovementStateCurve, Index(%d) of movement definitions can't be found. dynamic adjust movement state failed! Actor:%s"),
			       Index, *GetOwner()->GetName());
		}
	}
}

void USigilCharacterMovementSystemComponent::RefreshLocomotionLate(const float DeltaTime)
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

void USigilCharacterMovementSystemComponent::RefreshGroundedRotation(const float DeltaTime)
{
	if (!EnableRotate)
		return;
	if (LocomotionMode != SigilMovementModeTags::Grounded || GetGameplayTags().HasAny(GroundedRotationBlockingTags))
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

void USigilCharacterMovementSystemComponent::RefreshGroundedNotMovingRotation(float DeltaTime)
{
	ApplyRotationYawSpeedAnimationCurve(DeltaTime);

	if (RefreshCustomGroundedNotMovingRotation(DeltaTime))
	{
		return;
	}

	if (RotationMode == SigilRotationModeTags::ViewDirection)
	{
		if (GetViewDirSetting().DirectionMode == ESigilViewDirectionMode::Default)
		{
			if (GetViewDirSetting().bRotateToViewDirectionWhileNotMoving)
			{
				SetRotationExtraSmooth(ViewState.Rotation.Yaw, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
				return;
			}
		}

		if (GetViewDirSetting().DirectionMode == ESigilViewDirectionMode::Aiming)
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

	if (RotationMode == SigilRotationModeTags::VelocityDirection)
	{
		const FSigilVelocityDirectionSetting& DirSetting = GetVelocityDirSetting();
		if (DirSetting.bEnableRotationWhenNotMoving)
		{
			if (DirSetting.DirectionMode == ESigilVelocityDirectionMode::TurningCircle)
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

void USigilCharacterMovementSystemComponent::RefreshGroundedMovingRotation(float DeltaTime)
{
	// Moving.
	if (RefreshCustomGroundedMovingRotation(DeltaTime))
	{
		return;
	}

	if (RotationMode == SigilRotationModeTags::VelocityDirection)
	{
		if (GetVelocityDirSetting().DirectionMode == ESigilVelocityDirectionMode::TurningCircle)
		{
			SetRotationInstant(DesiredVelocityYawAngle, ETeleportType::None);
			return;
		}

		if (LocomotionState.bHasInput && GetVelocityDirSetting().DirectionMode == ESigilVelocityDirectionMode::OrientToLastVelocityDirection)
		{
			// Rotate to the last target yaw angle when not moving (relative to the movement base or not).
			float TargetYawAngle = LocomotionState.VelocityYawAngle;

			SetRotationExtraSmooth(TargetYawAngle, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
			return;
		}

		if (LocomotionState.bHasInput && GetVelocityDirSetting().DirectionMode == ESigilVelocityDirectionMode::OrientToInputDirection)
		{
			SetRotationExtraSmooth(LocomotionState.InputYawAngle, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
			return;
		}
	}

	if (RotationMode == SigilRotationModeTags::ViewDirection)
	{
		if (GetViewDirSetting().DirectionMode == ESigilViewDirectionMode::Default)
		{
			//TODO maybe use root yaw offset here?
			float RotationYawOffset = AnimationInstance->GetCurveValue(USigilConstants::RotationYawOffsetCurveName());

			float TargetYawAngle{UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw + RotationYawOffset)};

			SetRotationExtraSmooth(TargetYawAngle, DeltaTime, CalculateGroundedRotationInterpolationSpeed(), CalculateTargetYawRotationSpeed());
			return;
		}

		if (GetViewDirSetting().DirectionMode == ESigilViewDirectionMode::Aiming)
		{
			//TODO Should use this if using root yaw offset?
			FRotator NewActorRotation{GetOwner()->GetActorRotation()};

			SetTargetYawAngleSmooth(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, CalculateTargetYawRotationSpeed());

			NewActorRotation.Yaw = USigilMath::ExponentialDecayAngle(
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


bool USigilCharacterMovementSystemComponent::ConstrainAimingRotation(FRotator& ActorRotation, float DeltaTime, bool bApplySecondaryConstraint)
{
	// Limit the actor's rotation when aiming to prevent situations where the lower body noticeably
	// fails to keep up with the rotation of the upper body when the camera is rotating very fast.

	float ViewRelativeAngle{FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - ActorRotation.Yaw))};

	if (FMath::Abs(ViewRelativeAngle) <= GetViewDirSetting().MinAimingYawAngleLimit + UE_KINDA_SMALL_NUMBER)
	{
		LocomotionState.AimingYawAngleLimit = GetViewDirSetting().MinAimingYawAngleLimit;
		return false;
	}

	ViewRelativeAngle = USigilMath::RemapAngleForCounterClockwiseRotation(ViewRelativeAngle);

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
		const auto InterpolationAmount{USigilMath::ExponentialDecay(DeltaTime, RotationInterpolationSpeed)};

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

bool USigilCharacterMovementSystemComponent::ApplyRotationYawSpeedAnimationCurve(float DeltaTime)
{
	if (!EnableRotate)
		return false;
	// Use curve to drive actor rotation only if no root bone rotation. 仅在没有使用根骨旋转时，采用曲线驱动Actor旋转。
	if (USigilMainAnimInstance* AnimInst = Cast<USigilMainAnimInstance>(MainAnimInstance))
	{
		if (AnimInst->GetOffsetRootBoneRotationMode() != EOffsetRootBoneMode::Release)
		{
			return false;
		}
	}
	const float CurveValue = AnimationInstance->GetCurveValue(USigilConstants::RotationYawSpeedCurveName());
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

bool USigilCharacterMovementSystemComponent::RefreshCustomGroundedMovingRotation_Implementation(float DeltaTime)
{
	return false;
}

bool USigilCharacterMovementSystemComponent::RefreshCustomGroundedNotMovingRotation_Implementation(float DeltaTime)
{
	return false;
}

float USigilCharacterMovementSystemComponent::CalculateGroundedRotationInterpolationSpeed() const
{
	return GetMovementStateSetting().RotationInterpolationSpeed;
}

float USigilCharacterMovementSystemComponent::CalculateTargetYawRotationSpeed() const
{
	return GetMovementStateSetting().TargetYawAngleRotationSpeed;
}

void USigilCharacterMovementSystemComponent::RefreshInAirRotation(const float DeltaTime)
{
	if (!EnableRotate)
		return;
	if (LocomotionMode != SigilMovementModeTags::InAir || GetGameplayTags().HasAny(InAirRotationBlockingTags))
	{
		return;
	}

	if (RotationMode == SigilRotationModeTags::VelocityDirection || RotationMode == SigilRotationModeTags::ViewDirection)
	{
		switch (ControlSetting->InAirRotationMode)
		{
		case ESigilInAirRotationMode::RotateToVelocityOnJump:
			if (LocomotionState.bMoving)
			{
				SetRotationSmooth(LocomotionState.VelocityYawAngle, DeltaTime, ControlSetting->InAirRotationInterpolationSpeed);
			}
			else
			{
				RefreshTargetYawAngleUsingLocomotionRotation();
			}
			break;

		case ESigilInAirRotationMode::KeepRelativeRotation:
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

void USigilCharacterMovementSystemComponent::SetRotationInstant_Implementation(const float TargetYawAngle, const ETeleportType Teleport)
{
	if (!EnableRotate)
		return ;
	SetTargetYawAngle(TargetYawAngle);

	auto NewRotation{GetOwner()->GetActorRotation()};
	NewRotation.Yaw = TargetYawAngle;

	GetOwner()->SetActorRotation(NewRotation, Teleport);

	RefreshLocomotionLocationAndRotation();
}

void USigilCharacterMovementSystemComponent::SetRotationSmooth_Implementation(const float TargetYawAngle, const float DeltaTime, const float RotationInterpolationSpeed)
{
	if (!EnableRotate)
		return;
	SetTargetYawAngle(TargetYawAngle);

	auto NewRotation{OwnerPawn->GetActorRotation()};
	NewRotation.Yaw = USigilMath::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewRotation.Yaw)),
	                                                   TargetYawAngle, DeltaTime, RotationInterpolationSpeed);

	OwnerPawn->SetActorRotation(NewRotation);

	RefreshLocomotionLocationAndRotation();
}

void USigilCharacterMovementSystemComponent::SetRotationExtraSmooth_Implementation(const float TargetYawAngle, const float DeltaTime,
                                                                                  const float RotationInterpolationSpeed, const float TargetYawAngleRotationSpeed)
{
	if (!EnableRotate)
		return;
	LocomotionState.TargetYawAngle = TargetYawAngle;

	RefreshViewRelativeTargetYawAngle();

	// Interpolate target yaw angle for extra smooth rotation.

	LocomotionState.SmoothTargetYawAngle = USigilMath::InterpolateAngleConstant(LocomotionState.SmoothTargetYawAngle, TargetYawAngle,
	                                                                           DeltaTime, TargetYawAngleRotationSpeed);

	auto NewRotation{OwnerPawn->GetActorRotation()};
	NewRotation.Yaw = USigilMath::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewRotation.Yaw)),
	                                                   LocomotionState.SmoothTargetYawAngle, DeltaTime, RotationInterpolationSpeed);

	OwnerPawn->SetActorRotation(NewRotation);

	RefreshLocomotionLocationAndRotation();
}

void USigilCharacterMovementSystemComponent::RefreshTargetYawAngleUsingLocomotionRotation()
{
	SetTargetYawAngle(UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw));
}

void USigilCharacterMovementSystemComponent::SetTargetYawAngle(const float TargetYawAngle)
{
	LocomotionState.TargetYawAngle = FRotator3f::NormalizeAxis(TargetYawAngle);

	RefreshViewRelativeTargetYawAngle();

	LocomotionState.SmoothTargetYawAngle = LocomotionState.TargetYawAngle;
}

void USigilCharacterMovementSystemComponent::SetTargetYawAngleSmooth(float TargetYawAngle, float DeltaTime, float RotationSpeed)
{
	LocomotionState.TargetYawAngle = FRotator3f::NormalizeAxis(TargetYawAngle);

	LocomotionState.SmoothTargetYawAngle = USigilMath::InterpolateAngleConstant(
		LocomotionState.SmoothTargetYawAngle, LocomotionState.TargetYawAngle, DeltaTime, RotationSpeed);

	RefreshViewRelativeTargetYawAngle();
}

void USigilCharacterMovementSystemComponent::RefreshViewRelativeTargetYawAngle()
{
	if (!EnableRotate)
		return;
	LocomotionState.ViewRelativeTargetYawAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(
		ViewState.Rotation.Yaw - LocomotionState.TargetYawAngle));
}

FSigilPredictGroundMovementPivotLocationParams USigilCharacterMovementSystemComponent::GetPredictGroundMovementPivotLocationParams() const
{
	FSigilPredictGroundMovementPivotLocationParams Params;
	if (CharacterMovement)
	{
		Params.Acceleration = CharacterMovement->GetCurrentAcceleration();
		Params.Velocity = CharacterMovement->GetLastUpdateVelocity();
		Params.GroundFriction = CharacterMovement->GroundFriction;
	}
	return Params;
}

FSigilPredictGroundMovementStopLocationParams USigilCharacterMovementSystemComponent::GetPredictGroundMovementStopLocationParams() const
{
	FSigilPredictGroundMovementStopLocationParams Params;
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
