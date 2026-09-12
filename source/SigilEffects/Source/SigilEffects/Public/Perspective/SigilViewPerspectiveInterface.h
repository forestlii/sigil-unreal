// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SigilViewPerspectiveInterface.generated.h"

/**
 * Interface for actors or components that know which camera perspective their owner is currently viewed from
 * (first person body vs. world body). Implement it on the pawn, or on a component of the pawn; anything attached in
 * the pawn's owner chain (weapons, equipment actors) is resolved through it.
 * 供 Actor / 组件汇报"当前从哪个视角观看自己"（第一人称身体 vs 世界身体）。实现在 Pawn 或 Pawn 的组件上；
 * 挂在 Pawn 拥有链上的对象（武器、装备 Actor）都会通过它解析。
 */
UINTERFACE(Blueprintable, MinimalAPI)
class USigilViewPerspectiveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implementation class for the view perspective interface.
 * 视角接口的实现类。
 */
class SIGILEFFECTS_API ISigilViewPerspectiveInterface
{
	GENERATED_BODY()

public:
	/**
	 * True while the owner is rendered from a first person perspective.
	 * 拥有者当前以第一人称视角渲染时返回 true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GES|Perspective")
	bool IsInFirstPersonPerspective() const;
	virtual bool IsInFirstPersonPerspective_Implementation() const { return false; }
};
