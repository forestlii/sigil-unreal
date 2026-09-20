// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Traversal/SigilTraversalLibrary.h"

#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Traversal/SigilTraversableInterface.h"

namespace SigilTraversalPrivate
{
	/** Clearance kept between the capsule and the obstacle when checking for room. 做空间检查时胶囊与障碍物之间留的余量。 */
	constexpr float RoomClearance = 2.0f;

	/** How many things the forward probe may look past before giving up. 前探最多越过几个不可攀的东西。 */
	constexpr int32 MaxProbeAttempts = 4;

	/** The obstacle itself, or one of the hit actor's components, that knows its ledges. 知道自己边缘的对象：被命中的组件、其 Actor，或该 Actor 的某个组件。 */
	UObject* FindTraversable(const FHitResult& Hit)
	{
		if (UPrimitiveComponent* Component = Hit.GetComponent())
		{
			if (Component->GetClass()->ImplementsInterface(USigilTraversableInterface::StaticClass()))
			{
				return Component;
			}
		}
		AActor* Actor = Hit.GetActor();
		if (!Actor)
		{
			return nullptr;
		}
		if (Actor->GetClass()->ImplementsInterface(USigilTraversableInterface::StaticClass()))
		{
			return Actor;
		}
		const TArray<UActorComponent*> Components =
			Actor->GetComponentsByInterface(USigilTraversableInterface::StaticClass());
		return Components.IsEmpty() ? nullptr : Components[0];
	}

	FVector RoomLocation(const FVector& Ledge, const FVector& Normal, const float Radius, const float HalfHeight)
	{
		return Ledge + Normal * (Radius + RoomClearance) + FVector(0.0, 0.0, HalfHeight + RoomClearance);
	}
}

float USigilTraversalLibrary::ComputeForwardTraceDistance(const float ForwardSpeed, const bool bGrounded)
{
	return bGrounded
		? FMath::GetMappedRangeValueClamped(FVector2f(0.0f, 500.0f), FVector2f(75.0f, 350.0f), ForwardSpeed)
		: 75.0f;
}

ESigilTraversalAction USigilTraversalLibrary::ClassifyAction(
	const FSigilTraversalCheckResult& Measured,
	const FSigilTraversalRules& Rules,
	const bool bGrounded)
{
	if (!Measured.Ledges.bHasFrontLedge || Measured.ObstacleHeight <= 0.0f)
	{
		return ESigilTraversalAction::None;
	}
	if (Measured.ObstacleHeight > (bGrounded ? Rules.MaxGroundedHeight : Rules.MaxAirborneHeight))
	{
		return ESigilTraversalAction::None;
	}

	const bool bThin = Measured.Ledges.bHasBackLedge && Measured.ObstacleDepth < Rules.ThinObstacleMaxDepth;
	ESigilTraversalAction Action = ESigilTraversalAction::None;
	if (bThin && Measured.bHasBackFloor && Measured.BackLedgeHeight > Rules.HurdleMinBackLedgeHeight)
	{
		Action = ESigilTraversalAction::Hurdle;
	}
	else if (bThin && Measured.bHasBackFloor && Measured.BackLedgeHeight < Rules.StepUpMaxBackLedgeHeight)
	{
		Action = ESigilTraversalAction::Mantle;
	}
	else if (bThin && !Measured.bHasBackFloor)
	{
		Action = ESigilTraversalAction::Vault;
	}
	else if (!Measured.Ledges.bHasBackLedge || Measured.ObstacleDepth > Rules.PlatformMinDepth)
	{
		Action = ESigilTraversalAction::Mantle;
	}

	// The reference design has no grounded hurdle or vault above this height: the move is simply not offered.
	// 参考设计里地面状态的跨栏与翻越没有高于此值的档位：这种情况就是不提供该动作。
	if (bGrounded && Action != ESigilTraversalAction::Mantle && Measured.ObstacleHeight > Rules.MaxHurdleAndVaultHeight)
	{
		return ESigilTraversalAction::None;
	}
	return Action;
}

bool USigilTraversalLibrary::ComputeBoxLedges(
	const FTransform& BoxTransform,
	const FVector& BoxExtent,
	const FVector& HitLocation,
	const FVector& InstigatorLocation,
	FSigilTraversalLedges& OutLedges)
{
	OutLedges = FSigilTraversalLedges();

	const FVector Half = (BoxExtent * BoxTransform.GetScale3D()).GetAbs();
	if (Half.X <= KINDA_SMALL_NUMBER || Half.Y <= KINDA_SMALL_NUMBER || Half.Z <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FQuat Rotation = BoxTransform.GetRotation();
	const FVector Center = BoxTransform.GetLocation();
	const FVector LocalInstigator = Rotation.UnrotateVector(InstigatorLocation - Center);
	const FVector LocalHit = Rotation.UnrotateVector(HitLocation - Center);

	// The face the instigator is furthest outside of is the one being approached.
	// 来者在哪个侧面之外最远，就是正朝哪个侧面走来。
	const double OutsideX = FMath::Abs(LocalInstigator.X) - Half.X;
	const double OutsideY = FMath::Abs(LocalInstigator.Y) - Half.Y;
	if (OutsideX <= 0.0 && OutsideY <= 0.0)
	{
		return false; // Standing inside or on top of the box footprint. 站在盒子的投影范围之内或之上。
	}

	const bool bAlongX = OutsideX >= OutsideY;
	const double Sign = (bAlongX ? LocalInstigator.X : LocalInstigator.Y) >= 0.0 ? 1.0 : -1.0;
	const FVector LocalNormal = bAlongX ? FVector(Sign, 0.0, 0.0) : FVector(0.0, Sign, 0.0);
	const double HalfAlong = bAlongX ? Half.X : Half.Y;
	const double HalfAcross = bAlongX ? Half.Y : Half.X;
	const double Across = FMath::Clamp(bAlongX ? LocalHit.Y : LocalHit.X, -HalfAcross, HalfAcross);
	const FVector LocalAcross = bAlongX ? FVector(0.0, Across, 0.0) : FVector(Across, 0.0, 0.0);
	const FVector LocalTop(0.0, 0.0, Half.Z);

	OutLedges.bHasFrontLedge = true;
	OutLedges.FrontLedgeLocation = Center + Rotation.RotateVector(LocalNormal * HalfAlong + LocalAcross + LocalTop);
	OutLedges.FrontLedgeNormal = Rotation.RotateVector(LocalNormal).GetSafeNormal2D();
	OutLedges.bHasBackLedge = true;
	OutLedges.BackLedgeLocation = Center + Rotation.RotateVector(-LocalNormal * HalfAlong + LocalAcross + LocalTop);
	OutLedges.BackLedgeNormal = -OutLedges.FrontLedgeNormal;
	return !OutLedges.FrontLedgeNormal.IsNearlyZero();
}

bool USigilTraversalLibrary::CheckTraversal(
	AActor* Instigator,
	const FVector& FeetLocation,
	const FSigilTraversalCheckInputs& Inputs,
	const FSigilTraversalRules& Rules,
	const bool bGrounded,
	FSigilTraversalCheckResult& OutResult)
{
	using namespace SigilTraversalPrivate;

	OutResult = FSigilTraversalCheckResult();
	UWorld* World = Instigator ? Instigator->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SigilTraversalCheck), false, Instigator);
	const FCollisionShape Capsule = FCollisionShape::MakeCapsule(Inputs.TraceRadius, Inputs.TraceHalfHeight);
	const FVector InstigatorLocation = Instigator->GetActorLocation();
	const FVector Start = InstigatorLocation + Inputs.TraceOriginOffset;
	const FVector End = Start + Inputs.TraceForwardDirection.GetSafeNormal() * Inputs.TraceForwardDistance + Inputs.TraceEndOffset;

	// The first thing in the way is not always the traversable one: a window sits in a wall whose face is flush with it.
	// Look a little further behind anything that cannot be traversed, but never further than TraversableSearchDepth.
	// 挡在面前的第一个东西不一定就是可攀的：窗嵌在墙里，墙面与窗齐平。不可攀的就再往后找一点，但不超过 TraversableSearchDepth。
	FCollisionQueryParams ProbeParams = Params;
	FHitResult Hit;
	bool bFound = false;
	float FirstBlockingDistance = -1.0f;
	for (int32 Attempt = 0; Attempt < MaxProbeAttempts && !bFound; ++Attempt)
	{
		if (!World->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, Capsule, ProbeParams))
		{
			return false;
		}
		if (FirstBlockingDistance < 0.0f)
		{
			FirstBlockingDistance = Hit.Distance;
		}
		else if (Hit.Distance > FirstBlockingDistance + Inputs.TraversableSearchDepth)
		{
			return false;
		}

		// A probe that starts out touching the obstacle has no meaningful impact point: use the character's own position.
		// 起点就贴着障碍物的检测没有可用的命中点：改用角色自身位置。
		const FVector HitLocation = Hit.bStartPenetrating ? InstigatorLocation : FVector(Hit.ImpactPoint);
		UObject* Traversable = FindTraversable(Hit);
		bFound = Traversable &&
			ISigilTraversableInterface::Execute_GetTraversalLedges(Traversable, HitLocation, InstigatorLocation, OutResult.Ledges) &&
			OutResult.Ledges.bHasFrontLedge;
		if (!bFound)
		{
			if (const AActor* HitActor = Hit.GetActor())
			{
				ProbeParams.AddIgnoredActor(HitActor);
			}
			else if (const UPrimitiveComponent* HitPrimitive = Hit.GetComponent())
			{
				ProbeParams.AddIgnoredComponent(HitPrimitive);
			}
			else
			{
				break;
			}
		}
	}
	if (!bFound)
	{
		OutResult = FSigilTraversalCheckResult();
		return false;
	}
	OutResult.HitComponent = Hit.GetComponent();

	// Room above the front ledge: standing first, then crouched.
	// 前边缘上方放不放得下角色：先按站立，再按蹲下。
	const float Radius = Inputs.CapsuleRadius > 0.0f ? Inputs.CapsuleRadius : Inputs.TraceRadius;
	const float StandingHalfHeight = FMath::Max(Inputs.CapsuleHalfHeight > 0.0f ? Inputs.CapsuleHalfHeight : Inputs.TraceHalfHeight, Radius);
	float FitHalfHeight = StandingHalfHeight;
	FVector FrontRoom = RoomLocation(OutResult.Ledges.FrontLedgeLocation, OutResult.Ledges.FrontLedgeNormal, Radius, FitHalfHeight);
	FHitResult RoomHit;
	if (World->SweepSingleByChannel(RoomHit, InstigatorLocation, FrontRoom, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(Radius, FitHalfHeight), Params))
	{
		FitHalfHeight = FMath::Max(
			Inputs.CrouchedHalfHeight > 0.0f ? Inputs.CrouchedHalfHeight : StandingHalfHeight * 0.5f,
			Radius);
		FrontRoom = RoomLocation(OutResult.Ledges.FrontLedgeLocation, OutResult.Ledges.FrontLedgeNormal, Radius, FitHalfHeight);
		const FCollisionShape Crouched = FCollisionShape::MakeCapsule(Radius, FitHalfHeight);
		if (World->SweepSingleByChannel(RoomHit, InstigatorLocation, FrontRoom, FQuat::Identity, ECC_Visibility, Crouched, Params))
		{
			OutResult = FSigilTraversalCheckResult();
			return false;
		}
		OutResult.bShouldCrouch = true;
	}
	const FCollisionShape FitCapsule = FCollisionShape::MakeCapsule(Radius, FitHalfHeight);
	OutResult.FitHalfHeight = FitHalfHeight;

	OutResult.ObstacleHeight = static_cast<float>(OutResult.Ledges.FrontLedgeLocation.Z - FeetLocation.Z);

	if (OutResult.Ledges.bHasBackLedge)
	{
		const FVector BackRoom = RoomLocation(OutResult.Ledges.BackLedgeLocation, OutResult.Ledges.BackLedgeNormal, Radius, FitHalfHeight);
		// Something on top of the obstacle blocks the way across: the far edge cannot be reached.
		// 障碍物顶面上有东西挡着：到不了远侧边缘。
		if (World->SweepSingleByChannel(RoomHit, FrontRoom, BackRoom, FQuat::Identity, ECC_Visibility, FitCapsule, Params))
		{
			OutResult.Ledges.bHasBackLedge = false;
		}
		else
		{
			OutResult.ObstacleDepth = static_cast<float>(FVector::Dist2D(
				OutResult.Ledges.FrontLedgeLocation,
				OutResult.Ledges.BackLedgeLocation));

			const double LowestCenterZ = FeetLocation.Z - Rules.BackFloorMaxDropBelowFeet + FitHalfHeight;
			const FVector FloorProbeEnd(BackRoom.X, BackRoom.Y, FMath::Min(LowestCenterZ, BackRoom.Z - 1.0));
			FHitResult FloorHit;
			if (World->SweepSingleByChannel(FloorHit, BackRoom, FloorProbeEnd, FQuat::Identity, ECC_Visibility, FitCapsule, Params))
			{
				OutResult.bHasBackFloor = true;
				OutResult.BackFloorLocation = FloorHit.ImpactPoint;
				OutResult.BackLedgeHeight = static_cast<float>(OutResult.Ledges.BackLedgeLocation.Z - FloorHit.ImpactPoint.Z);
			}
		}
	}

	OutResult.Action = ClassifyAction(OutResult, Rules, bGrounded);
	return OutResult.Action != ESigilTraversalAction::None;
}
