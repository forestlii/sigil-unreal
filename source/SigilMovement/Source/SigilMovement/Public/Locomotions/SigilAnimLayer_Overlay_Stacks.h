// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAnimLayer.h"
#include "SigilAnimLayer_Overlay.h"
#include "Settings/SigilSettingObjectLibrary.h"
#include "SigilAnimLayer_Overlay_Stacks.generated.h"


struct FCachedAnimStateData;

USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilAnimData_StackedOverlays
{
	GENERATED_BODY()

public:
	/**
	 * Animation overlays are evaluated when any of the animation state nodes in this list have relevance.
	 * @attention If you are customising the animation blueprints, you can configure the mapping of state names to gameplay tags in the GMS main AnimInstance or any SigilAnimLayer derived AnimInstances.
	 * 当该列表中的任何动画状态节点具有相关性时，将对动画叠加进行评估。
	 * @attention 如果你在自定义动画蓝图，您可以在 GMS 主AnimInstance或任何 SigilAnimLayer派生AnimInstance中配置状态名称到游戏标记的映射。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(Categories="Sigil.Movement.SM"))
	FGameplayTagContainer TargetAnimNodes;

	/**
	 * List of potential animation overlays to use, First matched one will be selected.
	 * 潜在的用于混合的动画叠层，第一个满足条件的会被选用。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(TitleProperty="EditorMessage"))
	TArray<FSigilAnimData_Overlay> Overlays;

#if WITH_EDITORONLY_DATA
	/** Friendly name for displaying in the Editor's Index */
	UPROPERTY(VisibleAnywhere, Category="GMS", Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

USTRUCT(BlueprintType)
struct FSigilStackedOverlaySetting
{
	GENERATED_BODY()

	/**
	 * The name of this stack. must be unique.
	 * 此栈的名称，必须唯一。
	 */
	UPROPERTY(EditAnywhere, Category="GMS", meta=(Categories="Sigil.Movement.OverlayMode"))
	FGameplayTag Tag;

	/**
	 * Allows you to blend different animations on different body parts in parallel(max to 10).
	 * @attention The later Stack the higher the priority, Each Stack in turn allows you to choose an appropriate animation to blend among multiple potential animations.
	 * 允许你并行地在不同的身体部位混合不同的动画(最多并行10个)。
	 * @注意 越往后的Stack优先级越高,每一个Stack又允许你在多个潜在动画中选择一个合适的动画进行混合。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAnimData_StackedOverlays> StackedOverlays;
};


USTRUCT(BlueprintType)
struct SIGILMOVEMENT_API FSigilOverlayStackState
{
	GENERATED_BODY()

	// is target anim nodes relevant?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	bool bRelevant{false};

	// current weight of this overlay.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	float BlendWeight{0.0f};

	// blend out speed.
	float BlendOutSpeed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GMS")
	FSigilAnimData_Overlay Overlay;
};


/**
 * Native stack based overlay anim layer setting implementation.
 */
UCLASS(NotBlueprintable)
class SIGILMOVEMENT_API USigilAnimLayerSetting_Overlay_Stack final : public USigilAnimLayerSetting_Overlay
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="GMS", meta=(EditCondition=false, EditConditionHides))
	TMap<FGameplayTag, FSigilStackedOverlaySetting> AcceleratedOverlayModes;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings",meta=(TitleProperty="Tag"))
	TArray<FSigilStackedOverlaySetting> OverlayModes;

#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};


/**
 * Native stack based overlay anim layer implementation.
 */
UCLASS(Abstract)
class SIGILMOVEMENT_API USigilAnimLayer_Overlay_Stack : public USigilAnimLayer
{
	GENERATED_BODY()
	friend USigilAnimLayerSetting_Overlay_Stack;

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	virtual void ApplySetting_Implementation(const USigilAnimLayerSetting* Setting) override;
	virtual void ResetSetting_Implementation() override;

protected:
	virtual void RefreshRelevance();

	virtual void RefreshBlend(float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<const USigilAnimLayerSetting_Overlay_Stack> PrevSetting{nullptr};

	UPROPERTY()
	FGameplayTag PrevOverlayMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAnimData_StackedOverlays> OverlayStacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	TArray<FSigilOverlayStackState> OverlayStackStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State", meta=(ClampMin=4))
	int32 MaxLayers{10};
};
