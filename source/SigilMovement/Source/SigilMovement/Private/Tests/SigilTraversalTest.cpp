// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Traversal/SigilTraversalLedgeComponent.h"
#include "Traversal/SigilTraversalLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilMovementTraversalRulesTest,
	"SigilMovement.Traversal.Rules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilMovementTraversalRulesTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("standing still probes 75"), USigilTraversalLibrary::ComputeForwardTraceDistance(0.0f, true), 75.0f);
	TestEqual(TEXT("full speed probes 350"), USigilTraversalLibrary::ComputeForwardTraceDistance(500.0f, true), 350.0f);
	TestEqual(TEXT("speed is clamped"), USigilTraversalLibrary::ComputeForwardTraceDistance(900.0f, true), 350.0f);
	TestEqual(TEXT("half speed is halfway"), USigilTraversalLibrary::ComputeForwardTraceDistance(250.0f, true), 212.5f);
	TestEqual(TEXT("airborne always probes 75"), USigilTraversalLibrary::ComputeForwardTraceDistance(500.0f, false), 75.0f);

	const FSigilTraversalRules Rules;
	auto Classify = [&Rules](const bool bBackLedge, const bool bBackFloor, const float Height, const float Depth, const float Drop, const bool bGrounded)
	{
		FSigilTraversalCheckResult Measured;
		Measured.Ledges.bHasFrontLedge = true;
		Measured.Ledges.bHasBackLedge = bBackLedge;
		Measured.bHasBackFloor = bBackFloor;
		Measured.ObstacleHeight = Height;
		Measured.ObstacleDepth = Depth;
		Measured.BackLedgeHeight = Drop;
		return USigilTraversalLibrary::ClassifyAction(Measured, Rules, bGrounded);
	};

	TestTrue(TEXT("thin, lower floor behind: hurdle"), Classify(true, true, 100.0f, 30.0f, 100.0f, true) == ESigilTraversalAction::Hurdle);
	TestTrue(TEXT("thin, floor level with the top: mantle"), Classify(true, true, 100.0f, 30.0f, 5.0f, true) == ESigilTraversalAction::Mantle);
	TestTrue(TEXT("thin, no floor behind: vault"), Classify(true, false, 100.0f, 30.0f, 0.0f, true) == ESigilTraversalAction::Vault);
	TestTrue(TEXT("deep platform: mantle"), Classify(true, true, 100.0f, 200.0f, 100.0f, true) == ESigilTraversalAction::Mantle);
	TestTrue(TEXT("no back ledge: mantle"), Classify(false, false, 100.0f, 0.0f, 0.0f, true) == ESigilTraversalAction::Mantle);

	// Boundaries of the four-row table. 四行判定表的边界。
	TestTrue(TEXT("depth 58 is thin"), Classify(true, false, 100.0f, 58.0f, 0.0f, true) == ESigilTraversalAction::Vault);
	TestTrue(TEXT("depth 60 is a platform"), Classify(true, false, 100.0f, 60.0f, 0.0f, true) == ESigilTraversalAction::Mantle);
	TestTrue(TEXT("drop 51 hurdles"), Classify(true, true, 100.0f, 30.0f, 51.0f, true) == ESigilTraversalAction::Hurdle);
	TestTrue(TEXT("drop 9 steps up"), Classify(true, true, 100.0f, 30.0f, 9.0f, true) == ESigilTraversalAction::Mantle);
	TestTrue(TEXT("thin with a drop between 10 and 50, depth over 29: mantle by the last row"),
		Classify(true, true, 100.0f, 40.0f, 30.0f, true) == ESigilTraversalAction::Mantle);
	TestTrue(TEXT("thin with a drop between 10 and 50, depth under 29: nothing fits"),
		Classify(true, true, 100.0f, 20.0f, 30.0f, true) == ESigilTraversalAction::None);

	// Height limits. 高度上限。
	TestTrue(TEXT("275 is reachable from the ground"), Classify(false, false, 275.0f, 0.0f, 0.0f, true) == ESigilTraversalAction::Mantle);
	TestTrue(TEXT("276 is not"), Classify(false, false, 276.0f, 0.0f, 0.0f, true) == ESigilTraversalAction::None);
	TestTrue(TEXT("airborne catches up to 200"), Classify(false, false, 200.0f, 0.0f, 0.0f, false) == ESigilTraversalAction::Mantle);
	TestTrue(TEXT("airborne 201 is out of reach"), Classify(false, false, 201.0f, 0.0f, 0.0f, false) == ESigilTraversalAction::None);
	TestTrue(TEXT("no grounded vault above 125"), Classify(true, false, 126.0f, 30.0f, 0.0f, true) == ESigilTraversalAction::None);
	TestTrue(TEXT("airborne vault above 125 is allowed"), Classify(true, false, 150.0f, 30.0f, 0.0f, false) == ESigilTraversalAction::Vault);

	FSigilTraversalCheckResult NoLedge;
	NoLedge.ObstacleHeight = 100.0f;
	TestTrue(TEXT("no front ledge: nothing"), USigilTraversalLibrary::ClassifyAction(NoLedge, Rules, true) == ESigilTraversalAction::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilMovementTraversalBoxLedgesTest,
	"SigilMovement.Traversal.BoxLedges",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilMovementTraversalBoxLedgesTest::RunTest(const FString& Parameters)
{
	const FVector Extent(50.0, 50.0, 50.0);
	FSigilTraversalLedges Ledges;

	// Axis aligned, scaled to 40 x 400 x 100: a low wall along Y. Approached from -X.
	// 轴对齐，缩放成 40 x 400 x 100：一堵沿 Y 方向的矮墙，从 -X 方向走近。
	const FTransform Wall(FQuat::Identity, FVector(1000.0, 0.0, 50.0), FVector(0.4, 4.0, 1.0));
	TestTrue(TEXT("wall has ledges"), USigilTraversalLibrary::ComputeBoxLedges(Wall, Extent, FVector(980.0, 120.0, 60.0), FVector(800.0, 120.0, 90.0), Ledges));
	TestTrue(TEXT("front ledge on the near top edge"), Ledges.FrontLedgeLocation.Equals(FVector(980.0, 120.0, 100.0), 0.01));
	TestTrue(TEXT("front normal faces the instigator"), Ledges.FrontLedgeNormal.Equals(FVector(-1.0, 0.0, 0.0), 0.001));
	TestTrue(TEXT("back ledge on the far top edge"), Ledges.BackLedgeLocation.Equals(FVector(1020.0, 120.0, 100.0), 0.01));
	TestTrue(TEXT("back normal points away"), Ledges.BackLedgeNormal.Equals(FVector(1.0, 0.0, 0.0), 0.001));

	// Same wall from the other side. 同一堵墙从另一侧走近。
	TestTrue(TEXT("other side works"), USigilTraversalLibrary::ComputeBoxLedges(Wall, Extent, FVector(1020.0, -30.0, 60.0), FVector(1200.0, -30.0, 90.0), Ledges));
	TestTrue(TEXT("front ledge flips"), Ledges.FrontLedgeLocation.Equals(FVector(1020.0, -30.0, 100.0), 0.01));
	TestTrue(TEXT("front normal flips"), Ledges.FrontLedgeNormal.Equals(FVector(1.0, 0.0, 0.0), 0.001));

	// The hit is clamped onto the ledge when it slides past the end of the wall. 命中点滑出墙端时被钳回边缘上。
	TestTrue(TEXT("hit past the end still yields a ledge"), USigilTraversalLibrary::ComputeBoxLedges(Wall, Extent, FVector(980.0, 260.0, 60.0), FVector(800.0, 260.0, 90.0), Ledges));
	TestTrue(TEXT("ledge point stays on the wall"), FMath::IsNearlyEqual(Ledges.FrontLedgeLocation.Y, 200.0, 0.01));

	// Yawed 90 degrees: the wall now runs along X and is approached from -Y. 偏航 90 度：墙改为沿 X 方向，从 -Y 方向走近。
	const FTransform Yawed(FRotator(0.0, 90.0, 0.0).Quaternion(), FVector(0.0, 1000.0, 50.0), FVector(0.4, 4.0, 1.0));
	TestTrue(TEXT("yawed wall has ledges"), USigilTraversalLibrary::ComputeBoxLedges(Yawed, Extent, FVector(50.0, 980.0, 60.0), FVector(50.0, 800.0, 90.0), Ledges));
	TestTrue(TEXT("yawed front ledge"), Ledges.FrontLedgeLocation.Equals(FVector(50.0, 980.0, 100.0), 0.05));
	TestTrue(TEXT("yawed front normal"), Ledges.FrontLedgeNormal.Equals(FVector(0.0, -1.0, 0.0), 0.001));
	TestTrue(TEXT("depth equals the thin side"), FMath::IsNearlyEqual(FVector::Dist2D(Ledges.FrontLedgeLocation, Ledges.BackLedgeLocation), 40.0, 0.05));

	// Standing over the box footprint offers nothing. 站在盒子投影范围内时不提供边缘。
	TestFalse(TEXT("inside the footprint"), USigilTraversalLibrary::ComputeBoxLedges(Wall, Extent, FVector(1000.0, 0.0, 100.0), FVector(1000.0, 0.0, 190.0), Ledges));
	TestFalse(TEXT("degenerate box"), USigilTraversalLibrary::ComputeBoxLedges(FTransform(FQuat::Identity, FVector::ZeroVector, FVector(0.0, 1.0, 1.0)), Extent, FVector::ZeroVector, FVector(-100.0, 0.0, 0.0), Ledges));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilMovementTraversalPolylineLedgesTest,
	"SigilMovement.Traversal.PolylineLedges",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilMovementTraversalPolylineLedgesTest::RunTest(const FString& Parameters)
{
	// A window sill: two 80 long edges along Y, 64 apart in X, 100 above the floor. Approached from -X.
	// 一个窗台：两条沿 Y、长 80 的边，X 方向相距 64，离地 100。从 -X 方向走近。
	const TArray<FVector> Near = {FVector(100.0, -40.0, 100.0), FVector(100.0, 40.0, 100.0)};
	const TArray<FVector> Far = {FVector(164.0, -40.0, 100.0), FVector(164.0, 40.0, 100.0)};
	FSigilTraversalLedges Ledges;

	TestTrue(TEXT("sill has ledges"), USigilTraversalLedgeComponent::ComputePolylineLedges(Near, Far, FVector(100.0, 10.0, 60.0), FVector(0.0, 10.0, 90.0), 0.0f, Ledges));
	TestTrue(TEXT("front point is the nearest point of the near edge"), Ledges.FrontLedgeLocation.Equals(FVector(100.0, 10.0, 100.0), 0.01));
	TestTrue(TEXT("front normal points away from the far edge"), Ledges.FrontLedgeNormal.Equals(FVector(-1.0, 0.0, 0.0), 0.001));
	TestTrue(TEXT("back point is straight across"), Ledges.bHasBackLedge && Ledges.BackLedgeLocation.Equals(FVector(164.0, 10.0, 100.0), 0.01));
	TestTrue(TEXT("back normal is opposite"), Ledges.BackLedgeNormal.Equals(FVector(1.0, 0.0, 0.0), 0.001));

	// A hit beside the opening is pulled back onto the edge, and MinLedgeWidth keeps it off the very end.
	// 命中点在洞口旁边时被拉回边上；MinLedgeWidth 让它不贴着端点。
	TestTrue(TEXT("hit beside the opening"), USigilTraversalLedgeComponent::ComputePolylineLedges(Near, Far, FVector(100.0, 90.0, 60.0), FVector(0.0, 90.0, 90.0), 0.0f, Ledges));
	TestTrue(TEXT("clamped to the end"), FMath::IsNearlyEqual(Ledges.FrontLedgeLocation.Y, 40.0, 0.01));
	TestTrue(TEXT("with a minimum width"), USigilTraversalLedgeComponent::ComputePolylineLedges(Near, Far, FVector(100.0, 90.0, 60.0), FVector(0.0, 90.0, 90.0), 60.0f, Ledges));
	TestTrue(TEXT("kept half the width from the end"), FMath::IsNearlyEqual(Ledges.FrontLedgeLocation.Y, 10.0, 0.01));
	TestFalse(TEXT("edge shorter than the minimum width"), USigilTraversalLedgeComponent::ComputePolylineLedges(Near, Far, FVector(100.0, 0.0, 60.0), FVector(0.0, 0.0, 90.0), 100.0f, Ledges));

	// No far edge: a platform. The normal is the side the character stands on.
	// 没有对边：平台。法线取角色所在的一侧。
	TestTrue(TEXT("single edge"), USigilTraversalLedgeComponent::ComputePolylineLedges(Near, TArray<FVector>(), FVector(100.0, 0.0, 60.0), FVector(0.0, 0.0, 90.0), 0.0f, Ledges));
	TestFalse(TEXT("no back ledge"), Ledges.bHasBackLedge);
	TestTrue(TEXT("normal faces the character"), Ledges.FrontLedgeNormal.Equals(FVector(-1.0, 0.0, 0.0), 0.001));
	TestTrue(TEXT("single edge from the other side"), USigilTraversalLedgeComponent::ComputePolylineLedges(Near, TArray<FVector>(), FVector(100.0, 0.0, 60.0), FVector(300.0, 0.0, 90.0), 0.0f, Ledges));
	TestTrue(TEXT("normal flips"), Ledges.FrontLedgeNormal.Equals(FVector(1.0, 0.0, 0.0), 0.001));

	// A bent edge: the nearest point is found on the right segment. 折线边：最近点落在正确的线段上。
	const TArray<FVector> Bent = {FVector(0.0, 0.0, 50.0), FVector(100.0, 0.0, 50.0), FVector(100.0, 100.0, 50.0)};
	TestTrue(TEXT("bent edge"), USigilTraversalLedgeComponent::ComputePolylineLedges(Bent, TArray<FVector>(), FVector(130.0, 60.0, 20.0), FVector(200.0, 60.0, 90.0), 0.0f, Ledges));
	TestTrue(TEXT("point on the second segment"), Ledges.FrontLedgeLocation.Equals(FVector(100.0, 60.0, 50.0), 0.01));
	TestTrue(TEXT("normal of the second segment"), Ledges.FrontLedgeNormal.Equals(FVector(1.0, 0.0, 0.0), 0.001));

	TestFalse(TEXT("degenerate edge"), USigilTraversalLedgeComponent::ComputePolylineLedges(TArray<FVector>{FVector::ZeroVector}, TArray<FVector>(), FVector::ZeroVector, FVector::ZeroVector, 0.0f, Ledges));
	return true;
}

#endif
