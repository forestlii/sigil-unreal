// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GCS_BulletInstance.h"
#include "GCS_SphereBulletInstance.generated.h"

class USphereComponent;

/**
 * Bullet instance with a spherical collision shape.
 * 具有球形碰撞形状的子弹实例。
 */
UCLASS(Abstract, Blueprintable)
class GENERICCOMBATSYSTEM_API AGCS_SphereBulletInstance : public AGCS_BulletInstance
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	AGCS_SphereBulletInstance();

	/**
	 * Gets the bullet's shape component.
	 * 获取子弹的形状组件。
	 * @return The sphere component. 球形组件。
	 */
	virtual UShapeComponent* GetBulletShape_Implementation() const override;

protected:
	/**
	 * Called when the game starts or when spawned.
	 * 游戏开始或生成时调用。
	 */
	virtual void BeginPlay() override;

	/**
	 * The sphere component for collision detection.
	 * 用于碰撞检测的球形组件。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GCS")
	TObjectPtr<USphereComponent> Sphere;

public:
	/**
	 * Called every frame.
	 * 每帧调用。
	 * @param DeltaTime Time since last frame. 上一帧以来的时间。
	 */
	virtual void Tick(float DeltaTime) override;
};