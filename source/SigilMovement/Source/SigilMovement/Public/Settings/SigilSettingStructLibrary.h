// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilSettingEnumLibrary.h"
#include "Animation/AnimData/BoneMaskFilter.h"
#include "UObject/Object.h"
#include "Curves/CurveFloat.h"
#include "Engine/EngineTypes.h"
#include "BoneControllers/BoneControllerTypes.h"
#include "Utility/SigilMovementTags.h"
#include "SigilSettingStructLibrary.generated.h"


class USigilMovementSetUserSetting;
class USigilAnimLayerSetting_SkeletalControls;
class USigilMovementControlSetting_Default;
class USigilMovementControlSetting;
class USigilAnimLayerSetting_Additive;
class USigilCharacterMovementSetting;
class USigilCharacterRotationSetting;
class USigilAnimLayerSetting_View;
class USigilAnimLayerSetting_Overlay;
class USigilAnimLayerSetting_StateOverlays;
class USigilAnimLayerSetting_States;

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilOrientationWarpingBoneReference
{
	GENERATED_USTRUCT_BODY()

	// Spine bone definitions
	// Used to counter rotate the body in order to keep the character facing forward
	// The amount of counter rotation applied is driven by DistributedBoneOrientationAlpha
	UPROPERTY(EditAnywhere, Category="GMS")
	TArray<FBoneReference> SpineBones;

	// IK Foot Root Bone definition
	UPROPERTY(EditAnywhere, Category="GMS")
	FBoneReference IKFootRootBone;

	// IK Foot definitions
	UPROPERTY(EditAnywhere, Category="GMS")
	TArray<FBoneReference> IKFootBones;
};


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilOrientationWarpingSettings
{
	GENERATED_BODY()

public:
	/**
	 * @brief Orientation warping evaluation mode (Graph or Manual)
	 * @attention If your animation has no root motion, use Manual mode.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	EWarpingEvaluationMode Mode = EWarpingEvaluationMode::Graph;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GMS")
	FSigilOrientationWarpingBoneReference BoneReference;

	/**
	 * Specifies how much rotation is applied to the character body versus IK feet
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GMS", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DistributedBoneOrientationAlpha = 0.75f;

	// Specifies the interpolation speed (in Alpha per second) towards reaching the final warped rotation angle
	// A value of 0 will cause instantaneous rotation, while a greater value will introduce smoothing
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GMS", meta=(ClampMin="0.0"))
	float RotationInterpSpeed = 10.f;
};


USTRUCT()
struct SIGILMOVEMENT_API FSigilStrideWarpingBoneReference
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS")
	FBoneReference PelvisBone;

	UPROPERTY(EditAnywhere, Category="GMS")
	FBoneReference IKFootRootBone;
};

USTRUCT()
struct SIGILMOVEMENT_API FSigilStrideWarpingFootDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="GMS")
	FBoneReference IKFootBone;

	UPROPERTY(EditAnywhere, Category="GMS")
	FBoneReference FKFootBone;

	UPROPERTY(EditAnywhere, Category="GMS")
	FBoneReference ThighBone;
};


/**
 * Settings about StrideWarping node in anim graph.
 * 动画图表中的步幅适配设置。
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilStrideWarpingSettings
{
	GENERATED_BODY()

public:
	/**
	 * If stride warping enabled?
	 * @attention For non-humanoid character, you should disable this.
	 * 是否启用步幅适配？
	 * @注意 对于非人形角色，你应该禁用它。
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GMS")
	bool bEnabled{true};

	/**
	 * Start offset of blending in.
	 * @attention For animations with turns, you should set this time to the moment the character starts moving in a straight line.
	 * 混入开始时间。
	 * @注意 对于带有转身的动画，你应该将此时间设置为角色开始直线运动的时刻。
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GMS", meta=(ClampMin=0))
	float BlendInStartOffset{0.15f};

	/**
	 * How long it would take to fully blending in.
	 * 需要多长时间步幅适配Alpha变成1.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GMS")
	float BlendInDurationScaled{0.2f};
};

/**
 * Settings about Steering node in anim graph.
 * 动画图表中的Steering设置。
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilSteeringSettings
{
	GENERATED_BODY()

public:

	FSigilSteeringSettings()
	{
		bEnabled = true;
		ProceduralTargetTime = 0.2f;
		AnimatedTargetTime = 0.5f;
	}
	
	FSigilSteeringSettings(bool bInEnabled, float InProceduralTargetTime, float InAnimatedTargetTime)
	{
		bEnabled = bInEnabled;
		ProceduralTargetTime = InProceduralTargetTime;
		AnimatedTargetTime = InAnimatedTargetTime;
	};
	
	/**
	 * If steering enabled?
	 * 是否启用Steering？
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GMS")
	bool bEnabled{true};

	/**
	 * The number of seconds in the future before we should reach the TargetOrientation when play animations with no root motion rotation
	 * @attention Set large value to disable procedural turn.
	 * 在播放没有根运动旋转的动画时，预计几秒内到达目标朝向。
	 * @注意 填写一个较大值，以禁用程序化转身。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float ProceduralTargetTime = 0.2f;

	/**
	 * The number of seconds in the future before we should reach the TargetOrientation when playing animations with root motion rotation
	 * 播放根运动旋转动画时，预计几秒内到达目标朝向。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float AnimatedTargetTime = 0.5f;
};


//A wrapper of branch filters for bp usage.
USTRUCT(BlueprintType)
struct FSigilInputBlendPose
{
	GENERATED_USTRUCT_BODY()

	/** Bone Name to filter **/
	UPROPERTY(EditAnywhere, Category=Filter)
	TArray<FBranchFilter> BranchFilters;
};

//Common setting for main anim instance.
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimDataSetting_General
{
	GENERATED_BODY()

	/**
	 * If you enable this, the character’s root bone is allowed to rotate independently of the actor rotation.
	 * @attention 这是一个全局设置，不同的动画层应该遵守这一设置，即在此选项开启的前提下，才去修改RootOffsetState.
	 * @细节 默认Locomotion会在不同状态下修改RootBoneRotationMode，这让实现复杂的转身逻辑更加容易。
	 * @details The default locomotion will use so that the character rotation is entirely controlled with root motion, which gets steered within the motion matching node to always align toward the target. This makes it easier to perform more complex rotational animations like turn in places, turn starts, and turn pivots.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common")
	bool bEnableOffsetRootBoneRotation{true};

	/**
	 * Control how fast the root bone rotation offset is blended out, Value closer to 0 make it faster, =0 mean instantly blend out.
	 * 控制根骨旋转偏移的混出速度，数值越接近 0 速度越快，=0 表示立即混出。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common", Meta = (ClampMin = 0, ClampMax = 1))
	float RootBoneRotationHalfLife{0.2};

	//Controls the speed of lean on grounded or in air state.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common", Meta = (ClampMin = 0))
	float LeanInterpolationSpeed{4.0f};

	// moving threshold used in animation state.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSmoothSpeedThreshold{150.0f};

	// Vertical velocity to lean amount curve.If empty, will not refresh lean state when in air state.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InAir")
	TObjectPtr<UCurveFloat> InAirLeanAmountCurve{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InAir")
	TEnumAsByte<ECollisionChannel> GroundPredictionSweepChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InAir")
	TArray<TEnumAsByte<ECollisionChannel>> GroundPredictionResponseChannels;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="InAir")
	FCollisionResponseContainer GroundPredictionSweepResponses{ECR_Ignore};

public:
#if WITH_EDITOR
	void PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent);
#endif
};

/**
 * Settings for rotation mode:ViewDirection
 */
USTRUCT(BlueprintType)
struct FSigilViewDirectionSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilViewDirectionMode DirectionMode{ESigilViewDirectionMode::Default};

	/**
	 * If enabled, the default Locomotion's Turn-In-Place animation tries to catch up with the Actor rotation via the Steering node.
	 * If disabled, the default Locomotion's Turn-In-Place animation will drive actor rotation to target rotation.
	 * 如果启用，默认Locomotion的原地转身动画会跟上Actor旋转。
	 * 如果禁用，默认Locomotion的原地转身动画会驱动Actor旋转。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bRotateToViewDirectionWhileNotMoving{true};

	/**
	 * The angle limit between character orientation and camera orientation ensures that the character's rotation can quickly keep up with the camera rotation when the camera is moving too fast.
	 * 角色朝向与相机朝向之间的夹角限制，当镜头移动速度过快时，确保角色的旋转能快速跟上镜头旋转。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="DirectionMode==ESigilViewDirectionMode::Aiming", EditConditionHides))
	float MinAimingYawAngleLimit{90.0f};
};

/**
 * Settings for rotation mode:VelocityDirection
 */
USTRUCT(BlueprintType)
struct FSigilVelocityDirectionSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	ESigilVelocityDirectionMode DirectionMode{ESigilVelocityDirectionMode::OrientToLastVelocityDirection};

	/**
	 * Allow rotation when not moving?
	 * 没移动时允许旋转吗？
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bEnableRotationWhenNotMoving{true};

	// angle/s
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="DirectionMode==ESigilVelocityDirectionMode::TurningCircle", EditConditionHides))
	float TurningRate{30};
};

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilInAirSetting
{
	GENERATED_BODY()

public:
	// Vertical velocity to lean amount curve.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TObjectPtr<UCurveFloat> LeanAmountCurve{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TEnumAsByte<ECollisionChannel> GroundPredictionSweepChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TArray<TEnumAsByte<ECollisionChannel>> GroundPredictionResponseChannels;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="GMS")
	FCollisionResponseContainer GroundPredictionSweepResponses{ECR_Ignore};
};

/**
 * Settings for a single movement state(walk,jog,sprint etc.)
 * 单一运动状态的设置，如走跑疾跑等。
 */
USTRUCT(BlueprintType)
struct FSigilMovementStateSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(Categories="Sigil.Movement.State"))
	FGameplayTag Tag;

	/**
	 * Describe the level of speed. <0 means reverse movement.
	 * 描述速度的等级，<0意味着逆行速度(倒车)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	int32 SpeedLevel = INDEX_NONE;

	/**
	 * The Forward Speed of this movement state.
	 * 此运动状态前向的速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed{375.0f};
	/**
	 * The Strafe Speed of this movement state.
	 * 此运动状态侧向的速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float StrafeSpeed{300.0f};
	/**
	 * The Backwards Speed of this movement state.
	 * 此运动状态倒退的速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float BackwardsSpeed{225.0f};
	/**
	 * The acceleration of this movement state.
	 * 此运动状态的加速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float Acceleration = 800.f;

	/**
	 * The deceleration of this movement state.
	 * 此运动状态的减速度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float BrakingDeceleration = 1024.f;

	/**
	 * Use to limit available rotation modes.
	 * 可用用来限制此运动状态下允许的旋转模式。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	TArray<FGameplayTag> AllowedRotationModes{SigilRotationModeTags::VelocityDirection, SigilRotationModeTags::ViewDirection};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="bVelocityDirection"))
	FSigilVelocityDirectionSetting VelocityDirectionSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(EditCondition="bViewDirection"))
	FSigilViewDirectionSetting ViewDirectionSetting;

	/**
	 * Controls the speed at which the character's current yaw angle is smoothed to "SmoothTargetYawAngle", <=0 is equal to turning off smoothing.
	 * 用于控制角色的当前偏航角平滑过渡到 “SmoothTargetYawAngle”的速度，<=0 等于关闭平滑。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(ClampMin=0))
	float RotationInterpolationSpeed = 12.0f;

	/**
	 * Used to control the speed at which SmoothTargetYawAngle smooths the transition to TargetYawAngle. <=0 means turnoff smoothing.
	 * 用于控制SmoothTargetYawAngle平滑过渡到TargetYawAngle的速度。 <=0 意味着没有平滑，瞬间到达。
	 **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(ClampMin=0))
	float TargetYawAngleRotationSpeed = 800.0f;

	friend bool operator==(const FSigilMovementStateSetting& Lhs, const FSigilMovementStateSetting& RHS)
	{
		return Lhs.Tag == RHS.Tag;
	}

	friend bool operator!=(const FSigilMovementStateSetting& Lhs, const FSigilMovementStateSetting& RHS)
	{
		return !(Lhs == RHS);
	}

	bool operator==(const FGameplayTag& Other) const
	{
		return Tag == Other;
	}

	bool operator!=(const FGameplayTag& Other) const
	{
		return Tag != Other;
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="Settings", meta=(EditCondition=false, EditConditionHides))
	FString EditorFriendlyName;

	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition=false, EditConditionHides))
	bool bViewDirection{false};

	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition=false, EditConditionHides))
	bool bVelocityDirection{false};
#endif
};
 /*
 Settings for a Jump movement state(walk,jog,sprint etc.)
 跳跃状态的设置，如走跑疾跑等。
 */
 
USTRUCT(BlueprintType)
struct FSigilJumpStateSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FGameplayTag Tag;

	/**
	 * Describe the level of speed. <0 means reverse movement.
	 * 描述速度的等级，<0意味着逆行速度(倒车)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	int32 SpeedLevel = INDEX_NONE;

	/**
	 * The speed of this Jump state.
	 * 此跳跃状态的速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed{375.0f};
	/**
	* The GravityScale of this Jump state(1.0 = Default Gravity(980 cm/s²).
	* 此跳跃状态的重力加速度(1.0 = 默认重力 (980 cm/s²))
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float GravityScale{1.0f};
	/**
	* The GravityScale of this Jump state(1.0 = Default Gravity(980 cm/s²).
	* 此跳跃状态的空中控制力 (0.0-1.0)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float AirControl {0.5f};
	/**
	* The AirControlBoostMultiplier of this Jump state.
	* 此跳跃状态的最大空中速度 (默认=10)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float AirControlBoostMultiplier{10.0f};
	/**
	* The AirControlBoostVelocityThreshold of this Jump state(1.0 = Default Gravity(980 cm/s²).
	* 此跳跃状态的最大空中加速度 (默认=2048)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float AirControlBoostVelocityThreshold {2048.0f};
	/**
	* The MaxFlySpeed  of this Jump state(1.0 = Default Gravity(980 cm/s²).
	* 此跳跃状态的最大下落速度 (默认=800)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float MaxFlySpeed {800.0f};
	/**
	* The BrakingDecelerationFalling   of this Jump state(1.0 = Default Gravity(980 cm/s²).
	* 此跳跃状态的控制下落时的减速加速度(默认=2048)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float BrakingDecelerationFalling  {2048.0f};

	bool operator==(const FGameplayTag& Other) const
	{
		return Tag == Other;
	}

	bool operator!=(const FGameplayTag& Other) const
	{
		return Tag != Other;
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="Settings", meta=(EditCondition=false, EditConditionHides))
	FString EditorFriendlyName;
#endif
};
/**
 * MovementSetSetting defines control and animation settings for a movement set.
 * 运动集设置控制定义了一个运动集的控制和动画设置。
 */
USTRUCT(BlueprintType)
struct FSigilMovementSetSetting
{
	GENERATED_BODY()

	/**
	 * Default control setting.
	 * 默认的控制设置
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Control")
	TObjectPtr<USigilMovementControlSetting_Default> ControlSetting;

	/**
	 * For some user heavily use overlay, You may want different overlay mode has different control setting, this is option for you.
	 * @attention If new overlay mode -> control setting not found, default control setting will be used.
	 * 对于叠层系统的重度用户，你可能需要不同的叠层有不同的运动控制设置，那么就可以利用这个选项。
	 * @注意 如果新的叠层模式没有对应的控制设置，那么会使用默认的控制设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Control")
	bool bControlSettingPerOverlayMode{false};

	/**
	 * Override control settings for different overlay mode.
	 * 叠层模式与控制设置的映射。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Control", meta=(Categories="Sigil.Movement.OverlayMode", EditCondition="bControlSettingPerOverlayMode"))
	TMap<FGameplayTag, TObjectPtr<USigilMovementControlSetting_Default>> ControlSettings;

	/**
	 * Shared anim settings.
	 * 共享的动画设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	FSigilAnimDataSetting_General AnimDataSetting_General;

	/**
	 * If you have multiple movement set settings share same states layer setting, then you should uncheck this.
	 * 如果你有多个运动集设置共享同样的状态层，那么你不要勾选此选项。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	bool bUseInstancedStatesSetting{true};

	/**
	 * Settings for states anim layer.
	 * 状态动画层的设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="Animation", meta = (EditCondition="bUseInstancedStatesSetting", EditConditionHides, DisplayName="Anim Layer Setting (States)"))
	TObjectPtr<USigilAnimLayerSetting_States> AnimLayerSetting_States;

	/**
	 * Settings for states anim layer.
	 * 状态动画层的设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation",
		meta = (EditCondition="!bUseInstancedStatesSetting", EditConditionHides, DisplayName="Anim Layer Setting (States)"))
	TObjectPtr<const USigilAnimLayerSetting_States> DA_AnimLayerSetting_States;

	/**
	 * If you have multiple movement set settings shares same overlay layer setting, then you uncheck this.
	 * 如果你有多个运动集设置共享同样的叠层设置，那么你不要勾选此选项。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	bool bUseInstancedOverlaySetting{true};

	/**
	 * Settings for overlay anim layer.
	 * 叠层动画层的设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation",
		meta = (EditCondition="!bUseInstancedOverlaySetting", EditConditionHides, DisplayName="Anim Layer Setting (Overlay)"))
	TObjectPtr<const USigilAnimLayerSetting_Overlay> DA_AnimLayerSetting_Overlay;

	/**
	 * Settings for overlay anim layer.
	 * 叠层动画层的设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="Animation",
		meta = (EditCondition="bUseInstancedOverlaySetting", EditConditionHides, DisplayName="Anim Layer Setting (Overlay)"))
	TObjectPtr<USigilAnimLayerSetting_Overlay> AnimLayerSetting_Overlay;

	/**
	 * Settings for view anim layer.
	 * View动画层的设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="Animation", meta = (DisplayName="Anim Layer Setting (View)"))
	TObjectPtr<USigilAnimLayerSetting_View> AnimLayerSetting_View;

	/**
	 * Settings for additive anim layer.
	 * 附加动画层的设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="Animation", meta = (DisplayName="Anim Layer Setting (Additive)"))
	TObjectPtr<USigilAnimLayerSetting_Additive> AnimLayerSetting_Additive;

	/**
	 * Settings for skeletal controls anim layer.
	 * 骨骼控制动画层的设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="Animation", meta = (DisplayName="Anim Layer Setting (SkeletalControls)"))
	TObjectPtr<USigilAnimLayerSetting_SkeletalControls> AnimLayerSetting_SkeletalControls;

	/**
	 * You can subclass from SigilMovementSetUserSetting to create different custom setting.
	 * @attention You should consume these settings from your anim layer instance(anim blueprint). " GetMovementSetUserSetting"
	 * @example GetMovementSystemComponent()->GetMovementSetSetting()->GetMovementSetUserSetting(SettingClass);
	 * 你可以通过集成SigilMovementSetUserSetting来创建自定义设置。
	 * @attention 你应该在你的动画层实例中使用这些数据.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="Extension", meta=(AllowAbstract=false))
	TArray<TObjectPtr<USigilMovementSetUserSetting>> UserSettings;
};
