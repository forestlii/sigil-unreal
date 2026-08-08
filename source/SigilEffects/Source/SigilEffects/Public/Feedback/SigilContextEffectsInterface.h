// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Engine/HitResult.h"
#include "SigilContextEffectsStructLibrary.h"
#include "UObject/Interface.h"
#include "SigilContextEffectsInterface.generated.h"

class UAnimSequenceBase;
class UObject;
class USceneComponent;
struct FFrame;

/**
 * Interface for objects that can respond to context effects.
 * 可响应情景效果的对象接口。
 */
UINTERFACE(Blueprintable)
class SIGILEFFECTS_API USigilContextEffectsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implementation class for context effects interface.
 * 情景效果接口的实现类。
 */
class SIGILEFFECTS_API ISigilContextEffectsInterface : public IInterface
{
	GENERATED_BODY()

public:
	/**
	 * Plays context effects based on input parameters.
	 * 根据输入参数播放情景效果。
	 * @param Input The context effects input data. 情景效果输入数据。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GES|ContextEffect")
	void PlayContextEffectsWithInput(FSigilSpawnContextEffectsInput Input);
	virtual void PlayContextEffectsWithInput_Implementation(FSigilSpawnContextEffectsInput Input) = 0;
};