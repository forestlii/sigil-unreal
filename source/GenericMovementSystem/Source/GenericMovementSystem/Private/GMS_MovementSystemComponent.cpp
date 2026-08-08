// Copyright 2024 RedMoonGames All Rights Reserved.

#include "GMS_MovementSystemComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Animation/AnimInstance.h"
#include "Misc/DataValidation.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "Settings/GMS_SettingObjectLibrary.h"
#include "Utility/GMS_Log.h"
#include "Utility/GMS_Math.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_MovementSystemComponent)

UGMS_MovementSystemComponent::UGMS_MovementSystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGMS_MovementSystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerPawn = Cast<APawn>(GetOwner());

	check(OwnerPawn)
}

void UGMS_MovementSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	{
		if (GetOwner()->GetClass()->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
		{
			SetGameplayTagsProvider(GetOwner());
		}
		else
		{
			TArray<UActorComponent*> Components = GetOwner()->GetComponentsByInterface(UGameplayTagAssetInterface::StaticClass());
			if (Components.IsValidIndex(0))
			{
				SetGameplayTagsProvider(Components[0]);
			}
		}
	}
}

void UGMS_MovementSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	Parameters.Condition = COND_SkipOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredMovementState, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredRotationMode, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OverlayMode, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MovementSet, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MovementDefinitions, Parameters)

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InputDirection, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredVelocityYawAngle, Parameters)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OwnedTags, Parameters)
}

UGMS_MovementSystemComponent* UGMS_MovementSystemComponent::GetMovementSystemComponent(const AActor* Actor)
{
	return Actor != nullptr ? Actor->FindComponentByClass<UGMS_MovementSystemComponent>() : nullptr;
}

bool UGMS_MovementSystemComponent::K2_FindMovementComponent(const AActor* Actor, UGMS_MovementSystemComponent*& Instance)
{
	if (Actor == nullptr)
	{
		return false;
	}
	Instance = GetMovementSystemComponent(Actor);
	return Instance != nullptr;
}

bool UGMS_MovementSystemComponent::K2_FindMovementComponentExt(const AActor* Actor, TSubclassOf<UGMS_MovementSystemComponent> DesiredClass, UGMS_MovementSystemComponent*& Instance)
{
	if (DesiredClass)
	{
		Instance = GetMovementSystemComponent(Actor);
		return Instance != nullptr && Instance->GetClass()->IsChildOf(DesiredClass);
	}
	return false;
}

FGameplayTagContainer UGMS_MovementSystemComponent::GetGameplayTags() const
{
	if (IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(GameplayTagsProvider))
	{
		FGameplayTagContainer RetTags;
		TagAssetInterface->GetOwnedGameplayTags(RetTags);

		if (!OwnedTags.IsEmpty())
		{
			RetTags.AppendTags(OwnedTags);
		}

		RetTags.AddTagFast(MovementState);
		RetTags.AddTagFast(RotationMode);

		return RetTags;
	}
	return OwnedTags;
}

void UGMS_MovementSystemComponent::SetGameplayTagsProvider(UObject* Provider)
{
	if (!IsValid(Provider))
	{
		UE_LOG(LogGMS, Warning, TEXT("Passed invalid GameplayTagsProvider. Actor:%s  %S"), *GetName(), __FUNCTION__);
		return;
	}
	if (IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(Provider))
	{
		GameplayTagsProvider = Provider;
	}
	else
	{
		UE_LOG(LogGMS, Warning, TEXT("Passed in GameplayTagsProvider(%s) Doesn't implement GameplayTagAssetInterface, it can't provide gameplay tags. Actor:%s  %S"), *Provider->GetClass()->GetName(),
		       *GetName(), __FUNCTION__);
		return;
	}
}
#pragma region GameplayTags
void UGMS_MovementSystemComponent::AddGameplayTag(FGameplayTag TagToAdd)
{
	AddGameplayTag(TagToAdd,true);
}

void UGMS_MovementSystemComponent::RemoveGameplay(FGameplayTag TagToRemove)
{
	RemoveGameplayTag(TagToRemove,true);
}

void UGMS_MovementSystemComponent::SetGameplayTags(FGameplayTagContainer TagsToSet)
{
	SetGameplayTags(TagsToSet,true);
}

void UGMS_MovementSystemComponent::AddGameplayTag(const FGameplayTag& TagToAdd, bool bSendRpc)
{
	if (GetOwner()->GetLocalRole() <= ROLE_SimulatedProxy)
	{
		return;
	}

	OwnedTags.AddTag(TagToAdd);

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OwnedTags, this)

	if (bSendRpc)
	{
		if (GetOwner()->GetLocalRole() >= ROLE_Authority)
		{
			ClientAddGameplayTag(TagToAdd);
		}
		else
		{
			ServerAddGameplayTag(TagToAdd);
		}
	}
}

void UGMS_MovementSystemComponent::RemoveGameplayTag(const FGameplayTag& TagToRemove, bool bSendRpc)
{
	if (GetOwner()->GetLocalRole() <= ROLE_SimulatedProxy)
	{
		return;
	}

	OwnedTags.RemoveTag(TagToRemove);

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OwnedTags, this)

	if (bSendRpc)
	{
		if (GetOwner()->GetLocalRole() >= ROLE_Authority)
		{
			ClientRemoveGameplayTag(TagToRemove);
		}
		else
		{
			ServerRemoveGameplayTag(TagToRemove);
		}
	}
}

void UGMS_MovementSystemComponent::SetGameplayTags(const FGameplayTagContainer& TagsToSet, bool bSendRpc)
{
	if (GetOwner()->GetLocalRole() <= ROLE_SimulatedProxy)
	{
		return;
	}

	OwnedTags = TagsToSet;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OwnedTags, this)

	if (bSendRpc)
	{
		if (GetOwner()->GetLocalRole() >= ROLE_Authority)
		{
			ClientSetGameplayTags(TagsToSet);
		}
		else
		{
			ServerSetGameplayTags(TagsToSet);
		}
	}
}

void UGMS_MovementSystemComponent::ClientAddGameplayTag_Implementation(const FGameplayTag& TagToAdd)
{
	AddGameplayTag(TagToAdd,false);
}

void UGMS_MovementSystemComponent::ServerAddGameplayTag_Implementation(const FGameplayTag& TagToAdd)
{
	AddGameplayTag(TagToAdd,false);
}

void UGMS_MovementSystemComponent::ClientRemoveGameplayTag_Implementation(const FGameplayTag& TagToRemove)
{
	RemoveGameplayTag(TagToRemove,false);
}

void UGMS_MovementSystemComponent::ServerRemoveGameplayTag_Implementation(const FGameplayTag& TagToRemove)
{
	RemoveGameplayTag(TagToRemove,false);
}

void UGMS_MovementSystemComponent::ClientSetGameplayTags_Implementation(const FGameplayTagContainer& TagsToSet)
{
	SetGameplayTags(TagsToSet,false);
}

void UGMS_MovementSystemComponent::ServerSetGameplayTags_Implementation(const FGameplayTagContainer& TagsToSet)
{
	SetGameplayTags(TagsToSet,false);
}
#pragma endregion GameplayTags

#pragma region Locomotion

const FGMS_LocomotionState& UGMS_MovementSystemComponent::GetLocomotionState() const
{
	return LocomotionState;
}

float UGMS_MovementSystemComponent::GetMaxAcceleration() const
{
	return 1000.0f;
}

void UGMS_MovementSystemComponent::SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle)
{
	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, DesiredVelocityYawAngle, NewDesiredVelocityYawAngle, this);
}

void UGMS_MovementSystemComponent::ServerSetDesiredVelocityYawAngle_Implementation(float NewDesiredVelocityYawAngle)
{
	SetDesiredVelocityYawAngle(NewDesiredVelocityYawAngle);
}


const FGameplayTag& UGMS_MovementSystemComponent::GetLocomotionMode() const
{
	return LocomotionMode;
}

void UGMS_MovementSystemComponent::SetLocomotionMode(const FGameplayTag& NewLocomotionMode)
{
	if (LocomotionMode != NewLocomotionMode)
	{
		const auto PreviousLocomotionMode{LocomotionMode};

		LocomotionMode = NewLocomotionMode;

		NotifyLocomotionModeChanged(PreviousLocomotionMode);
	}
}

void UGMS_MovementSystemComponent::NotifyLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode)
{
	OnLocomotionModeChanged(PreviousLocomotionMode);
}

void UGMS_MovementSystemComponent::OnLocomotionModeChanged_Implementation(const FGameplayTag& PreviousLocomotionMode)
{
	OnLocomotionModeChangedEvent.Broadcast(PreviousLocomotionMode);
}


#pragma endregion

#pragma region MovementSet

const FGameplayTag& UGMS_MovementSystemComponent::GetMovementSet() const
{
	return MovementSet;
}

const FGMS_MovementSetSetting& UGMS_MovementSystemComponent::GetMovementSetSetting() const
{
	return MovementSetSetting;
}

const FGMS_MovementStateSetting& UGMS_MovementSystemComponent::GetMovementStateSetting() const
{
	return MovementStateSetting;
}

const FGMS_ViewDirectionSetting& UGMS_MovementSystemComponent::GetViewDirSetting() const
{
	return MovementStateSetting.ViewDirectionSetting;
}

const FGMS_VelocityDirectionSetting& UGMS_MovementSystemComponent::GetVelocityDirSetting() const
{
	return MovementStateSetting.VelocityDirectionSetting;
}

const UGMS_MovementControlSetting_Default* UGMS_MovementSystemComponent::GetControlSetting() const
{
	if (IsValid(ControlSetting))
	{
		return ControlSetting;
	}

	UE_LOG(LogGMS, Warning, TEXT("ControlSetting is not valid"));
	return nullptr;
}

void UGMS_MovementSystemComponent::SetJumpLogEnable(bool Enable)
{
	// ControlSetting is a const shared DataAsset and must not be mutated at runtime,
	// so the debug toggle lives on the component instead.
	bJumpLogEnabled = Enable;
}

int32 UGMS_MovementSystemComponent::GetNumOfMovementStateSettings() const
{
	return IsValid(ControlSetting) ? ControlSetting->MovementStates.Num() : 0;
}
const UGMS_MovementDefinition* UGMS_MovementSystemComponent::GetMovementDefinition() const
{
	return MovementDefinition;
}

void UGMS_MovementSystemComponent::SetMovementSet(const FGameplayTag& NewMovementSet)
{
	SetMovementSet(NewMovementSet, true);
}

void UGMS_MovementSystemComponent::PushAvailableMovementDefinition(TSoftObjectPtr<UGMS_MovementDefinition> NewDefinition, bool bPopCurrent)
{
	if (NewDefinition.IsNull())
		return;
	UGMS_MovementDefinition* LoadedNewDefinition = NewDefinition.LoadSynchronous();
	if (LoadedNewDefinition == nullptr)
		return;
	PushMovementDefinition(LoadedNewDefinition, bPopCurrent, true);
}

void UGMS_MovementSystemComponent::PopAvailableMovementDefinition()
{
	PopMovementDefinition(true);
}

void UGMS_MovementSystemComponent::PushMovementDefinition(const UGMS_MovementDefinition* NewDefinition, bool bPopCurrent, bool bSendRpc)
{
	if (MovementDefinitions.Num() >= 2 && bPopCurrent)
	{
		MovementDefinitions.Pop();
	}

	MovementDefinitions.Push(NewDefinition);

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementDefinitions, this)

	OnMovementSetChanged(MovementSet);

	if (bSendRpc)
	{
		if (GetOwner()->GetLocalRole() >= ROLE_Authority)
		{
			ClientPushMovementDefinition(NewDefinition, bPopCurrent);
		}
		else
		{
			ServerPushMovementDefinition(NewDefinition, bPopCurrent);
		}
	}
}

void UGMS_MovementSystemComponent::ClientPushMovementDefinition_Implementation(const UGMS_MovementDefinition* NewDefinition, bool bPopCurrent)
{
	PushMovementDefinition(NewDefinition, bPopCurrent, false);
}

void UGMS_MovementSystemComponent::ServerPushMovementDefinition_Implementation(const UGMS_MovementDefinition* NewDefinition, bool bPopCurrent)
{
	PushMovementDefinition(NewDefinition, bPopCurrent, false);
}

void UGMS_MovementSystemComponent::PopMovementDefinition(bool bSendRpc)
{
	if (MovementDefinitions.Num() > 1)
	{
		MovementDefinitions.Pop();
		OnMovementSetChanged(MovementSet);
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementDefinitions, this)

	if (bSendRpc)
	{
		if (GetOwner()->GetLocalRole() >= ROLE_Authority)
		{
			ClientPopMovementDefinition();
		}
		else
		{
			ServerPopMovementDefinition();
		}
	}
}

void UGMS_MovementSystemComponent::ClientPopMovementDefinition_Implementation()
{
	PopMovementDefinition(false);
}

void UGMS_MovementSystemComponent::ServerPopMovementDefinition_Implementation()
{
	PopMovementDefinition(false);
}

void UGMS_MovementSystemComponent::OnReplicated_MovementDefinitions()
{
	OnMovementSetChanged(MovementSet);
}

void UGMS_MovementSystemComponent::SetMovementSet(const FGameplayTag& NewMovementSet, bool bSendRpc)
{
	if (MovementSet == NewMovementSet || GetOwner()->GetLocalRole() <= ROLE_SimulatedProxy)
	{
		return;
	}

	const auto PreviousMovementSet{MovementSet};

	MovementSet = NewMovementSet;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementSet, this)

	OnMovementSetChanged(PreviousMovementSet);

	if (bSendRpc)
	{
		if (GetOwner()->GetLocalRole() >= ROLE_Authority)
		{
			ClientSetMovementSet(MovementSet);
		}
		else
		{
			ServerSetMovementSet(MovementSet);
		}
	}
}

void UGMS_MovementSystemComponent::ClientSetMovementSet_Implementation(const FGameplayTag& NewMovementSet)
{
	SetMovementSet(NewMovementSet, false);
}

void UGMS_MovementSystemComponent::ServerSetMovementSet_Implementation(const FGameplayTag& NewMovementSet)
{
	SetMovementSet(NewMovementSet, false);
}

void UGMS_MovementSystemComponent::OnReplicated_MovementSet(const FGameplayTag& PreviousMovementSet)
{
	OnMovementSetChanged(PreviousMovementSet);
}

void UGMS_MovementSystemComponent::RefreshMovementSetSetting()
{
	bool bFoundMovementSet{false};
	for (int32 i = MovementDefinitions.Num() - 1; i >= 0; i--)
	{
		if (MovementDefinitions[i].IsNull())
		{
			continue;
		}
		if (!MovementDefinitions[i].IsValid())
		{
			MovementDefinitions[i].LoadSynchronous();
		}
		if (MovementDefinitions[i]->MovementSets.Contains(MovementSet))
		{
			MovementDefinition = MovementDefinitions[i].Get();
			MovementSetSetting = MovementDefinition->MovementSets[MovementSet];
			UE_LOG(LogGMS, VeryVerbose, TEXT("Refreshing movement set settings"));
			bFoundMovementSet = true;
			RefreshControlSetting();
			break;
		}
	}
	if (!bFoundMovementSet)
	{
		UE_LOG(LogGMS, Error, TEXT("No movement set(%s) found in movement definitions on actor(%s)! %S"), *MovementSet.ToString(), *GetOwner()->GetName(), __FUNCTION__);
	}
}

void UGMS_MovementSystemComponent::RefreshControlSetting()
{
	 UGMS_MovementControlSetting_Default* NewSetting = MovementSetSetting.bControlSettingPerOverlayMode && MovementSetSetting.ControlSettings.Contains(OverlayMode)
		                                                        ? MovementSetSetting.ControlSettings[OverlayMode]
		                                                        : MovementSetSetting.ControlSetting;

	if (NewSetting != nullptr && !NewSetting->MovementStates.IsEmpty())
	{
		ControlSetting = NewSetting;
		//TODO 确保当前想要的速度存在于此控制设置，或者回调。
		RefreshMovementStateSetting();
		ApplyMovementSetting();
	}
	else
	{
		ControlSetting = nullptr;
		UE_LOG(LogGMS, Error, TEXT("Empty MovementState settings are found in the movement set(%s) of definition(%s), which is not allowed! Actor:%s %S"), *MovementSet.ToString(),
		       *MovementDefinition->GetName(), *GetOwner()->GetName(), __FUNCTION__);
	}
}

void UGMS_MovementSystemComponent::OnMovementSetChanged_Implementation(const FGameplayTag& PreviousMovementSet)
{
	RefreshMovementSetSetting();
	OnMovementSetChangedEvent.Broadcast(PreviousMovementSet);
}


void UGMS_MovementSystemComponent::RefreshMovementStateSetting()
{
	if (!IsValid(ControlSetting))
	{
		return;
	}

	FGMS_MovementStateSetting NewStateSetting;
	if (!ControlSetting->GetStateByTag(MovementState, NewStateSetting))
	{
		checkf(!ControlSetting->MovementStates.IsEmpty(), TEXT("Found empty MovementState Settings on %s!"), *ControlSetting->GetName())
		NewStateSetting = ControlSetting->MovementStates.Last();
		SetDesiredMovement(NewStateSetting.Tag);
		UE_LOG(LogGMS, Verbose, TEXT("No MovementState setting for current movement state(%s), Change desired last one(%s) in list. actor:%s"), *MovementState.ToString(),
		       *NewStateSetting.Tag.ToString(),
		       *GetOwner()->GetName());
	}

	MovementStateSetting = NewStateSetting;

	check(!MovementStateSetting.AllowedRotationModes.IsEmpty())

	if (!MovementStateSetting.AllowedRotationModes.Contains(DesiredRotationMode))
	{
		SetDesiredRotationMode(MovementStateSetting.AllowedRotationModes.Last());
	}
}

void UGMS_MovementSystemComponent::ApplyMovementSetting()
{
}

#pragma endregion

#pragma region DesiredMovement

const FGameplayTag& UGMS_MovementSystemComponent::GetDesiredMovementState() const
{
	return DesiredMovementState;
}

void UGMS_MovementSystemComponent::SetDesiredMovement(const FGameplayTag& NewDesiredMovement)
{
	SetDesiredMovement(NewDesiredMovement, true);
}

void UGMS_MovementSystemComponent::SetDesiredMovement(const FGameplayTag& NewDesiredMovement, bool bSendRpc)
{
	if (DesiredMovementState == NewDesiredMovement || GetOwner()->GetLocalRole() < ROLE_AutonomousProxy)
	{
		return;
	}
	const auto PreviousMovement{DesiredMovementState};

	DesiredMovementState = NewDesiredMovement;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredMovementState, this)

	if (bSendRpc)
	{
		if (GetOwner()->GetLocalRole() >= ROLE_Authority)
		{
			ClientSetDesiredMovement(NewDesiredMovement);
		}
		else
		{
			ServerSetDesiredMovement(NewDesiredMovement);
		}
	}
}

void UGMS_MovementSystemComponent::ClientSetDesiredMovement_Implementation(const FGameplayTag& NewDesiredMovement)
{
	SetDesiredMovement(NewDesiredMovement, false);
}

void UGMS_MovementSystemComponent::ServerSetDesiredMovement_Implementation(const FGameplayTag& NewDesiredMovement)
{
	SetDesiredMovement(NewDesiredMovement, false);
}

void UGMS_MovementSystemComponent::CycleDesiredMovementState(bool bForward)
{
	if (GetNumOfMovementStateSettings() == 1)
	{
		return;
	}

	int32 Index = ControlSetting->MovementStates.IndexOfByKey(MovementState);

	if (Index == INDEX_NONE)
	{
		return;
	}

	if (bForward && ControlSetting->MovementStates.IsValidIndex(Index + 1))
	{
		SetDesiredMovement(ControlSetting->MovementStates[Index + 1].Tag);
	}

	if (!bForward && ControlSetting->MovementStates.IsValidIndex(Index - 1))
	{
		SetDesiredMovement(ControlSetting->MovementStates[Index - 1].Tag);
	}
}

#pragma endregion

#pragma region MovementState

const FGameplayTag& UGMS_MovementSystemComponent::GetMovementState() const
{
	return MovementState;
}

int32 UGMS_MovementSystemComponent::GetSpeedLevel() const
{
	return MovementStateSetting.SpeedLevel;
}

void UGMS_MovementSystemComponent::SetMovementState(const FGameplayTag& NewMovementState)
{
	if (NewMovementState.IsValid() && MovementState != NewMovementState)
	{
		const FGameplayTag PreviousMovementState{MovementState};

		MovementState = NewMovementState;

		OnMovementStateChanged(PreviousMovementState);
	}
}

void UGMS_MovementSystemComponent::RefreshMovementState()
{
	if (!DesiredMovementState.IsValid())
	{
		return;
	}

	if (MovementState == DesiredMovementState)
	{
		return;
	}

	ApplyMovementSetting();

	SetMovementState(CalculateActualMovementState());
}

FGameplayTag UGMS_MovementSystemComponent::CalculateActualMovementState()
{
	check(GetNumOfMovementStateSettings() != 0)

	if (ControlSetting->MovementStates.Num() == 1)
	{
		return ControlSetting->MovementStates[0].Tag;
	}

	for (int32 i = 0; i < ControlSetting->MovementStates.Num(); i++)
	{
		float Speed = ControlSetting->MovementStates[i].Speed;
		if (Speed > 0.0f && LocomotionState.Speed < Speed + 10.0f)
		{
			return ControlSetting->MovementStates[i].Tag;
		}
	}

	return FGameplayTag::EmptyTag;
}

void UGMS_MovementSystemComponent::OnMovementStateChanged_Implementation(const FGameplayTag& PreviousMovementState)
{
	RefreshMovementStateSetting();
	OnMovementStateChangedEvent.Broadcast(PreviousMovementState);
}

#pragma endregion

#pragma region Input

const FVector& UGMS_MovementSystemComponent::GetInputDirection() const
{
	return InputDirection;
}

void UGMS_MovementSystemComponent::RefreshInput(float DeltaTime)
{
	LocomotionState.bHasInput = InputDirection.SizeSquared() > UE_KINDA_SMALL_NUMBER;

	if (LocomotionState.bHasInput)
	{
		LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(UGMS_Math::DirectionToAngleXY(InputDirection));
	}
}

void UGMS_MovementSystemComponent::SetInputDirection(FVector NewInputDirection)
{
	NewInputDirection = NewInputDirection.GetSafeNormal();

	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, InputDirection, NewInputDirection, this);
}

void UGMS_MovementSystemComponent::TurnAtRate(float Direction)
{
	if (Direction != 0 && RotationMode == GMS_RotationModeTags::VelocityDirection && MovementStateSetting.VelocityDirectionSetting.DirectionMode == EGMS_VelocityDirectionMode::TurningCircle &&
		OwnerPawn->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		float YawDelta = Direction * MovementStateSetting.VelocityDirectionSetting.TurningRate * GetWorld()->GetDeltaSeconds();
		if (FMath::Abs(YawDelta) > UE_SMALL_NUMBER)
		{
			float TargetYawAngle = FMath::UnwindDegrees(LocomotionState.RotationQuaternion.Rotator().Yaw + YawDelta);

			// 或者，方法2：使用 RotateAngleAxis（如果需要保持四元数精度）
			// FQuat DeltaRotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(YawDelta));
			// FQuat NewRotation = DeltaRotation * LocomotionState.RotationQuaternion;
			// NewDesiredVelocityYawAngle = NewRotation.Rotator().Yaw;

			// FRotator RotationDelta = FRotator(0,Direction * GetVelocityDirectionSetting().TurningRate * DeltaTime,0);
			//
			// NewDesiredVelocityYawAngle = (RotationDelta.Quaternion() * LocomotionState.RotationQuaternion).Rotator().Yaw;

			SetDesiredVelocityYawAngle(TargetYawAngle);
			if (OwnerPawn->GetLocalRole() < ROLE_Authority)
			{
				ServerSetDesiredVelocityYawAngle(TargetYawAngle);
			}
		}
	}
}

#pragma endregion

#pragma region ViewSystem

const FGMS_ViewState& UGMS_MovementSystemComponent::GetViewState() const
{
	return ViewState;
}

void UGMS_MovementSystemComponent::SetReplicatedViewRotation(const FRotator& NewViewRotation, bool bSendRpc)
{
	if (!ReplicatedViewRotation.Equals(NewViewRotation))
	{
		ReplicatedViewRotation = NewViewRotation;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this)

		if (bSendRpc && GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			ServerSetReplicatedViewRotation(ReplicatedViewRotation);
		}
	}
}

void UGMS_MovementSystemComponent::ServerSetReplicatedViewRotation_Implementation(const FRotator& NewViewRotation)
{
	SetReplicatedViewRotation(NewViewRotation, false);
}

void UGMS_MovementSystemComponent::OnReplicated_ReplicatedViewRotation()
{
	ViewState.Rotation = ReplicatedViewRotation;
}

#pragma endregion

#pragma region Desired Rotation Mode

const FGameplayTag& UGMS_MovementSystemComponent::GetDesiredRotationMode() const
{
	return DesiredRotationMode;
}

void UGMS_MovementSystemComponent::SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode, true);
}

void UGMS_MovementSystemComponent::SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode, bool bSendRpc)
{
	if (DesiredRotationMode == NewDesiredRotationMode || GetOwner()->GetLocalRole() < ROLE_AutonomousProxy)
	{
		return;
	}

	DesiredRotationMode = NewDesiredRotationMode;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredRotationMode, this)

	if (bSendRpc)
	{
		if (GetOwner()->GetLocalRole() >= ROLE_Authority)
		{
			ClientSetDesiredRotationMode(DesiredRotationMode);
		}
		else
		{
			ServerSetDesiredRotationMode(DesiredRotationMode);
		}
	}
}

void UGMS_MovementSystemComponent::ClientSetDesiredRotationMode_Implementation(const FGameplayTag& NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode, false);
}

void UGMS_MovementSystemComponent::ServerSetDesiredRotationMode_Implementation(const FGameplayTag& NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode, false);
}
#pragma endregion

#pragma region Rotation Mode

const FGameplayTag& UGMS_MovementSystemComponent::GetRotationMode() const
{
	return RotationMode;
}

void UGMS_MovementSystemComponent::SetRotationMode(const FGameplayTag& NewRotationMode)
{
	if (RotationMode != NewRotationMode && GetMovementStateSetting().AllowedRotationModes.Contains(NewRotationMode))
	{
		const auto PreviousRotationMode{RotationMode};

		RotationMode = NewRotationMode;

		OnRotationModeChanged(PreviousRotationMode);
	}
}

void UGMS_MovementSystemComponent::OnRotationModeChanged_Implementation(const FGameplayTag& PreviousRotationMode)
{
	ApplyMovementSetting();
	OnRotationModeChangedEvent.Broadcast(PreviousRotationMode);
}

void UGMS_MovementSystemComponent::RefreshRotationMode()
{
	SetRotationMode(DesiredRotationMode);
}

#pragma endregion

#pragma region OverlayMode
const FGameplayTag& UGMS_MovementSystemComponent::GetOverlayMode() const
{
	return OverlayMode;
}

void UGMS_MovementSystemComponent::SetOverlayMode(const FGameplayTag& NewOverlayMode)
{
	SetOverlayMode(NewOverlayMode, true);
}

void UGMS_MovementSystemComponent::SetOverlayMode(const FGameplayTag& NewOverlayMode, bool bSendRpc)
{
	if (OverlayMode == NewOverlayMode || OwnerPawn->GetLocalRole() <= ROLE_SimulatedProxy)
	{
		return;
	}

	const auto PreviousOverlayMode{OverlayMode};

	OverlayMode = NewOverlayMode;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OverlayMode, this)

	OnOverlayModeChanged(PreviousOverlayMode);

	if (bSendRpc)
	{
		if (OwnerPawn->GetLocalRole() >= ROLE_Authority)
		{
			ClientSetOverlayMode(OverlayMode);
		}
		else
		{
			ServerSetOverlayMode(OverlayMode);
		}
	}
}

void UGMS_MovementSystemComponent::ClientSetOverlayMode_Implementation(const FGameplayTag& NewOverlayMode)
{
	SetOverlayMode(NewOverlayMode, false);
}

void UGMS_MovementSystemComponent::ServerSetOverlayMode_Implementation(const FGameplayTag& NewOverlayMode)
{
	SetOverlayMode(NewOverlayMode, false);
}

void UGMS_MovementSystemComponent::OnReplicated_OverlayMode(const FGameplayTag& PreviousOverlayMode)
{
	OnOverlayModeChanged(PreviousOverlayMode);
}

void UGMS_MovementSystemComponent::OnOverlayModeChanged_Implementation(const FGameplayTag& PreviousOverlayMode)
{
	if (MovementSetSetting.bControlSettingPerOverlayMode)
	{
		RefreshControlSetting();
	}
	OnOverlayModeChangedEvent.Broadcast(PreviousOverlayMode);
}
#pragma endregion


#if WITH_EDITORONLY_DATA
void UGMS_MovementSystemComponent::SetEnableRotate(bool bEnable)
{
	EnableRotate = bEnable;
}

EDataValidationResult UGMS_MovementSystemComponent::IsDataValid(class FDataValidationContext& Context) const
{
	if (!IsValid(AnimGraphSetting))
	{
		Context.AddError(FText::FromString("AnimGraphSetting is required!"));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
