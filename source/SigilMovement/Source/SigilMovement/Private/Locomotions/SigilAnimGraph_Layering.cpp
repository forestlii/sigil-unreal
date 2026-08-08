// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Locomotions/SigilAnimGraph_Layering.h"

#include "Animation/AnimInstanceProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAnimGraph_Layering)

void USigilAnimGraph_Layering::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	RefreshLayering();

}

void USigilAnimGraph_Layering::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

}

void USigilAnimGraph_Layering::RefreshLayering()
{
	const auto& Curves{GetAnimationCurves(EAnimCurveType::AttributeCurve)};

	static const auto GetCurveValue{
		[](const TMap<FName, float>& InCurves, const FName& CurveName) -> float
		{
			const auto* Value{InCurves.Find(CurveName)};

			return Value != nullptr ? *Value : 0.0f;
		}
	};

	LayeringState.HeadBlendAmount = GetCurveValue(Curves, LayerHeadCurveName);
	LayeringState.HeadAdditiveBlendAmount = GetCurveValue(Curves, LayerHeadAdditiveCurveName);
	LayeringState.HeadSlotBlendAmount = GetCurveValue(Curves, LayerHeadSlotCurveName);

	// The mesh space blend will always be 1 unless the local space blend is 1.

	LayeringState.ArmLeftBlendAmount = GetCurveValue(Curves, LayerArmLeft);
	LayeringState.ArmLeftAdditiveBlendAmount = GetCurveValue(Curves, LayerArmLeftAdditive);
	LayeringState.ArmLeftSlotBlendAmount = GetCurveValue(Curves, LayerArmLeftSlot);
	LayeringState.ArmLeftLocalSpaceBlendAmount = GetCurveValue(Curves, LayerArmLeftLocalSpace);
	LayeringState.ArmLeftMeshSpaceBlendAmount = !FAnimWeight::IsFullWeight(LayeringState.ArmLeftLocalSpaceBlendAmount);

	// The mesh space blend will always be 1 unless the local space blend is 1.

	LayeringState.ArmRightBlendAmount = GetCurveValue(Curves, LayerArmRight);
	LayeringState.ArmRightAdditiveBlendAmount = GetCurveValue(Curves, LayerArmRightAdditive);
	LayeringState.ArmRightSlotBlendAmount = GetCurveValue(Curves, LayerArmRightSlot);
	LayeringState.ArmRightLocalSpaceBlendAmount = GetCurveValue(Curves, LayerArmRightLocalSpace);
	LayeringState.ArmRightMeshSpaceBlendAmount = !FAnimWeight::IsFullWeight(LayeringState.ArmRightLocalSpaceBlendAmount);

	LayeringState.HandLeftBlendAmount = GetCurveValue(Curves, LayerHandLeft);
	LayeringState.HandRightBlendAmount = GetCurveValue(Curves, LayerHandRight);

	LayeringState.SpineBlendAmount = GetCurveValue(Curves, LayerSpine);
	LayeringState.SpineAdditiveBlendAmount = GetCurveValue(Curves, LayerSpineAdditive);
	LayeringState.SpineSlotBlendAmount = GetCurveValue(Curves, LayerSpineSlot);

	LayeringState.PelvisBlendAmount = GetCurveValue(Curves, LayerPelvis);
	LayeringState.PelvisSlotBlendAmount = GetCurveValue(Curves, LayerPelvisSlot);

	LayeringState.LegsBlendAmount = GetCurveValue(Curves, LayerLegs);
	LayeringState.LegsSlotBlendAmount = GetCurveValue(Curves, LayerLegsSlot);
}
