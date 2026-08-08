// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SigilTargetingSourceInterface.generated.h"

/**
 * Interface for objects providing targeting system information.
 * 为目标系统提供信息的对象的接口。
 * @note Used to provide additional targeting data.
 * @注意 用于提供额外的目标数据。
 */
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class USigilTargetingSourceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for targeting source objects.
 * 目标源对象的接口。
 */
class SIGILCOMBAT_API ISigilTargetingSourceInterface
{
	GENERATED_BODY()

public:
	/**
	 * Gets the trace level for dynamic tracing.
	 * 获取动态追踪的追踪级别。
	 * @return The trace level. 追踪级别。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Trace")
	float GetTraceLevel() const;
	virtual float GetTraceLevel_Implementation() const = 0;

	/**
	 * Gets the trace direction for targeting.
	 * 获取目标的追踪方向。
	 * @param OutDirection The trace direction (output). 追踪方向（输出）。
	 * @return True if a direction is provided. 如果提供方向返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Trace")
	bool GetTraceDirection(FVector& OutDirection) const;
	virtual bool GetTraceDirection_Implementation(FVector& OutDirection) const = 0;

	/**
	 * Gets the swept trace rotation for capsule or box traces.
	 * 获取胶囊或盒体追踪的旋转。
	 * @param OutRotation The rotation (output). 旋转（输出）。
	 * @return True if a rotation is provided. 如果提供旋转返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Trace")
	bool GetSweptTraceRotation(FRotator& OutRotation) const;
	virtual bool GetSweptTraceRotation_Implementation(FRotator& OutRotation) const = 0;

	/**
	 * Gets the shape component for trace properties.
	 * 获取追踪属性的形状组件。
	 * @param OutShape The shape component (output). 形状组件（输出）。
	 * @return True if a shape component is provided. 如果提供形状组件返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Trace")
	bool GetTraceShape(UShapeComponent*& OutShape) const;
	virtual bool GetTraceShape_Implementation(UShapeComponent*& OutShape) const = 0;

	/**
	 * Gets additional actors to ignore during tracing.
	 * 获取追踪期间忽略的额外Actor。
	 * @return Array of actors to ignore. 忽略的Actor数组。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Trace")
	TArray<AActor*> GetAdditionalActorsToIgnore() const;
	virtual TArray<AActor*> GetAdditionalActorsToIgnore_Implementation() const = 0;
};