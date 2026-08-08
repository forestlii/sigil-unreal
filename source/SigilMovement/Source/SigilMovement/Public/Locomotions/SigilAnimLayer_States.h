// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAnimLayer.h"
#include "UObject/Object.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MINOR_VERSION > 4
#include "StructUtils/InstancedStruct.h"
#else
#include "InstancedStruct.h"
#endif
#include "Settings/SigilSettingObjectLibrary.h"
#include "SigilAnimLayer_States.generated.h"

//Anim data for anim state.
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData
{
	GENERATED_BODY()
	virtual ~FSigilAnimData() = default;

	virtual void Validate();

	/**
	 * Indicate if this data is valid.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GMS", meta=(EditCondition=false, EditConditionHides))
	bool bValid{false};

};

/**
 * Base class for all states anim layer setting.
 * @details You should inherit this class to create a new setting class for your Custom States Anim Layer.
 * 所有状态动画层的基类。
 * @细节 你应该继承此类来新建你的“自定义状态动画层”对应的设置类型。
 */
UCLASS(Abstract, Blueprintable)
class SIGILMOVEMENT_API USigilAnimLayerSetting_States : public USigilAnimLayerSetting
{
	GENERATED_BODY()
};

