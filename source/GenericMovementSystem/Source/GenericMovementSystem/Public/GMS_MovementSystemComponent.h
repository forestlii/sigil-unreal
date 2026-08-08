// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "Components/ActorComponent.h"
#include "Locomotions/GMS_LocomotionStructLibrary.h"
#include "Settings/GMS_SettingStructLibrary.h"
#include "Utility/GMS_Tags.h"
#include "GMS_MovementSystemComponent.generated.h"

class UGMS_MainAnimInstance;
class UGMS_MovementControlSetting_Default;
class UGMS_AnimGraphSetting;
class UGMS_MovementSystemComponentSettings;
class APawn;
class UAnimInstance;
class UGMS_MovementDefinition;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementSetChangedSignature, const FGameplayTag&, PreviousMovementSet);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementChangedSignature, const FGameplayTag&, PreviousMovement);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOverlayModeChangedSignature, const FGameplayTag&, PreviousMovement);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRotationModeChangedSignature, const FGameplayTag&, PreviousRotationMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLocomotionModeChangedSignature, const FGameplayTag&, PreviousLocomotionMode);


UCLASS(BlueprintType, Abstract, NotBlueprintable)
class GENERICMOVEMENTSYSTEM_API UGMS_MovementSystemComponent : public UActorComponent
{
	GENERATED_BODY()

	friend UGMS_MainAnimInstance;

public:

	// Sets default values for this component's properties
	explicit UGMS_MovementSystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeComponent() override;

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem", meta=(DefaultToSelf="Actor", DisplayName="Get Movement System Component"))
	static UGMS_MovementSystemComponent* GetMovementSystemComponent(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem", meta=(DisplayName="Find Movement System Component", DefaultToSelf="Actor", ExpandBoolAsExecs = "ReturnValue"))
	static bool K2_FindMovementComponent(const AActor* Actor, UGMS_MovementSystemComponent*& Instance);

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem",
		meta=(DisplayName="Find Movement System Component(Ext)", DefaultToSelf="Actor", ExpandBoolAsExecs = "ReturnValue", DeterminesOutputType=DesiredClass, DynamicOutputParam="Instance"))
	static bool K2_FindMovementComponentExt(const AActor* Actor, TSubclassOf<UGMS_MovementSystemComponent> DesiredClass, UGMS_MovementSystemComponent*& Instance);

	/**
	 * Get the combination of owned tags and tags from TagProvider.
	 * 获取OwnedTags与来自TagProvider的Tags的合并Tags。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem", meta=(BlueprintThreadSafe))
	FGameplayTagContainer GetGameplayTags() const;

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	void SetGameplayTagsProvider(UObject* Provider);

#pragma region GameplayTags

public:
	/**
	 * Add a single tag to owned tags.
	 * 为OwnedTags添加单个标签。
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	void AddGameplayTag(FGameplayTag TagToAdd);

	/**
	 * Remove a single tag from owned tags.
	 * 为OwnedTags移除单个标签。
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	void RemoveGameplay(FGameplayTag TagToRemove);

	/**
	 * Override owned tags with new tags.
	 * 使用新的标签去覆盖Owned标签。
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	void SetGameplayTags(FGameplayTagContainer TagsToSet);

private:
	void AddGameplayTag(const FGameplayTag& TagToAdd, bool bSendRpc);
	void RemoveGameplayTag(const FGameplayTag& TagToRemove, bool bSendRpc);
	void SetGameplayTags(const FGameplayTagContainer& TagsToSet, bool bSendRpc);

	UFUNCTION(Client, Reliable)
	void ClientAddGameplayTag(const FGameplayTag& TagToAdd);

	UFUNCTION(Server, Reliable)
	void ServerAddGameplayTag(const FGameplayTag& TagToAdd);

	UFUNCTION(Client, Reliable)
	void ClientRemoveGameplayTag(const FGameplayTag& TagToRemove);

	UFUNCTION(Server, Reliable)
	void ServerRemoveGameplayTag(const FGameplayTag& TagToRemove);

	UFUNCTION(Client, Reliable)
	void ClientSetGameplayTags(const FGameplayTagContainer& TagsToSet);

	UFUNCTION(Server, Reliable)
	void ServerSetGameplayTags(const FGameplayTagContainer& TagsToSet);

#pragma endregion

#pragma region Abstraction

	const FGMS_LocomotionState& GetLocomotionState() const;

	virtual float GetScaledCapsuleRadius() const { return 0.0f; };

	virtual float GetScaledCapsuleHalfHeight() const { return 0.0f; };

	virtual float GetMaxAcceleration() const;

	virtual float GetMaxBrakingDeceleration() const { return 0; };

	virtual float GetWalkableFloorZ() const { return 0; }

	virtual float GetGravityZ() const { return 0; }

	virtual USkeletalMeshComponent* GetMesh() const { return nullptr; };

#pragma endregion

#pragma region Locomotion

public:
	const FGameplayTag& GetLocomotionMode() const;
	void SetLocomotionMode(const FGameplayTag& NewLocomotionMode);

protected:
	void SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle);

	UFUNCTION(Server, Unreliable)
	void ServerSetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle);
	virtual void ServerSetDesiredVelocityYawAngle_Implementation(float NewDesiredVelocityYawAngle);

	void NotifyLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);

	UFUNCTION(BlueprintNativeEvent, Category="GMS|MovementSystem")
	void OnLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);
	virtual void OnLocomotionModeChanged_Implementation(const FGameplayTag& PreviousLocomotionMode);

#pragma endregion

#pragma region MovementSet

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem")
	const FGameplayTag& GetMovementSet() const;

	// Get current selected movement definition.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem")
	const UGMS_MovementDefinition* GetMovementDefinition() const;

	// Get current movement set setting of current movement definition.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem")
	const FGMS_MovementSetSetting& GetMovementSetSetting() const;

	// Get current movement state setting of current movement set setting.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem")
	const FGMS_MovementStateSetting& GetMovementStateSetting() const;

	const FGMS_ViewDirectionSetting& GetViewDirSetting() const;
	const FGMS_VelocityDirectionSetting& GetVelocityDirSetting() const;

	// Get current selected control setting.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem", meta=(DefaultToSelf="Actor", DisplayName="Get Control Setting"))
	const UGMS_MovementControlSetting_Default* GetControlSetting() const;
	// Get current selected control setting Copy(New One).
	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	void SetJumpLogEnable(bool Enable);
	
	int32 GetNumOfMovementStateSettings() const;

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem", Meta = (AutoCreateRefTerm = "NewMovementSet"))
	void SetMovementSet(UPARAM(meta=(Categories="GMS.MovementSet")) const FGameplayTag& NewMovementSet);

	/**
	 * Pushing a new available MovementDefinition, will cause movement set setting refresh.
	 * @param NewDefinition 
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	void PushAvailableMovementDefinition(TSoftObjectPtr<UGMS_MovementDefinition> NewDefinition, bool bPopCurrent = true);

	/**
	 * Remove last available MovementDefinition from list, will cause movement set setting refresh.
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	void PopAvailableMovementDefinition();

private:
	void PushMovementDefinition(const UGMS_MovementDefinition* NewDefinition, bool bPopCurrent = true, bool bSendRpc = true);

	UFUNCTION(Client, Reliable)
	void ClientPushMovementDefinition(const UGMS_MovementDefinition* NewDefinition, bool bPopCurrent = true);
	void ClientPushMovementDefinition_Implementation(const UGMS_MovementDefinition* NewDefinition, bool bPopCurrent);

	UFUNCTION(Server, Reliable)
	void ServerPushMovementDefinition(const UGMS_MovementDefinition* NewDefinition, bool bPopCurrent = true);
	virtual void ServerPushMovementDefinition_Implementation(const UGMS_MovementDefinition* NewDefinition, bool bPopCurrent = true);

	void PopMovementDefinition(bool bSendRpc = true);

	UFUNCTION(Client, Reliable)
	void ClientPopMovementDefinition();
	void ClientPopMovementDefinition_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerPopMovementDefinition();
	virtual void ServerPopMovementDefinition_Implementation();

	UFUNCTION()
	void OnReplicated_MovementDefinitions();

private:
	void SetMovementSet(const FGameplayTag& NewMovementSet, bool bSendRpc);

	UFUNCTION(Client, Reliable)
	void ClientSetMovementSet(const FGameplayTag& NewMovementSet);

	UFUNCTION(Server, Reliable)
	void ServerSetMovementSet(const FGameplayTag& NewMovementSet);
	virtual void ServerSetMovementSet_Implementation(const FGameplayTag& NewMovementSet);

	UFUNCTION()
	void OnReplicated_MovementSet(const FGameplayTag& PreviousMovementSet);

protected:
	//update current movement set setting
	virtual void RefreshMovementSetSetting();

	virtual void RefreshControlSetting();

	//update current movement state setting
	virtual void RefreshMovementStateSetting();

	//update current movement setting.
	virtual void ApplyMovementSetting();

	UFUNCTION(BlueprintNativeEvent, Category="GMS|MovementSystem")
	void OnMovementSetChanged(const FGameplayTag& PreviousMovementSet);
	virtual void OnMovementSetChanged_Implementation(const FGameplayTag& PreviousMovementSet);

#pragma endregion

#pragma region DesiredMovementState

public:
	const FGameplayTag& GetDesiredMovementState() const;

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem", Meta = (DisplayName="Set Desired Movement State", AutoCreateRefTerm = "NewDesiredMovement"))
	void SetDesiredMovement(UPARAM(meta=(Categories="GMS.MovementState")) const FGameplayTag& NewDesiredMovement);

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	void CycleDesiredMovementState(bool bForward = true);

private:
	void SetDesiredMovement(const FGameplayTag& NewDesiredMovement, bool bSendRpc);

	UFUNCTION(Client, Reliable)
	void ClientSetDesiredMovement(const FGameplayTag& NewDesiredMovement);
	virtual void ClientSetDesiredMovement_Implementation(const FGameplayTag& NewDesiredMovement);
	UFUNCTION(Server, Reliable)
	void ServerSetDesiredMovement(const FGameplayTag& NewDesiredMovement);
	virtual void ServerSetDesiredMovement_Implementation(const FGameplayTag& NewDesiredMovement);

#pragma endregion

#pragma region MovementState

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem")
	const FGameplayTag& GetMovementState() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem")
	int32 GetSpeedLevel() const;

	void SetMovementState(const FGameplayTag& NewMovementState);

protected:
	virtual void RefreshMovementState();

	virtual FGameplayTag CalculateActualMovementState();

	UFUNCTION(BlueprintNativeEvent, Category="GMS|MovementSystem")
	void OnMovementStateChanged(const FGameplayTag& PreviousMovementState);
	virtual void OnMovementStateChanged_Implementation(const FGameplayTag& PreviousMovementState);

#pragma endregion

#pragma region Input

public:
	const FVector& GetInputDirection() const;

	UFUNCTION(BlueprintCallable, Category="GMS")
	void SetInputDirection(FVector NewInputDirection);

	UFUNCTION(BlueprintCallable, Category="GMS")
	void TurnAtRate(float Direction);

protected:
	virtual void RefreshInput(float DeltaTime);
#pragma endregion

#pragma region ViewSystem

public:
	const FGMS_ViewState& GetViewState() const;

protected:
	virtual void SetReplicatedViewRotation(const FRotator& NewViewRotation, bool bSendRpc);

	UFUNCTION(Server, Unreliable)
	virtual void ServerSetReplicatedViewRotation(const FRotator& NewViewRotation);

	UFUNCTION()
	virtual void OnReplicated_ReplicatedViewRotation();

#pragma endregion

#pragma region Desired Rotation Mode

public:
	const FGameplayTag& GetDesiredRotationMode() const;

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem", Meta = (AutoCreateRefTerm = "NewDesiredRotationMode"))
	void SetDesiredRotationMode(UPARAM(meta=(Categories="GMS.RotationMode")) const FGameplayTag& NewDesiredRotationMode);

private:
	void SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode, bool bSendRpc);

	UFUNCTION(Client, Reliable)
	void ClientSetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode);
	virtual void ClientSetDesiredRotationMode_Implementation(const FGameplayTag& NewDesiredRotationMode);

	UFUNCTION(Server, Reliable)
	void ServerSetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode);
	virtual void ServerSetDesiredRotationMode_Implementation(const FGameplayTag& NewDesiredRotationMode);

#pragma endregion

#pragma region Rotation Mode

public:
	const FGameplayTag& GetRotationMode() const;

private:
	void SetRotationMode(const FGameplayTag& NewRotationMode);

protected:
	UFUNCTION(BlueprintNativeEvent, Category="GMS|MovementSystem")
	void OnRotationModeChanged(const FGameplayTag& PreviousRotationMode);
	virtual void OnRotationModeChanged_Implementation(const FGameplayTag& PreviousRotationMode);

	virtual void RefreshRotationMode();


#pragma endregion

#pragma region OverlayMode

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem")
	const FGameplayTag& GetOverlayMode() const;

	UFUNCTION(BlueprintCallable, Category = "GMS|MovementSystem", Meta = (AutoCreateRefTerm = "NewOverlayMode", Categories="GMS.OverlayMode"))
	void SetOverlayMode(const FGameplayTag& NewOverlayMode);

private:
	void SetOverlayMode(const FGameplayTag& NewOverlayMode, bool bSendRpc);

	UFUNCTION(Client, Reliable)
	void ClientSetOverlayMode(const FGameplayTag& NewOverlayMode);

	UFUNCTION(Server, Reliable)
	void ServerSetOverlayMode(const FGameplayTag& NewOverlayMode);

	UFUNCTION()
	void OnReplicated_OverlayMode(const FGameplayTag& PreviousOverlayMode);

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
	void OnOverlayModeChanged(const FGameplayTag& PreviousOverlayMode);
#pragma endregion

#pragma region DistanceMatching

public:
	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|DistanceMatching")
	virtual FGMS_PredictGroundMovementStopLocationParams GetPredictGroundMovementStopLocationParams() const
	{
		return FGMS_PredictGroundMovementStopLocationParams();
	};

	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|DistanceMatching")
	virtual FGMS_PredictGroundMovementPivotLocationParams GetPredictGroundMovementPivotLocationParams() const
	{
		return FGMS_PredictGroundMovementPivotLocationParams();
	};
#pragma endregion

#pragma region Propertis

public:
	UPROPERTY(BlueprintAssignable, Category="Event")
	FLocomotionModeChangedSignature OnLocomotionModeChangedEvent;

	UPROPERTY(BlueprintAssignable, Category="Event")
	FMovementSetChangedSignature OnMovementSetChangedEvent;

	UPROPERTY(BlueprintAssignable, Category="Event")
	FRotationModeChangedSignature OnRotationModeChangedEvent;

	UPROPERTY(BlueprintAssignable, Category="Event")
	FMovementChangedSignature OnMovementStateChangedEvent;

	UPROPERTY(BlueprintAssignable, Category="Event")
	FOverlayModeChangedSignature OnOverlayModeChangedEvent;

protected:
	/**
	 * Available movement definitions of this movement system component
	 * @details When MovementSet is changed, the matching MovementSetSetting is queried from bottom to top of this list. if there are multiple MovementSetSettings sharing same Tag name, the lower MovementDefinition has the highest priority.
	 * @attention The purpose of this array is to dynamically add/remove movement set settings available to the character at runtime (e.g. if the character is equipped a Greatsword, the movement set settings for the GreatSword can be added) 
	 * 该运动系统组件可用的运动定义
	 * @details 当更改MovementSet时，会在此列表从下往上进行查询匹配的MovementSetSetting。若存在多个MovementSetSetting共享一个Tag，越往下的MovementSet优先级更高。
	 * @attention 此数组的目的主要用于在运行时，动态地添加/移除角色可用的运动集（比如角色装备了一把大剑，即可以添加大剑的运动集合）。
	 */
	UPROPERTY(EditAnywhere, Category="Settings|Definitions", ReplicatedUsing=OnReplicated_MovementDefinitions, meta=(DisplayName="Movement Definitions"))
	TArray<TSoftObjectPtr<const UGMS_MovementDefinition>> MovementDefinitions;

	/**
	 * Current selected movement definition.
	 * 当前选择的运动定义。
	 */
	UPROPERTY(VisibleAnywhere, Category="Settings|Definitions", meta=(DisplayName="Movement Definition"))
	TObjectPtr<const UGMS_MovementDefinition> MovementDefinition;

	/**
	 * Current selected movement set setting.
	 * 当前选择的运动集设置
	 */
	UPROPERTY(VisibleInstanceOnly, Category="Settings|Definitions")
	FGMS_MovementSetSetting MovementSetSetting;

	/**
	 * Current selected control setting.
	 * 当前选择的控制设置。
	 */
	UPROPERTY(VisibleInstanceOnly, Category="Settings|Definitions")
	TObjectPtr<UGMS_MovementControlSetting_Default> ControlSetting;

	/**
	 * Current selected movement state setting.
	 * 当前选择的运动状态设置。
	 */
	UPROPERTY(VisibleInstanceOnly, Category="Settings|Definitions")
	FGMS_MovementStateSetting MovementStateSetting;

protected:
	/**
	 * Current movement set.
	 * 当前的运动集
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", ReplicatedUsing = "OnReplicated_MovementSet", meta=(Categories="GMS.MovementSet"))
	FGameplayTag MovementSet{FGameplayTag::EmptyTag};

	/**
	 * Current desired rotation mode.
	 * 当前想要的旋转模式
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", Replicated, meta=(Categories="GMS.RotationMode"))
	FGameplayTag DesiredRotationMode{GMS_RotationModeTags::ViewDirection};

	/**
	 * Current desired movement state.
	 * 当前想要的运动状态
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", Replicated, meta=(DisplayName="Desired MovementState", Categories="GMS.MovementState"))
	FGameplayTag DesiredMovementState{GMS_MovementStateTags::Jog};

	/**
	 * Current tags of this component.
	 * @attention GameplayTagsProvider is more preferred.
	 * 当前组件的标签.
	 * @注意 更建议使用GameplayTagsProvider。
	 */
	UPROPERTY(EditAnywhere, Replicated, Category="Settings|GameplayTags")
	FGameplayTagContainer OwnedTags;

	/**
	 * This is an optional UObject which should implements GameplayTagAssetInterface. The owned tags of this object will be the return value of GetGameplayTags.
	 * @attention If you are using GameplayAbilitySystem, You can set ability system component as TagsProvider, so you can bring power of GAS to GMS.
	 * 这是一个可选的 UObject，它应该 GameplayTagAssetInterface。该对象所拥有的标签将是 GetGameplayTags 的返回值。
	 * @注意 如果您使用 GameplayAbilitySystem，您可以将能力系统组件设置为 TagsProvider，这样您就可以通过 ASC的Tags来驱动GMS的动画。
	 */
	UPROPERTY(VisibleAnywhere, Category="Settings|GameplayTags")
	TObjectPtr<UObject> GameplayTagsProvider{nullptr};

protected:
	
	UFUNCTION(BlueprintCallable, Category = "Test")
	void SetEnableRotate(bool bEnable);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Test")
	bool EnableRotate = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|GameplayTags")
	FGameplayTagContainer GroundedRotationBlockingTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|GameplayTags")
	FGameplayTagContainer InAirRotationBlockingTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, meta=(Categories="GMS.RotationMode"))
	FGameplayTag LocomotionMode{GMS_MovementModeTags::Grounded};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, meta=(Categories="GMS.MovementState"))
	FGameplayTag MovementState{GMS_MovementStateTags::Jog};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, meta=(Categories="GMS.RotationMode"))
	FGameplayTag RotationMode{GMS_RotationModeTags::ViewDirection};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", ReplicatedUsing = "OnReplicated_OverlayMode")
	FGameplayTag OverlayMode{GMS_OverlayModeTags::Default};

public:
	/**
	 * This Object hold graph specific setting.
	 * UseCase:If your project has too many similar characters but with different skeletons, and you don't intend to use the runtime animation retargeting, then you may need to set up different GraphSetting for different skeletons.
	 * UseCase2:If you need to customise the animation layer settings and implementation, then you need to change the mapping between anim layer settings and anim instances via GraphSetting.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Animation")
	TObjectPtr<UGMS_AnimGraphSetting> AnimGraphSetting{nullptr};

protected:
	/**
	 * 	The VelocityAcceleration direction.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State|Input", Transient, Replicated)
	FVector_NetQuantizeNormal InputDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, Replicated, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float DesiredVelocityYawAngle{0.0f};

	/**
	* Replicated raw view rotation. Depending on the context, this rotation can be in world space, or in movement
	* base space. In most cases, it is better to use FGMS_ViewState::Rotation to take advantage of network smoothing.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, ReplicatedUsing = "OnReplicated_ReplicatedViewRotation")
	FRotator ReplicatedViewRotation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State", Transient)
	FGMS_LocomotionState LocomotionState;

	// UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State", Transient)
	// FGMS_SpeedTransitionState SpeedTransitionState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGMS_ViewState ViewState;

	UPROPERTY(VisibleInstanceOnly, Category="State", Transient, meta=(ShowInnerProperties))
	TWeakObjectPtr<UAnimInstance> MainAnimInstance{nullptr};

	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimationInstance{nullptr};

	UPROPERTY(Transient)
	TObjectPtr<APawn> OwnerPawn{nullptr};

#pragma endregion

#if WITH_EDITORONLY_DATA
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
