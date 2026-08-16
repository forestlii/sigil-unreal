// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilMovementSystemComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Animation/AnimInstance.h"
#include "Misc/DataValidation.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "Settings/SigilSettingObjectLibrary.h"
#include "Utility/SigilLog.h"
#include "Utility/SigilMath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilMovementSystemComponent)

USigilMovementSystemComponent::USigilMovementSystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USigilMovementSystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerPawn = Cast<APawn>(GetOwner());

	check(OwnerPawn)
}

void USigilMovementSystemComponent::BeginPlay()
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

void USigilMovementSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
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

USigilMovementSystemComponent* USigilMovementSystemComponent::GetMovementSystemComponent(const AActor* Actor)
{
	return Actor != nullptr ? Actor->FindComponentByClass<USigilMovementSystemComponent>() : nullptr;
}

bool USigilMovementSystemComponent::K2_FindMovementComponent(const AActor* Actor, USigilMovementSystemComponent*& Instance)
{
	if (Actor == nullptr)
	{
		return false;
	}
	Instance = GetMovementSystemComponent(Actor);
	return Instance != nullptr;
}

bool USigilMovementSystemComponent::K2_FindMovementComponentExt(const AActor* Actor, TSubclassOf<USigilMovementSystemComponent> DesiredClass, USigilMovementSystemComponent*& Instance)
{
	if (DesiredClass)
	{
		Instance = GetMovementSystemComponent(Actor);
		return Instance != nullptr && Instance->GetClass()->IsChildOf(DesiredClass);
	}
	return false;
}

FGameplayTagContainer USigilMovementSystemComponent::GetGameplayTags() const
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

void USigilMovementSystemComponent::SetGameplayTagsProvider(UObject* Provider)
{
	if (!IsValid(Provider))
	{
		UE_LOG(LogSigilMovement, Warning, TEXT("Passed invalid GameplayTagsProvider. Actor:%s  %S"), *GetName(), __FUNCTION__);
		return;
	}
	if (IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(Provider))
	{
		GameplayTagsProvider = Provider;
	}
	else
	{
		UE_LOG(LogSigilMovement, Warning, TEXT("Passed in GameplayTagsProvider(%s) Doesn't implement GameplayTagAssetInterface, it can't provide gameplay tags. Actor:%s  %S"), *Provider->GetClass()->GetName(),
		       *GetName(), __FUNCTION__);
		return;
	}
}
#pragma region GameplayTags
void USigilMovementSystemComponent::AddGameplayTag(FGameplayTag TagToAdd)
{
	AddGameplayTag(TagToAdd,true);
}

void USigilMovementSystemComponent::RemoveGameplay(FGameplayTag TagToRemove)
{
	RemoveGameplayTag(TagToRemove,true);
}

void USigilMovementSystemComponent::SetGameplayTags(FGameplayTagContainer TagsToSet)
{
	SetGameplayTags(TagsToSet,true);
}

void USigilMovementSystemComponent::AddGameplayTag(const FGameplayTag& TagToAdd, bool bSendRpc)
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

void USigilMovementSystemComponent::RemoveGameplayTag(const FGameplayTag& TagToRemove, bool bSendRpc)
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

void USigilMovementSystemComponent::SetGameplayTags(const FGameplayTagContainer& TagsToSet, bool bSendRpc)
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

void USigilMovementSystemComponent::ClientAddGameplayTag_Implementation(const FGameplayTag& TagToAdd)
{
	AddGameplayTag(TagToAdd,false);
}

void USigilMovementSystemComponent::ServerAddGameplayTag_Implementation(const FGameplayTag& TagToAdd)
{
	AddGameplayTag(TagToAdd,false);
}

void USigilMovementSystemComponent::ClientRemoveGameplayTag_Implementation(const FGameplayTag& TagToRemove)
{
	RemoveGameplayTag(TagToRemove,false);
}

void USigilMovementSystemComponent::ServerRemoveGameplayTag_Implementation(const FGameplayTag& TagToRemove)
{
	RemoveGameplayTag(TagToRemove,false);
}

void USigilMovementSystemComponent::ClientSetGameplayTags_Implementation(const FGameplayTagContainer& TagsToSet)
{
	SetGameplayTags(TagsToSet,false);
}

void USigilMovementSystemComponent::ServerSetGameplayTags_Implementation(const FGameplayTagContainer& TagsToSet)
{
	SetGameplayTags(TagsToSet,false);
}
#pragma endregion GameplayTags

#pragma region Locomotion

const FSigilLocomotionState& USigilMovementSystemComponent::GetLocomotionState() const
{
	return LocomotionState;
}

float USigilMovementSystemComponent::GetMaxAcceleration() const
{
	return 1000.0f;
}

void USigilMovementSystemComponent::SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle)
{
	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, DesiredVelocityYawAngle, NewDesiredVelocityYawAngle, this);
}

void USigilMovementSystemComponent::ServerSetDesiredVelocityYawAngle_Implementation(float NewDesiredVelocityYawAngle)
{
	SetDesiredVelocityYawAngle(NewDesiredVelocityYawAngle);
}


const FGameplayTag& USigilMovementSystemComponent::GetLocomotionMode() const
{
	return LocomotionMode;
}

void USigilMovementSystemComponent::SetLocomotionMode(const FGameplayTag& NewLocomotionMode)
{
	if (LocomotionMode != NewLocomotionMode)
	{
		const auto PreviousLocomotionMode{LocomotionMode};

		LocomotionMode = NewLocomotionMode;

		NotifyLocomotionModeChanged(PreviousLocomotionMode);
	}
}

void USigilMovementSystemComponent::NotifyLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode)
{
	OnLocomotionModeChanged(PreviousLocomotionMode);
}

void USigilMovementSystemComponent::OnLocomotionModeChanged_Implementation(const FGameplayTag& PreviousLocomotionMode)
{
	OnLocomotionModeChangedEvent.Broadcast(PreviousLocomotionMode);
}


#pragma endregion

#pragma region MovementSet

const FGameplayTag& USigilMovementSystemComponent::GetMovementSet() const
{
	return MovementSet;
}

const FSigilMovementSetSetting& USigilMovementSystemComponent::GetMovementSetSetting() const
{
	return MovementSetSetting;
}

const FSigilMovementStateSetting& USigilMovementSystemComponent::GetMovementStateSetting() const
{
	return MovementStateSetting;
}

const FSigilViewDirectionSetting& USigilMovementSystemComponent::GetViewDirSetting() const
{
	return MovementStateSetting.ViewDirectionSetting;
}

const FSigilVelocityDirectionSetting& USigilMovementSystemComponent::GetVelocityDirSetting() const
{
	return MovementStateSetting.VelocityDirectionSetting;
}

const USigilMovementControlSetting_Default* USigilMovementSystemComponent::GetControlSetting() const
{
	if (IsValid(ControlSetting))
	{
		return ControlSetting;
	}

	UE_LOG(LogSigilMovement, Warning, TEXT("ControlSetting is not valid"));
	return nullptr;
}

int32 USigilMovementSystemComponent::GetNumOfMovementStateSettings() const
{
	return IsValid(ControlSetting) ? ControlSetting->MovementStates.Num() : 0;
}
const USigilMovementDefinition* USigilMovementSystemComponent::GetMovementDefinition() const
{
	return MovementDefinition;
}

void USigilMovementSystemComponent::SetMovementSet(const FGameplayTag& NewMovementSet)
{
	SetMovementSet(NewMovementSet, true);
}

void USigilMovementSystemComponent::PushAvailableMovementDefinition(TSoftObjectPtr<USigilMovementDefinition> NewDefinition, bool bPopCurrent)
{
	if (NewDefinition.IsNull())
		return;
	USigilMovementDefinition* LoadedNewDefinition = NewDefinition.LoadSynchronous();
	if (LoadedNewDefinition == nullptr)
		return;
	PushMovementDefinition(LoadedNewDefinition, bPopCurrent, true);
}

void USigilMovementSystemComponent::PopAvailableMovementDefinition()
{
	PopMovementDefinition(true);
}

void USigilMovementSystemComponent::PushMovementDefinition(const USigilMovementDefinition* NewDefinition, bool bPopCurrent, bool bSendRpc)
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

void USigilMovementSystemComponent::ClientPushMovementDefinition_Implementation(const USigilMovementDefinition* NewDefinition, bool bPopCurrent)
{
	PushMovementDefinition(NewDefinition, bPopCurrent, false);
}

void USigilMovementSystemComponent::ServerPushMovementDefinition_Implementation(const USigilMovementDefinition* NewDefinition, bool bPopCurrent)
{
	PushMovementDefinition(NewDefinition, bPopCurrent, false);
}

void USigilMovementSystemComponent::PopMovementDefinition(bool bSendRpc)
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

void USigilMovementSystemComponent::ClientPopMovementDefinition_Implementation()
{
	PopMovementDefinition(false);
}

void USigilMovementSystemComponent::ServerPopMovementDefinition_Implementation()
{
	PopMovementDefinition(false);
}

void USigilMovementSystemComponent::OnReplicated_MovementDefinitions()
{
	OnMovementSetChanged(MovementSet);
}

void USigilMovementSystemComponent::SetMovementSet(const FGameplayTag& NewMovementSet, bool bSendRpc)
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

void USigilMovementSystemComponent::ClientSetMovementSet_Implementation(const FGameplayTag& NewMovementSet)
{
	SetMovementSet(NewMovementSet, false);
}

void USigilMovementSystemComponent::ServerSetMovementSet_Implementation(const FGameplayTag& NewMovementSet)
{
	SetMovementSet(NewMovementSet, false);
}

void USigilMovementSystemComponent::OnReplicated_MovementSet(const FGameplayTag& PreviousMovementSet)
{
	OnMovementSetChanged(PreviousMovementSet);
}

void USigilMovementSystemComponent::RefreshMovementSetSetting()
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
			UE_LOG(LogSigilMovement, VeryVerbose, TEXT("Refreshing movement set settings"));
			bFoundMovementSet = true;
			RefreshControlSetting();
			break;
		}
	}
	if (!bFoundMovementSet)
	{
		UE_LOG(LogSigilMovement, Error, TEXT("No movement set(%s) found in movement definitions on actor(%s)! %S"), *MovementSet.ToString(), *GetOwner()->GetName(), __FUNCTION__);
	}
}

void USigilMovementSystemComponent::RefreshControlSetting()
{
	 USigilMovementControlSetting_Default* NewSetting = MovementSetSetting.bControlSettingPerOverlayMode && MovementSetSetting.ControlSettings.Contains(OverlayMode)
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
		UE_LOG(LogSigilMovement, Error, TEXT("Empty MovementState settings are found in the movement set(%s) of definition(%s), which is not allowed! Actor:%s %S"), *MovementSet.ToString(),
		       *MovementDefinition->GetName(), *GetOwner()->GetName(), __FUNCTION__);
	}
}

void USigilMovementSystemComponent::OnMovementSetChanged_Implementation(const FGameplayTag& PreviousMovementSet)
{
	RefreshMovementSetSetting();
	OnMovementSetChangedEvent.Broadcast(PreviousMovementSet);
}


void USigilMovementSystemComponent::RefreshMovementStateSetting()
{
	if (!IsValid(ControlSetting))
	{
		return;
	}

	FSigilMovementStateSetting NewStateSetting;
	if (!ControlSetting->GetStateByTag(MovementState, NewStateSetting))
	{
		checkf(!ControlSetting->MovementStates.IsEmpty(), TEXT("Found empty MovementState Settings on %s!"), *ControlSetting->GetName())
		NewStateSetting = ControlSetting->MovementStates.Last();
		SetDesiredMovement(NewStateSetting.Tag);
		UE_LOG(LogSigilMovement, Verbose, TEXT("No MovementState setting for current movement state(%s), Change desired last one(%s) in list. actor:%s"), *MovementState.ToString(),
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

void USigilMovementSystemComponent::ApplyMovementSetting()
{
}

#pragma endregion

#pragma region DesiredMovement

const FGameplayTag& USigilMovementSystemComponent::GetDesiredMovementState() const
{
	return DesiredMovementState;
}

void USigilMovementSystemComponent::SetDesiredMovement(const FGameplayTag& NewDesiredMovement)
{
	SetDesiredMovement(NewDesiredMovement, true);
}

void USigilMovementSystemComponent::SetDesiredMovement(const FGameplayTag& NewDesiredMovement, bool bSendRpc)
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

void USigilMovementSystemComponent::ClientSetDesiredMovement_Implementation(const FGameplayTag& NewDesiredMovement)
{
	SetDesiredMovement(NewDesiredMovement, false);
}

void USigilMovementSystemComponent::ServerSetDesiredMovement_Implementation(const FGameplayTag& NewDesiredMovement)
{
	SetDesiredMovement(NewDesiredMovement, false);
}

void USigilMovementSystemComponent::CycleDesiredMovementState(bool bForward)
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

const FGameplayTag& USigilMovementSystemComponent::GetMovementState() const
{
	return MovementState;
}

int32 USigilMovementSystemComponent::GetSpeedLevel() const
{
	return MovementStateSetting.SpeedLevel;
}

void USigilMovementSystemComponent::SetMovementState(const FGameplayTag& NewMovementState)
{
	if (NewMovementState.IsValid() && MovementState != NewMovementState)
	{
		const FGameplayTag PreviousMovementState{MovementState};

		MovementState = NewMovementState;

		OnMovementStateChanged(PreviousMovementState);
	}
}

void USigilMovementSystemComponent::RefreshMovementState()
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

FGameplayTag USigilMovementSystemComponent::CalculateActualMovementState()
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

void USigilMovementSystemComponent::OnMovementStateChanged_Implementation(const FGameplayTag& PreviousMovementState)
{
	RefreshMovementStateSetting();
	OnMovementStateChangedEvent.Broadcast(PreviousMovementState);
}

#pragma endregion

#pragma region Input

const FVector& USigilMovementSystemComponent::GetInputDirection() const
{
	return InputDirection;
}

void USigilMovementSystemComponent::RefreshInput(float DeltaTime)
{
	LocomotionState.bHasInput = InputDirection.SizeSquared() > UE_KINDA_SMALL_NUMBER;

	if (LocomotionState.bHasInput)
	{
		LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(USigilMath::DirectionToAngleXY(InputDirection));
	}
}

void USigilMovementSystemComponent::SetInputDirection(FVector NewInputDirection)
{
	NewInputDirection = NewInputDirection.GetSafeNormal();

	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, InputDirection, NewInputDirection, this);
}

void USigilMovementSystemComponent::TurnAtRate(float Direction)
{
	if (Direction != 0 && RotationMode == SigilRotationModeTags::VelocityDirection && MovementStateSetting.VelocityDirectionSetting.DirectionMode == ESigilVelocityDirectionMode::TurningCircle &&
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

const FSigilViewState& USigilMovementSystemComponent::GetViewState() const
{
	return ViewState;
}

void USigilMovementSystemComponent::SetReplicatedViewRotation(const FRotator& NewViewRotation, bool bSendRpc)
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

void USigilMovementSystemComponent::ServerSetReplicatedViewRotation_Implementation(const FRotator& NewViewRotation)
{
	SetReplicatedViewRotation(NewViewRotation, false);
}

void USigilMovementSystemComponent::OnReplicated_ReplicatedViewRotation()
{
	ViewState.Rotation = ReplicatedViewRotation;
}

#pragma endregion

#pragma region Desired Rotation Mode

const FGameplayTag& USigilMovementSystemComponent::GetDesiredRotationMode() const
{
	return DesiredRotationMode;
}

void USigilMovementSystemComponent::SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode, true);
}

void USigilMovementSystemComponent::SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode, bool bSendRpc)
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

void USigilMovementSystemComponent::ClientSetDesiredRotationMode_Implementation(const FGameplayTag& NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode, false);
}

void USigilMovementSystemComponent::ServerSetDesiredRotationMode_Implementation(const FGameplayTag& NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode, false);
}
#pragma endregion

#pragma region Rotation Mode

const FGameplayTag& USigilMovementSystemComponent::GetRotationMode() const
{
	return RotationMode;
}

void USigilMovementSystemComponent::SetRotationMode(const FGameplayTag& NewRotationMode)
{
	if (RotationMode != NewRotationMode && GetMovementStateSetting().AllowedRotationModes.Contains(NewRotationMode))
	{
		const auto PreviousRotationMode{RotationMode};

		RotationMode = NewRotationMode;

		OnRotationModeChanged(PreviousRotationMode);
	}
}

void USigilMovementSystemComponent::OnRotationModeChanged_Implementation(const FGameplayTag& PreviousRotationMode)
{
	ApplyMovementSetting();
	OnRotationModeChangedEvent.Broadcast(PreviousRotationMode);
}

void USigilMovementSystemComponent::RefreshRotationMode()
{
	SetRotationMode(DesiredRotationMode);
}

#pragma endregion

#pragma region OverlayMode
const FGameplayTag& USigilMovementSystemComponent::GetOverlayMode() const
{
	return OverlayMode;
}

void USigilMovementSystemComponent::SetOverlayMode(const FGameplayTag& NewOverlayMode)
{
	SetOverlayMode(NewOverlayMode, true);
}

void USigilMovementSystemComponent::SetOverlayMode(const FGameplayTag& NewOverlayMode, bool bSendRpc)
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

void USigilMovementSystemComponent::ClientSetOverlayMode_Implementation(const FGameplayTag& NewOverlayMode)
{
	SetOverlayMode(NewOverlayMode, false);
}

void USigilMovementSystemComponent::ServerSetOverlayMode_Implementation(const FGameplayTag& NewOverlayMode)
{
	SetOverlayMode(NewOverlayMode, false);
}

void USigilMovementSystemComponent::OnReplicated_OverlayMode(const FGameplayTag& PreviousOverlayMode)
{
	OnOverlayModeChanged(PreviousOverlayMode);
}

void USigilMovementSystemComponent::OnOverlayModeChanged_Implementation(const FGameplayTag& PreviousOverlayMode)
{
	if (MovementSetSetting.bControlSettingPerOverlayMode)
	{
		RefreshControlSetting();
	}
	OnOverlayModeChangedEvent.Broadcast(PreviousOverlayMode);
}
#pragma endregion

void USigilMovementSystemComponent::SetEnableRotate(bool bEnable)
{
	EnableRotate = bEnable;
}

#if WITH_EDITORONLY_DATA
EDataValidationResult USigilMovementSystemComponent::IsDataValid(class FDataValidationContext& Context) const
{
	if (!IsValid(AnimGraphSetting))
	{
		Context.AddError(FText::FromString("AnimGraphSetting is required!"));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
