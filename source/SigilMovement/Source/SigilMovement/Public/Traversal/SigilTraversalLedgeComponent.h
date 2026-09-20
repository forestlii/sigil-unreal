// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Traversal/SigilTraversableInterface.h"
#include "SigilTraversalLedgeComponent.generated.h"

class USplineComponent;

/**
 * Two splines on the owning actor that face each other across an obstacle: the near and far edge of a window sill, a wall top, a counter.
 * 所属 Actor 上隔着障碍物相对的两条样条：窗台、墙头、柜台的近侧与远侧边缘。
 */
USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilTraversalLedgePair
{
	GENERATED_BODY()

	/** Name of a spline component on the owner. 所属 Actor 上样条组件的名字。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FName SideA;

	/** The opposite edge. Leave empty for a ledge with nothing behind it (a platform edge). 对侧边缘；留空表示后面没有对边（平台边）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Traversal")
	FName SideB;
};

/**
 * Makes its owner traversable along authored splines instead of a box. Whichever spline is nearest to the character is the
 * front ledge, its partner is the back ledge. Can be switched off at runtime (a closed window).
 * 让所属 Actor 沿摆好的样条可攀，而不是按盒体积。离角色最近的那条是前边缘，与它配对的是后边缘。可在运行时关闭（关着的窗）。
 */
UCLASS(ClassGroup = (Sigil), meta = (BlueprintSpawnableComponent))
class SIGILMOVEMENT_API USigilTraversalLedgeComponent : public UActorComponent, public ISigilTraversableInterface
{
	GENERATED_BODY()

public:
	USigilTraversalLedgeComponent();

	virtual bool GetTraversalLedges_Implementation(
		const FVector& HitLocation,
		const FVector& InstigatorLocation,
		FSigilTraversalLedges& OutLedges) const override;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Traversal")
	void SetTraversalEnabled(bool bNewEnabled) { bTraversalEnabled = bNewEnabled; }

	UFUNCTION(BlueprintPure, Category = "Sigil|Traversal")
	bool IsTraversalEnabled() const { return bTraversalEnabled; }

	/** Replaces the authored pairs. 覆盖已配置的样条对。 */
	UFUNCTION(BlueprintCallable, Category = "Sigil|Traversal")
	void SetLedgePairs(const TArray<FSigilTraversalLedgePair>& NewPairs) { LedgePairs = NewPairs; }

	/**
	 * Ledges for two straight or curved edges given as world-space polylines. Pure, so the geometry can be unit tested without components.
	 * 由两条世界空间折线算边缘。纯函数，几何部分无需组件即可单测。
	 *
	 * @param Front          The edge nearer to the character. 离角色较近的边。
	 * @param Back           The opposite edge, may be empty. 对侧的边，可为空。
	 * @param MinLedgeWidth  The ledge point keeps at least half of this from either end of the edge. 边缘点距边的两端至少保持此值的一半。
	 */
	static bool ComputePolylineLedges(
		TConstArrayView<FVector> Front,
		TConstArrayView<FVector> Back,
		const FVector& HitLocation,
		const FVector& InstigatorLocation,
		float MinLedgeWidth,
		FSigilTraversalLedges& OutLedges);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Traversal")
	TArray<FSigilTraversalLedgePair> LedgePairs;

	/** Edges shorter than this cannot be traversed; the ledge point also stays half of it away from the ends. 比这短的边不能攀；边缘点距两端也保持此值的一半。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Traversal", meta = (ClampMin = "0.0"))
	float MinLedgeWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Traversal")
	bool bTraversalEnabled = true;

	/** How many points each spline is sampled into. Two is exact for straight edges. 每条样条采样成多少个点；直边两个点即精确。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Traversal", meta = (ClampMin = "2", ClampMax = "64"))
	int32 SamplesPerSpline = 8;

private:
	USplineComponent* FindSpline(FName ComponentName) const;
	void SampleSpline(const USplineComponent& Spline, TArray<FVector>& OutPoints) const;
};
