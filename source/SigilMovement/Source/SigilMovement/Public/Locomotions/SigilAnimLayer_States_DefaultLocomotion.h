// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAnimLayer_States.h"
#include "SigilAnimState.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "SigilAnimLayer_States_DefaultLocomotion.generated.h"

#pragma region Settings

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Jump : public FSigilAnimData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> JumpStart = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> JumpStartLoop = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> JumpApex = nullptr;

	/***
	 * Required for fall state, or animation state machine won't entry fall state.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> JumpFallLoop = nullptr;

	/***
	 * Optional fall to land animation, will start to play when the character is about to touch the ground,
	 * the advancement of the animation timeline will be based on the animation's GroundDistance curve to match to ‘InAirState.GroundDistance’.
	 * @Example For example, a 2 seconds animation with a curve value of -300~0, when the character is falling, the animation will start to play when it is still 3m away from the ground,
	 * when it is still 2m away from the ground, the animation will play to the point where the curve value of -200 is located,
	 * when it is still 1m away from the ground, the animation will play to the point where the curve value of -100 is located. And so on until the character hits the ground and the animation finishes.
	 * 可选的落地动画，会在即将接触地面时开始播放，动画时间轴的推进会根据动画的GroundDistance曲线距离匹配到“InAirState.GroundDistance”。
	 * @示例 比如一个2秒的动画，其曲线值在-300~0,当角色下落时，距离地面还有3米时开始播放该动画，距离地面还有2米时，动画会播放到-200的曲线值所在的时间点，距离地面还有1米时，动画会播放到-100的曲线所在的时间点。以此类推直到角色着地，动画也播放完毕。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="bEnableGroundPrediction", EditConditionHides))
	TObjectPtr<UAnimSequence> JumpFallLand = nullptr;

	/**
	 * Should turn off if you don't use JumpFallLand.
	 * 关闭它以节约性能，如果你不需要播放JumpFallLand动画。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bEnableGroundPrediction{true};

	UPROPERTY(EditAnywhere, Category="GMS", BlueprintReadWrite, meta=(EditCondition=false, EditConditionHides))
	bool bValidJumpStartLoop{false};

	UPROPERTY(EditAnywhere, Category="GMS", BlueprintReadWrite, meta=(EditCondition=false, EditConditionHides))
	bool bValidJumpApex{false};

	UPROPERTY(EditAnywhere, Category="GMS", BlueprintReadWrite, meta=(EditCondition=false, EditConditionHides))
	bool bValidJumpFallLoop{false};

	UPROPERTY(EditAnywhere, Category="GMS", BlueprintReadWrite, meta=(EditCondition=false, EditConditionHides))
	bool bValidJumpFallLand{false};

	virtual void Validate() override;
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Idle : public FSigilAnimData
{
	GENERATED_BODY()

	virtual void Validate() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Idle{nullptr};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	float BlendTime{0.3f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bDisableIdleBreaks{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="!bDisableIdleBreaks", EditConditionHides))
	TArray<TObjectPtr<UAnimSequence>> Idle_Breaks;

	/**
	 * You set a value > 0 to enable fixed idle break interval. or the interval will be determined by nums of IdleBreak animations.  
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="!bDisableIdleBreaks", EditConditionHides))
	float IdleBreakDelayTime{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(DisplayName="Entry"))
	TObjectPtr<UAnimSequence> CrouchEntry{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(DisplayName="Exit"))
	TObjectPtr<UAnimSequence> CrouchExit{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition=false, EditConditionHides))
	bool bValidCrouchAnim{false};
};

UENUM(BlueprintType)
enum class ESigilStartAnimType_ViewDir : uint8
{
	Direction_4,
	Direction_8,
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Start_ViewDirection : public FSigilAnimData
{
	GENERATED_BODY()

	virtual void Validate() override;

	/**
	 * 4 direction are more preferred.
	 * 推荐使用4方向。
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	ESigilStartAnimType_ViewDir AnimType{ESigilStartAnimType_ViewDir::Direction_4};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS", meta=(EditCondition="AnimType==ESigilStartAnimType_ViewDir::Direction_4", EditConditionHides))
	FSigilAnimations_4Direction Animations;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS", meta=(EditCondition="AnimType==ESigilStartAnimType_ViewDir::Direction_8", EditConditionHides))
	FSigilAnimations_8Direction Animations_8Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector2D PlayRateClamp{0.8, 1.2};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GMS")
	FSigilStrideWarpingSettings StrideWarping;
};

UENUM(BlueprintType)
enum class ESigilStartAnimType_VelocityDir : uint8
{
	Reface,
	Single,
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Start_VelocityDirection : public FSigilAnimData
{
	GENERATED_BODY()

	virtual void Validate() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilStartAnimType_VelocityDir AnimType{ESigilStartAnimType_VelocityDir::Reface};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="AnimType==ESigilStartAnimType_VelocityDir::Reface", EditConditionHides))
	FSigilAnimations_StartForwardFacing Animations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="AnimType==ESigilStartAnimType_VelocityDir::Single", EditConditionHides))
	TObjectPtr<UAnimSequence> Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FVector2D PlayRateClamp{0.8, 1.2};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GMS")
	FSigilStrideWarpingSettings StrideWarping;
	
	/**
	 * Controls the steering behavior of start.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GMS")
	FSigilSteeringSettings Steering;
};

UENUM(BlueprintType)
enum class ESigilCycleAnimType : uint8
{
	Direction_4,
	Direction_8,
	Single,
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Cycle : public FSigilAnimData
{
	GENERATED_BODY()

	virtual void Validate() override;

	/**
	 * 4 direction are more preferred.
	 * 推荐使用4方向。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilCycleAnimType AnimType{ESigilCycleAnimType::Direction_4};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS", meta=(EditCondition="AnimType==ESigilCycleAnimType::Direction_4", EditConditionHides))
	FSigilAnimations_4Direction Animations;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS", meta=(EditCondition="AnimType==ESigilCycleAnimType::Direction_8", EditConditionHides))
	FSigilAnimations_8Direction Animations_8Direction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS", meta=(EditCondition="AnimType==ESigilCycleAnimType::Single", EditConditionHides))
	TObjectPtr<UAnimSequence> Animation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	float BlendTime{0.25f};

	/**
	 * Should adjust animations play rate to match the in-game movement speed? （是否调整动画播放速率以匹配游戏内实际移动速度？）
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	bool bDynamicPlayRate{true};

	/**
	 * If animations have root motion, GMS will use root motion delta to calculate play rate. if not, using Animated animation speed. （如果动画包含根运动，则会使用根运动增量来动态计算播放速率，否则会使用动画被制作时确定的速度值。）
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS", meta=(EditCondition="bDynamicPlayRate", EditConditionHides))
	bool bHasRootMotion{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "cm/s", EditCondition="!bHasRootMotion", EditConditionHides))
	float AnimatedSpeed{350.0f};

	/**
	 * Dynamic play rate adjustment range limitation.
	 * 动态播放倍率调整的范围
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	FVector2D PlayRateClamp{0.8f, 1.2f};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	bool bEnableStrideWarping{true};
};

UENUM(BlueprintType)
enum class ESigilStopAnimType : uint8
{
	Direction_4,
	Direction_8,
	Single,
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Stop : public FSigilAnimData
{
	GENERATED_BODY()

	virtual void Validate() override;

	/**
	 * 4 direction are more preferred.
	 * 推荐使用4方向。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilStopAnimType AnimType{ESigilStopAnimType::Direction_4};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="AnimType==ESigilStopAnimType::Direction_4", EditConditionHides))
	FSigilAnimations_4Direction Animations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="AnimType==ESigilStopAnimType::Single", EditConditionHides))
	TObjectPtr<UAnimSequence> Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="AnimType==ESigilStopAnimType::Direction_8", EditConditionHides))
	FSigilAnimations_8Direction Animations_8Direction;
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Pivot : public FSigilAnimData
{
	GENERATED_BODY()

	virtual void Validate() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	FSigilAnimations_4Direction Animations;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	float PivotCooldown{0.2f};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS")
	FVector2D PlayRateClamp{0.8, 1.2};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GMS")
	FSigilStrideWarpingSettings StrideWarping;
};


/**
 * Animations about land.
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Land : public FSigilAnimData
{
	GENERATED_BODY()

	/**
	 * Distance based land animations, The one with distance closest to Vertical speed will be selected, or fallback to last one.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GMS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAnimationWithDistance> Lands;

	virtual void Validate() override;
};


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Lean : public FSigilAnimData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UBlendSpace> BlendSpace{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(ClampMin=1, DisplayName="Max left/right lean angle"))
	float MaxXLeanAngle{20.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(ClampMin=1, DisplayName="Max forward/backward lean angle"))
	float MaxYLeanAngle{20.f};

	// Scales blend space value by the current speed so that the character cannot lean as far when moving slowly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bScaleBySpeed{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="bScaleBySpeed", EditConditionHides))
	FVector2D SpeedRange{200.0f, 600.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="bScaleBySpeed", EditConditionHides))
	FVector2D ScaleRange{0.2f, 1.0f};

	virtual void Validate() override;
};


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_TurnInPlace : public FSigilAnimData
{
	GENERATED_BODY()

	virtual void Validate() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Left = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Right = nullptr;

	/**
	 * Optional left 180 turn. 可选的180左转动画
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Left180{nullptr};

	/**
	 * Optional right 180 turn. 可选的180右转动画
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UAnimSequence> Right180{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (DisplayName="Root Yaw Angle Threshold", ClampMin = 1, ClampMax = 180, ForceUnits = "deg"))
	float RootYawAngleThreshold{10};

	/**
	 * Yaw angle threshold for triggering a turn.
	 * 触发转身的偏航角阈值
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (DisplayName="View Yaw Angle Threshold", ClampMin = 10, ClampMax = 180, ForceUnits = "deg"))
	float ViewYawAngleThreshold{45.0f};

	/**
	 * The blend in time of turn animations.
	 * 转身动画的混入时间。
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GMS", Meta = (ClampMin = 0))
	float BlendTime{0.2f};

	/**
	 * Will select 180 turn animations when Yaw Angle Threshold >= this value.
	 * 当偏航角度阈值 >= 此值时，将选择 180 转身动画。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 10, ClampMax = 180, ForceUnits = "deg"))
	float Turn180AngleThreshold{130.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bScaleTurnRate{true};	

	/**
	 * The play rate of the turn-in-place animation.
	 * @attention Only used when not aiming.
	 * 延时激活的原地转身动画的播放速率。
	 * @注意 只在延迟激活时使用。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(DisplayName="PlayRate", ForceUnits = "x"))
	float PlayRate{1.0f};

	/**
	 * Maps the turn yaw angle (the angle at which the turn is triggered, in the range {Threshold,180} degrees) to the turn activation delay.
	 * @attention Set to zero to disable delay.
	 * 将转身偏航角（触发转身的角度，在{Threshold,180}度范围内） 映射为转身激活延迟。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (DisplayName="Turn Yaw Angle to Activation Delay", ClampMin = 0, ForceUnits="s"))
	FVector2D ViewYawAngleToActivationDelay{0.2f, 0.75f};

	/**
	 * If the character should turn, set the play rate to scale with the view yaw speed. This makes the character rotate faster when moving the camera faster.
	 * @attention Only used when no delayed activation, See PlayRateRange for more.
	 * 如果角色需要转动，则将播放速率设置为与视角偏航角速度成比例。这样当摄像机移动速度加快时，角色的旋转速度也会加快。
	 * @注意 只在没有延迟激活时使用，查看PlayRateRange了解更多。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(DisplayName="Reference View Yaw Speed", ForceUnits = "deg/s"))
	FVector2D ReferenceViewYawSpeed{180.0f, 460.0f};

	/**
	 * How quickly turn animations will play.
	 * @attention Only used when no delayed activation. Will dynamically adjust the play rate within this range based on ReferenceViewYawSpeed.
	 * 转身动画的播放速度。
	 * @注意 只在没有延迟激活时使用。将根据参考视图的偏航速度（ReferenceViewYawSpeed）在此范围内动态调整播放速度。* 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(ClampMin=0.5, ClampMax=10, ForceUnits = "x"))
	FVector2D PlayRateRange{1.0f, 1.5f};

	/**
	 * Controls the steering behavior of turn-in-place.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GMS")
	FSigilSteeringSettings Steering;

	/**
	 * When character in VelocityDirection rotation mode, should  turn to face TargetYaw? 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bTurnInVelocityDirection{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bTurnWhenAimingInViewDirection{true};
};

/***
 * Defines the grounded moving animations for movement state.
 * 定义了移动状态对应的地面移动动画
 */
USTRUCT(DisplayName="GMS AnimData Moving States")
struct SIGILMOVEMENT_API FSigilAnimData_MovingStates
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS", meta=(Categories="Sigil.Movement.State"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(DisplayName="Start(ViewDirection)", BaseStruct = "/Script/SigilMovement.SigilAnimData_Start_ViewDirection"))
	FInstancedStruct Start_ViewDir_Inst;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(DisplayName="Start(VelocityDirection)", BaseStruct = "/Script/SigilMovement.SigilAnimData_Start_VelocityDirection"))
	FInstancedStruct Start_VelocityDir_Inst;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(DisplayName="Cycle", BaseStruct = "/Script/SigilMovement.SigilAnimData_Cycle"))
	FInstancedStruct Cycle_Inst;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(DisplayName="Stop", BaseStruct = "/Script/SigilMovement.SigilAnimData_Stop"))
	FInstancedStruct Stop_Inst;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(DisplayName="Pivot", BaseStruct = "/Script/SigilMovement.SigilAnimData_Pivot"))
	FInstancedStruct Pivot_Inst;

	bool operator==(const FGameplayTag& Other) const
	{
		return Tag == Other;
	}

	bool operator!=(const FGameplayTag& Other) const
	{
		return Tag != Other;
	}
};


/**
 * Default(Lyra-like) states anim layer setting.
 * 默认(类Lyra)状态动画层设置实现。
 */
UCLASS(Blueprintable)
class SIGILMOVEMENT_API USigilAnimLayerSetting_States_Default : public USigilAnimLayerSetting_States
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Anim States", meta=(DisplayName="Idle/IdleBreaks", BaseStruct = "/Script/SigilMovement.SigilAnimData_Idle"))
	FInstancedStruct Idle_Inst;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Anim States", meta=(DisplayName="Jump/Fall", BaseStruct = "/Script/SigilMovement.SigilAnimData_Jump"))
	FInstancedStruct Jump_Inst;

	UPROPERTY(EditAnywhere, Category="Anim States", meta=(DisplayName="Land", BaseStruct = "/Script/SigilMovement.SigilAnimData_Land"))
	FInstancedStruct Land_Inst;

	UPROPERTY(EditAnywhere, Category="Anim States", meta=(DisplayName="Turn", BaseStruct = "/Script/SigilMovement.SigilAnimData_TurnInPlace"))
	FInstancedStruct Turn_Inst;

	UPROPERTY(EditAnywhere, Category="Anim States", meta=(DisplayName="Lean", BaseStruct = "/Script/SigilMovement.SigilAnimData_Lean"))
	FInstancedStruct Lean_Inst;

	UPROPERTY(EditAnywhere, Category="Anim States", meta = (TitleProperty="Tag"))
	TArray<FSigilAnimData_MovingStates> MovingStates;

	UPROPERTY(VisibleAnywhere, Category="Anim States", meta=(EditCondition=false, EditConditionHides))
	TMap<FGameplayTag, FSigilAnimData_MovingStates> AcceleratedMovingStates;

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};


#pragma endregion

/**
 * Native default(Lyra-like) states anim layer implementation.
 * @attention The purpose of this class is to put the core logic in the C++ layer to improve performance.
 * 原生默认(类Lyra)状态动画层实现。
 * @注意 此类的目的是将核心逻辑放在C++以提升性能。
 */
UCLASS(Abstract)
class SIGILMOVEMENT_API USigilAnimLayer_States_DefaultLocomotion : public USigilAnimLayer
{
	GENERATED_BODY()

	friend USigilAnimLayerSetting_States_Default;

#pragma region States

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	TObjectPtr<const USigilAnimLayerSetting_States_Default> Setting;

	UPROPERTY()
	FSigilAnimData_MovingStates AnimData_MovingStates;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FSigilOrientationWarpingSettings OWSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Idle AnimData_Idle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_TurnInPlace AnimData_TurnInPlace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Start_ViewDirection AnimData_Start_ViewDirection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Start_VelocityDirection AnimData_Start_VelocityDirection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Cycle AnimData_Cycle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Lean AnimData_GroundedLean;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Stop AnimData_Stop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Pivot AnimData_Pivot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Jump AnimData_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FSigilAnimData_Land AnimData_Land;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_Idle IdleState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_Start StartState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_Cycle CycleState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_Stop StopState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_Pivot PivotState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_TurnInPlace TurnInPlaceState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FSigilAnimState_IdleBreak IdleBreakState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bEnableLand{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bEnableStart{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bEnableStop{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bEnablePivot{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bEnableJump{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bEnableFall{false};

#pragma endregion States

public:
	USigilAnimLayer_States_DefaultLocomotion();

	virtual void NativeInitializeAnimation() override;

	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;

	virtual void NativePostEvaluateAnimation() override;

	virtual void ApplySetting_Implementation(const USigilAnimLayerSetting* NewSetting) override;
	virtual void ResetSetting_Implementation() override;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	bool IsMovingPerpendicularToInitialPivot() const;

	/**
	 * Check if velocity (where we're moving) is opposite to acceleration (where we want to be moving).
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	bool IsPivoting() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	bool IsStarting() const;

#pragma region Utility

public:
	ESigilMovementDirection SelectCardinalDirectionFromAngle(float Angle, float DeadZone, ESigilMovementDirection CurrentDirection, bool bUseCurrentDirection) const;

	ESigilMovementDirection_8Way SelectOctagonalDirectionFromAngle(float Angle, float DeadZone, ESigilMovementDirection_8Way CurrentDirection, bool bUseCurrentDirection) const;

	ESigilMovementDirection GetOppositeCardinalDirection(ESigilMovementDirection CurrentDirection) const;

	bool IsViewDirection() const;
	bool IsVelocityDirection() const;
	const FSigilViewDirectionSetting& GetViewDirectionSetting() const;
	const FSigilVelocityDirectionSetting& GetVelocityDirectionSetting() const;
#pragma endregion Utility

#pragma region Idle

protected:
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Idle_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void IdleBreak_AnimRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void InitializeIdleBreak();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void RefreshIdleBreak();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|IdleBreak", meta=(BlueprintThreadSafe))
	bool IsIdleBreakAllowed() const;

	// top level idle animation state node update method.
	UFUNCTION(BlueprintCallable, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Idle_StateUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

#pragma endregion

#pragma region Turn/Rotate

protected:
	virtual void SetupTurnInPlace(float TurnAngle);
	virtual void RefreshTurnInPlaceMode();
	virtual void RefreshTurnInPlaceInViewDirection();
	virtual void RefreshTurnInPlaceInVelocityDirection();
#pragma endregion

#pragma region Start

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	UAnimSequence* GetStartAnimSequence() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	const FSigilStrideWarpingSettings& GetStartStrideWarpingSettings() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	const FSigilSteeringSettings& GetStartSteeringSettings() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Start_AnimRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Start_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Start_StateEntry(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	// top level start animation state node update.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Start_StateUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

#pragma endregion

#pragma region Cycle

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Cycle_AnimRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Cycle_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Cycle_StateEntry(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Cycle_StateUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	UAnimSequence* GetCycleAnimation() const;

#pragma endregion

#pragma region Stop

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	UAnimSequence* GetStopAnimation() const;

	bool ShouldDistanceMatchStop() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Stop_AnimRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Stop_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Stop_StateRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Stop_StateUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);
	virtual void Stop_StateUpdate_Implementation(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	float GetDistanceToStopTarget() const;

#pragma endregion

#pragma region Pivot

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Pivot_AnimRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Pivot_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Pivot_StateRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Pivot_StateUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

#pragma endregion

#pragma region Jump

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void JumpStart_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void JumpStartLoop_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void JumpApex_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void FallLoop_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void FallLand_AnimRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void FallLand_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintPure, BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	bool Rule_JumpApex() const;

	UFUNCTION(BlueprintPure, BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	bool Rule_FallToFallLand() const;
#pragma endregion

#pragma region Land

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Land_AnimRelevant(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	void Land_AnimUpdate(UPARAM(ref) FAnimUpdateContext& Context,UPARAM(ref) FAnimNodeReference& Node);

#pragma endregion
};


