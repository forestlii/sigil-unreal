// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Traversal/SigilTraversalTypes.h"
#include "UObject/Interface.h"
#include "SigilTraversableInterface.generated.h"

UINTERFACE(BlueprintType)
class SIGILMOVEMENT_API USigilTraversableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by an actor or a component that can be climbed, vaulted or hurdled.
 * The traversal check never guesses at geometry: it asks the obstacle where its edges are.
 * 由可被攀爬、翻越或跨过的 Actor 或组件实现。攀爬检测不去猜几何，而是问障碍物它的边缘在哪。
 */
class SIGILMOVEMENT_API ISigilTraversableInterface
{
	GENERATED_BODY()

public:
	/**
	 * @param HitLocation        Where the forward probe touched the obstacle. 前探检测碰到障碍物的位置。
	 * @param InstigatorLocation Where the character stands. 角色所在位置。
	 * @param OutLedges          The edges facing the character. 面向角色的边缘。
	 * @return false when nothing here can be traversed from that side. 从该侧无可攀之处时返回假。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sigil|Traversal")
	bool GetTraversalLedges(
		const FVector& HitLocation,
		const FVector& InstigatorLocation,
		FSigilTraversalLedges& OutLedges) const;
};
