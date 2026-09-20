// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Traversal/SigilTraversalLedgeComponent.h"

#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"

namespace SigilTraversalLedgePrivate
{
	double PolylineLength(const TConstArrayView<FVector> Points)
	{
		double Length = 0.0;
		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			Length += FVector::Dist(Points[Index - 1], Points[Index]);
		}
		return Length;
	}

	/** Distance along the polyline of the point nearest to Target. 折线上离 Target 最近的点所在的路程。 */
	double ClosestDistanceAlong(const TConstArrayView<FVector> Points, const FVector& Target, double& OutDistanceSquared)
	{
		double BestAlong = 0.0;
		OutDistanceSquared = Points.IsEmpty() ? TNumericLimits<double>::Max() : FVector::DistSquared(Points[0], Target);
		double Walked = 0.0;
		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			const FVector Closest = FMath::ClosestPointOnSegment(Target, Points[Index - 1], Points[Index]);
			const double DistanceSquared = FVector::DistSquared(Closest, Target);
			if (DistanceSquared < OutDistanceSquared)
			{
				OutDistanceSquared = DistanceSquared;
				BestAlong = Walked + FVector::Dist(Points[Index - 1], Closest);
			}
			Walked += FVector::Dist(Points[Index - 1], Points[Index]);
		}
		return BestAlong;
	}

	FVector PointAt(const TConstArrayView<FVector> Points, double Along, FVector& OutTangent)
	{
		OutTangent = FVector::ZeroVector;
		if (Points.IsEmpty())
		{
			return FVector::ZeroVector;
		}
		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			const double Segment = FVector::Dist(Points[Index - 1], Points[Index]);
			if (Segment <= UE_DOUBLE_SMALL_NUMBER)
			{
				continue;
			}
			OutTangent = (Points[Index] - Points[Index - 1]) / Segment;
			if (Along <= Segment)
			{
				return Points[Index - 1] + OutTangent * FMath::Max(Along, 0.0);
			}
			Along -= Segment;
		}
		return Points.Last();
	}

	/** The point on the polyline nearest to Target, kept half of MinLedgeWidth away from both ends. 折线上离 Target 最近、且距两端不少于 MinLedgeWidth 一半的点。 */
	bool LedgePoint(const TConstArrayView<FVector> Points, const FVector& Target, const float MinLedgeWidth, FVector& OutPoint, FVector& OutTangent)
	{
		const double Length = PolylineLength(Points);
		if (Points.Num() < 2 || Length <= UE_DOUBLE_SMALL_NUMBER || Length < MinLedgeWidth)
		{
			return false;
		}
		double DistanceSquared = 0.0;
		const double Margin = MinLedgeWidth * 0.5;
		const double Along = FMath::Clamp(ClosestDistanceAlong(Points, Target, DistanceSquared), Margin, Length - Margin);
		OutPoint = PointAt(Points, Along, OutTangent);
		return true;
	}
}

USigilTraversalLedgeComponent::USigilTraversalLedgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool USigilTraversalLedgeComponent::ComputePolylineLedges(
	const TConstArrayView<FVector> Front,
	const TConstArrayView<FVector> Back,
	const FVector& HitLocation,
	const FVector& InstigatorLocation,
	const float MinLedgeWidth,
	FSigilTraversalLedges& OutLedges)
{
	using namespace SigilTraversalLedgePrivate;

	OutLedges = FSigilTraversalLedges();
	FVector FrontPoint;
	FVector FrontTangent;
	if (!LedgePoint(Front, HitLocation, MinLedgeWidth, FrontPoint, FrontTangent))
	{
		return false;
	}

	FVector BackPoint = FVector::ZeroVector;
	FVector BackTangent;
	const bool bHasBack = LedgePoint(Back, FrontPoint, MinLedgeWidth, BackPoint, BackTangent);

	// The normal points away from the far edge. With no far edge, or one straight below, it is the side of the edge the character is on.
	// 法线指向远离对边的方向；没有对边、或对边就在正下方时，取角色所在的那一侧。
	FVector Normal = bHasBack ? (FrontPoint - BackPoint).GetSafeNormal2D() : FVector::ZeroVector;
	if (Normal.IsNearlyZero())
	{
		const FVector Side = FVector::CrossProduct(FrontTangent, FVector::UpVector).GetSafeNormal2D();
		Normal = FVector::DotProduct(Side, InstigatorLocation - FrontPoint) >= 0.0 ? Side : -Side;
	}
	if (Normal.IsNearlyZero())
	{
		return false;
	}

	OutLedges.bHasFrontLedge = true;
	OutLedges.FrontLedgeLocation = FrontPoint;
	OutLedges.FrontLedgeNormal = Normal;
	OutLedges.bHasBackLedge = bHasBack;
	OutLedges.BackLedgeLocation = BackPoint;
	OutLedges.BackLedgeNormal = bHasBack ? -Normal : FVector::ZeroVector;
	return true;
}

bool USigilTraversalLedgeComponent::GetTraversalLedges_Implementation(
	const FVector& HitLocation,
	const FVector& InstigatorLocation,
	FSigilTraversalLedges& OutLedges) const
{
	using namespace SigilTraversalLedgePrivate;

	OutLedges = FSigilTraversalLedges();
	if (!bTraversalEnabled)
	{
		return false;
	}

	// Whichever spline is nearest to the character is the front ledge. 离角色最近的那条样条是前边缘。
	TArray<FVector> BestFront;
	TArray<FVector> BestBack;
	double BestDistanceSquared = TNumericLimits<double>::Max();
	TArray<FVector> PointsA;
	TArray<FVector> PointsB;
	for (const FSigilTraversalLedgePair& Pair : LedgePairs)
	{
		const USplineComponent* SplineA = FindSpline(Pair.SideA);
		const USplineComponent* SplineB = FindSpline(Pair.SideB);
		PointsA.Reset();
		PointsB.Reset();
		if (SplineA)
		{
			SampleSpline(*SplineA, PointsA);
		}
		if (SplineB)
		{
			SampleSpline(*SplineB, PointsB);
		}

		double DistanceSquared = 0.0;
		if (PointsA.Num() >= 2)
		{
			ClosestDistanceAlong(PointsA, InstigatorLocation, DistanceSquared);
			if (DistanceSquared < BestDistanceSquared)
			{
				BestDistanceSquared = DistanceSquared;
				BestFront = PointsA;
				BestBack = PointsB;
			}
		}
		if (PointsB.Num() >= 2)
		{
			ClosestDistanceAlong(PointsB, InstigatorLocation, DistanceSquared);
			if (DistanceSquared < BestDistanceSquared)
			{
				BestDistanceSquared = DistanceSquared;
				BestFront = PointsB;
				BestBack = PointsA;
			}
		}
	}

	return BestFront.Num() >= 2 &&
		ComputePolylineLedges(BestFront, BestBack, HitLocation, InstigatorLocation, MinLedgeWidth, OutLedges);
}

USplineComponent* USigilTraversalLedgeComponent::FindSpline(const FName ComponentName) const
{
	const AActor* Owner = GetOwner();
	if (!Owner || ComponentName.IsNone())
	{
		return nullptr;
	}
	for (UActorComponent* Component : Owner->GetComponents())
	{
		if (USplineComponent* Spline = Cast<USplineComponent>(Component); Spline && Spline->GetFName() == ComponentName)
		{
			return Spline;
		}
	}
	return nullptr;
}

void USigilTraversalLedgeComponent::SampleSpline(const USplineComponent& Spline, TArray<FVector>& OutPoints) const
{
	const float Length = Spline.GetSplineLength();
	const int32 Samples = FMath::Max(SamplesPerSpline, 2);
	OutPoints.Reserve(Samples);
	for (int32 Index = 0; Index < Samples; ++Index)
	{
		OutPoints.Add(Spline.GetLocationAtDistanceAlongSpline(
			Length * Index / (Samples - 1),
			ESplineCoordinateSpace::World));
	}
}
