// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "SigilLocomotionStructLibrary.h"
#include "Engine/TimerHandle.h"
#include "TimerManager.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNodeReference.h"
#include "Engine/World.h"
#include "Settings/SigilSettingStructLibrary.h"
#include "Locomotions/SigilAnimState.h"
#include "Utility/SigilMovementTags.h"
#include "SigilMainAnimInstance.generated.h"

class USigilAnimLayerSetting_Additive;
class USigilAnimLayer;
class USigilAnimLayerSetting;
class USigilAnimLayerSetting_View;
class USigilAnimLayerSetting_Overlay;
class USigilAnimLayerSetting_States;
class USigilMovementDefinition;
class USigilMovementSystemComponent;

/**
 * Base anim template.
 */
UCLASS(BlueprintType)
class SIGILMOVEMENT_API USigilMainAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	USigilMovementSystemComponent* GetMovementSystemComponent() const;

	USigilMainAnimInstance();

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUninitializeAnimation() override;

	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;

	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;

	/**
	 * Apply an anim layer setting to anim layer instance.
	 * 应用动画层设置到动画层实例。
	 */
	virtual void SetAnimLayerBySetting(const USigilAnimLayerSetting* LayerSetting, TObjectPtr<USigilAnimLayer>& LayerInstance);


	/**
	 *  Add "anim state node to tag mappings" for the specified anim instance.
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|Animation")
	virtual void RegisterStateNameToTagMapping(UAnimInstance* SourceAnimInstance, TArray<FSigilAnimStateNameToTag> Mapping);

	/**
	 *  Remove "anim state node to tag mappings" for the specified anim instance.
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|Animation")
	virtual void UnregisterStateNameToTagMapping(UAnimInstance* SourceAnimInstance);

	/**
	 * This event is called when the core state changes, it refreshes the anim layer settings, and Link/Unlink the corresponding anim layer to the main anim instance.
	 * You only need to override this function if you have a custom version of movement definition which contains additional anim layer setting fields.
	 * 此事件会在核心状态产生变化时调用，它会刷新动画层设置，并会Link/Unlink所对应的动画层到主要动画实例。
	 * 只有你在自定义的运动定义中加入了额外的动画层设置字段时，才需要覆盖此函数。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation")
	void RefreshLayerSettings();
	virtual void RefreshLayerSettings_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void SetOffsetRootBoneRotationMode(EOffsetRootBoneMode NewRotationMode);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootBoneRotationMode() const;

protected:
	UFUNCTION(BlueprintNativeEvent, Category="GMS|Animation")
	void OnLocomotionModeChanged(const FGameplayTag& Prev);
	UFUNCTION(BlueprintNativeEvent, Category="GMS|Animation")
	void OnRotationModeChanged(const FGameplayTag& Prev);
	UFUNCTION(BlueprintNativeEvent, Category="GMS|Animation")
	void OnMovementSetChanged(const FGameplayTag& Prev);
	UFUNCTION(BlueprintNativeEvent, Category="GMS|Animation")
	void OnMovementStateChanged(const FGameplayTag& Prev);
	UFUNCTION(BlueprintNativeEvent, Category="GMS|Animation")
	void OnOverlayModeChanged(const FGameplayTag& Prev);

	virtual void RefreshViewOnGameThread();

	virtual void RefreshView(float DeltaTime);

	/**
	 * @details This function have access to other UObject, so it should be executed on game thread.
	 */
	virtual void RefreshLocomotionOnGameThread();

	virtual void RefreshLocomotion(const float DeltaTime);

	virtual void RefreshBlock();

	/**
	 * Refresh anim node relevant tags based on node to tag mapping.
	 */
	virtual void RefreshRelevanceOnGameThread();

	virtual void RefreshGrounded();

	virtual void RefreshGroundedLean();

	virtual FVector2f GetRelativeAccelerationAmount() const;

	virtual void RefreshInAir();

	virtual void RefreshGroundPrediction();

	virtual void RefreshInAirLean();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void RefreshOffsetRootBone(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

public:
	float GetCurveValueClamped01(const FName& CurveName) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	UBlendProfile* GetNamedBlendProfile(const FName& BlendProfileName) const;

	ESigilMovementDirection SelectCardinalDirectionFromAngle(float Angle, float DeadZone, ESigilMovementDirection CurrentDirection, bool bUseCurrentDirection) const;

	ESigilMovementDirection_8Way SelectOctagonalDirectionFromAngle(float Angle, float DeadZone, ESigilMovementDirection_8Way CurrentDirection, bool bUseCurrentDirection) const;

	ESigilMovementDirection GetOppositeCardinalDirection(ESigilMovementDirection CurrentDirection) const;

	/**
	 * @return True if any core state changed(movement set/state, locomotion/rotation/overlay mode.)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	bool HasCoreStateChanges() const;

	/**
	 * @return True if any selected states changed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	bool CheckCoreStateChanges(bool bCheckLocomotionMode, bool bCheckMovementSet, bool bCheckRotationMode, bool bCheckMovementState, bool bCheckOverlayMode) const;

	/**
	 * You can map animation state name to gameplay tags so that you can know if a state node is active (relevant) at runtime by checking the NodeRelevantTags.
	 * 你可以将动画状态名称映射为游戏标签，这样你就可以在运行时通过检查NodeRelevantTags从而得知某状态节点是否激活（相关），
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings", meta=(TitleProperty="Tag"))
	TArray<FSigilAnimStateNameToTag> AnimStateNameToTagMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTag LocomotionMode{SigilMovementModeTags::Grounded};

	//For chooser only, don't use it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTagContainer LocomotionModeContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTag RotationMode{SigilRotationModeTags::ViewDirection};

	//For chooser only, don't use it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTagContainer RotationModeContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTag MovementState{SigilMovementStateTags::Jog};

	//For chooser only, don't use it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTagContainer MovementStateContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTag MovementSet;

	//For chooser only, don't use it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTagContainer MovementSetContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTag OverlayMode{SigilOverlayModeTags::Default};

	//For chooser only, don't use it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FGameplayTagContainer OverlayModeContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bEnableGroundPrediction{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FSigilAnimDataSetting_General GeneralSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_Root RootState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_Locomotion LocomotionState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_View ViewState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_Lean LeanState;

	//VelocityAcceleration direction.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FVector_NetQuantizeNormal InputDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_InAir InAirState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	FGameplayTagContainer OwnedTags;

	/**
	 * You can tell if an animation node is active (relevant) by checking if this container contains a tag.
	 * 你可以通过检查此容器是否包含某tag以此得知某个动画节点是否处于激活状态（有相关性）
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	FGameplayTagContainer NodeRelevanceTags;

	UPROPERTY(Transient)
	FSigilLocomotionState GameThreadState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	bool bAnyMontagePlaying{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	bool bMovementStateChanged{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	bool bLocomotionModeChanged{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	bool bMovementSetChanged{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	bool bRotationModeChanged{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	bool bOverlayModeChanged{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State", meta=(DisplayName="Movement Blocked"))
	bool bBlocked{false};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State")
	bool bJustLanded{false};

	bool bFirstUpdate = true;

protected:
	UPROPERTY()
	TObjectPtr<USigilMovementSystemComponent> MSC;

	UPROPERTY(Transient)
	TObjectPtr<APawn> PawnOwner;

	UPROPERTY()
	TMap<TObjectPtr<UAnimInstance>, FSigilAnimStateNameToTagWrapper> RuntimeAnimStateNameToTagMappings;

	FTimerHandle InitialTimerHandle;

	UPROPERTY(VisibleInstanceOnly, Category="AnimLayers", meta=(ShowInnerProperties))
	TObjectPtr<USigilAnimLayer> StateLayerInstance;

	UPROPERTY(VisibleInstanceOnly, Category="AnimLayers", meta=(ShowInnerProperties))
	TObjectPtr<USigilAnimLayer> OverlayLayerInstance;

	UPROPERTY(VisibleInstanceOnly, Category="AnimLayers", meta=(ShowInnerProperties))
	TObjectPtr<USigilAnimLayer> ViewLayerInstance;

	UPROPERTY(VisibleInstanceOnly, Category="AnimLayers", meta=(ShowInnerProperties))
	TObjectPtr<USigilAnimLayer> AdditiveLayerInstance;

	UPROPERTY(VisibleInstanceOnly, Category="AnimLayers", meta=(ShowInnerProperties))
	TObjectPtr<USigilAnimLayer> SkeletonControlsLayerInstance;
};
