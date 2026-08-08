// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GMS_SettingStructLibrary.h"
#include "UObject/Object.h"
#include "Engine/DataAsset.h"
#include "Utility/GMS_Tags.h"
#include "GMS_SettingObjectLibrary.generated.h"


#pragma region CommonSettings

class UGMS_AnimLayer;
class UGMS_AnimLayerSetting;

/**
 * MovementDefinition contains many movement set settings.
 * 运动定义包含多个运动集设置。
 */
UCLASS(BlueprintType, Const)
class GENERICMOVEMENTSYSTEM_API UGMS_MovementDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Available Movement set settings.
	 * 可用的运动集设置。
	 */
	UPROPERTY(EditAnywhere, Category="GMS", meta=(ForceInlineRow))
	TMap<FGameplayTag, FGMS_MovementSetSetting> MovementSets;

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};

/**
 * Object hold anim graph specific setting. Each unique skeleton should have one.
 * 该对象包含动画图表特定的设置。每一个独一无二的骨架应该拥有一个对应的动画图表设置。
 */
UCLASS()
class GENERICMOVEMENTSYSTEM_API UGMS_AnimGraphSetting : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Defines the relationship of anim layer settings and anim layers.
	 * @attention If you are creating your own anim layer settings/implementations, you should add to this mapping.
	 * 定义了动画层设置与动画层实现之间的映射关系。
	 * @注意 如果你创建了自己的的动画层设置/实现，你应该添加到这里进行映射。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	TMap<TSubclassOf<UGMS_AnimLayerSetting>, TSubclassOf<UGMS_AnimLayer>> AnimLayerSettingToInstanceMapping;

	/**
	 * Settings for Orientation Warping.
	 * 朝向扭曲相关的设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FGMS_OrientationWarpingSettings OrientationWarping;

	/**
	 * Deprecated since GMS v1.3, you can now adjust stride warping in each movement set settings.
	 * 自GMS v1.3起弃用，你现在可以在每一个运动集设置中对步幅适配进行配置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings",meta=(EditCondition=false, DisplayName="StrideWarping(Deprecated)"))
	FGMS_StrideWarpingSettings StrideWarpingWarping;
};

#pragma endregion

#pragma region ControlSettings
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJumpStatesUpdated, const TArray<FGMS_JumpStateSetting>&, UpdatedJumpStates);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStatesUpdated, const TArray<FGMS_MovementStateSetting>&, UpdatedMovementStates);
/**
 * Movement Control setting defines logic part of movement set setting.
 * 运动控制设置顶一个运动集设置的逻辑部分。
 */
UCLASS(BlueprintType, Blueprintable, Const, DisplayName="GMS Movement Control Setting")
class GENERICMOVEMENTSYSTEM_API UGMS_MovementControlSetting_Default : public UDataAsset
{
	GENERATED_BODY()

public:
	FGameplayTag MatchStateTagBySpeed(float Speed, float Threshold) const;

	bool GetStateByIndex(const int32& Index, FGMS_MovementStateSetting& OutSetting) const;

	bool GetStateBySpeedLevel(const int32& Level, FGMS_MovementStateSetting& OutSetting) const;

	bool GetStateByTag(const FGameplayTag& Tag, FGMS_MovementStateSetting& OutSetting) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Control", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSpeedThreshold{50.0f};
	/**
	 * Camera Look Speed Scale , Default Value = 1.f.
	 * 摄像机移动速率, 分X, Y轴, 默认是1
	 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Control")
	FVector2D LookSpeed ;
	/**
	 * Defines all movement states available in this movement control setting, this array should sort by speed.
	 * 定义了该运动控制所允许的所有运动状态设置，此数组应该依据速度从小到大进行排序。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GroundedControl", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FGMS_MovementStateSetting> MovementStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AirControl")
	EGMS_InAirRotationMode InAirRotationMode{EGMS_InAirRotationMode::KeepRelativeRotation};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AirControl", meta=(ClampMin=0))
	float InAirRotationInterpolationSpeed = 5.0f;

	/**
	 * Defines all movement states available in this movement control setting.
	 * 定义了该运动控制所允许的所有状态跳跃设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AirControl", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FGMS_JumpStateSetting> JumpStates;
	UPROPERTY(EditAnywhere, Category="Settings", meta=(EditCondition=false, ForceInlineRow))
	TMap<FGameplayTag, int32> TagToArrayIndex;

	UPROPERTY(EditAnywhere, Category="Settings", meta=(EditCondition=false, ForceInlineRow))
	TMap<int32, int32> SpeedLevelToArrayIndex;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnJumpStatesUpdated OnJumpStatesUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMovementStatesUpdated OnMovementStatesUpdated;

	UFUNCTION(BlueprintCallable, Category = "AirControl")
	void BroadcastJumpStates(const TArray<FGMS_JumpStateSetting>& NewStates)
	{
		OnJumpStatesUpdated.Broadcast(NewStates);
	}
	UFUNCTION(BlueprintCallable, Category = "MovementControl")
	void BroadcastMovementStates(const TArray<FGMS_MovementStateSetting>& NewStates)
	{
		OnMovementStatesUpdated.Broadcast(NewStates);
	}
#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};

#pragma endregion

#pragma region AnimSettings

/**
 * MovementSetUserSetting, you can subclass from this class to add custom settings to existing movement set settings without modify movement set setting itself.
 * 您可以子类化该类，为现有的运动集设置添加自定义设置，而无需修改运动集设置本身。
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const, EditInlineNew, DefaultToInstanced)
class GENERICMOVEMENTSYSTEM_API UGMS_MovementSetUserSetting : public UObject
{
	GENERATED_BODY()
};
#pragma endregion
