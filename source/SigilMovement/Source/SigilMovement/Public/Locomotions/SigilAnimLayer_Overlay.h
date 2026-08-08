// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAnimLayer.h"
#include "Settings/SigilSettingObjectLibrary.h"

#include "SigilAnimLayer_Overlay.generated.h"


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_Overlay
{
	GENERATED_BODY()

	void Validate();

	/**
	 * The gameplay tags of movement system component must match this TagQuery so this overlay can be selected.
	 * @attention Left empty if you don't need any preconditions.
	 * 运动系统组件上的通过GetGameplayTags返回的标签比如满足此Tag查询，此叠层才会被使用。
	 * @注意 留空如果你不需要任何前置Tag条件。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS")
	FGameplayTagQuery TagQuery;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS")
	TObjectPtr<UAnimSequenceBase> Sequence = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS", meta=(ClampMax=1, ClampMin=0))
	float BlendWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS")
	bool MeshSpaceBlend = true;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS")
	ESigilOverlayPlayMode PlayMode{ESigilOverlayPlayMode::SequenceEvaluator};

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS", meta=(ClampMin=0, EditCondition="PlayMode==ESigilOverlayPlayMode::SequencePlayer", EditConditionHides))
	float StartPosition = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS", meta=(ClampMin=0, EditCondition="PlayMode==ESigilOverlayPlayMode::SequenceEvaluator", EditConditionHides))
	float ExplicitTime = 0.0f;

	/***
	 * @attention If your project have many different skeletons, you should use branch filter method for reusability, otherwise you will define same blend mask for all of your skeletons.
	 * @注意 如果你的整个项目没有统一使用一套骨架，建议使用branch filter方式，否则建议你要为不同的骨架定义相同的混合遮罩，会很繁琐。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS")
	ESigilLayeredBoneBlendMode BlendMode{ESigilLayeredBoneBlendMode::BlendMask};

	/**
	 * Doc link of BlendMask:https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-blend-nodes-in-unreal-engine#blendmask
	 * @attention You must define the specified blend mask in your skeleton first.
	 * @注意 你必须在动画所关联的骨架上先定义BlendMask混合遮罩.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS", meta=(EditCondition="BlendMode==ESigilLayeredBoneBlendMode::BlendMask", EditConditionHides))
	FName BlendMaskName;

	/**
	 * Doc link of BranchFilters:https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-blend-nodes-in-unreal-engine#branchfilter
	 * @attention Read docs to know how to use branch filters.
	 * @注意 阅读上面的文档链接学习如何使用BranchFilters.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS", meta=(EditCondition="BlendMode==ESigilLayeredBoneBlendMode::BranchFilter", EditConditionHides))
	FSigilInputBlendPose BranchFilters;

	/**
	 * Controls how fast to blend to BlendWeight
	 * @attention  ==0 means instantly change to BlendWeight.
	 * 控制多快到达指定的BlendWeight(混合权重)。
	 * @注意 ==0 意味着瞬间混入。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS", meta=(ClampMin=0))
	float BlendInSpeed = 10.f;

	/**
	 * Controls how fast to go back to zero weight.
	 * @attention ==0 means instantly back to zero weight.
	 * 控制多快将权重归零。
	 * @注意 ==0 意味着瞬间归0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="GMS", meta=(ClampMin=0))
	float BlendOutSpeed = 10.f;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GMS")
	bool bValid{false};

	friend bool operator==(const FSigilAnimData_Overlay& Lhs, const FSigilAnimData_Overlay& RHS)
	{
		return Lhs.Sequence == RHS.Sequence
			&& Lhs.BlendMode == RHS.BlendMode
			&& Lhs.MeshSpaceBlend == RHS.MeshSpaceBlend
			&& Lhs.bValid == RHS.bValid;
	}

	friend bool operator!=(const FSigilAnimData_Overlay& Lhs, const FSigilAnimData_Overlay& RHS)
	{
		return !(Lhs == RHS);
	}

#if WITH_EDITORONLY_DATA
	/** Friendly name for displaying in the Editor's Index */
	UPROPERTY(VisibleAnywhere, Category="GMS", Meta=(EditCondition=False, EditConditionHides))
	FString EditorMessage;
#endif
};

/**
 * Native parent class for all overlay anim layer setting.
 */
UCLASS(Abstract, Blueprintable)
class SIGILMOVEMENT_API USigilAnimLayerSetting_Overlay : public USigilAnimLayerSetting
{
	GENERATED_BODY()

public:

	virtual bool IsValidForOverlayMode(const FGameplayTag& NewOverlayMode) const {return true;};
};
