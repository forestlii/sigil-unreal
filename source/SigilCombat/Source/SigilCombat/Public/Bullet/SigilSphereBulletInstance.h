// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilBulletInstance.h"
#include "SigilSphereBulletInstance.generated.h"

class USphereComponent;

/**
 * Bullet instance with a spherical collision shape.
 * 具有球形碰撞形状的子弹实例。
 */
UCLASS(Abstract, Blueprintable)
class SIGILCOMBAT_API ASigilSphereBulletInstance : public ASigilBulletInstance
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	ASigilSphereBulletInstance();

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