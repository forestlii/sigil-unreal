// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Chaos/ChaosEngineInterface.h"
#include "UObject/Object.h"
#include "GES_ContextEffectsPreviewSetting.generated.h"

/**
 * Data asset for context effects preview settings.
 * 情景效果预览设置的数据资产。
 */
UCLASS()
class GENERICEFFECTSSYSTEM_API UGES_ContextEffectsPreviewSetting : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Whether to use physical surface as context for preview.
	 * 是否使用物理表面作为预览情景。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Preview)
	bool bPreviewPhysicalSurfaceAsContext = true;

	/**
	 * Physical surface type for preview.
	 * 预览的物理表面类型。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Preview, meta = (EditCondition = "bPreviewPhysicalSurfaceAsContext"))
	TEnumAsByte<EPhysicalSurface> PreviewPhysicalSurface = EPhysicalSurface::SurfaceType_Default;

	/**
	 * Context effects libraries for preview.
	 * 预览的情景效果库。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Preview, meta = (AllowedClasses = "/Script/GenericEffectsSystem.GES_ContextEffectsLibrary"))
	TArray<FSoftObjectPath> PreviewContextEffectsLibraries;

	/**
	 * Source context tags for preview.
	 * 预览的源情景标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Preview)
	FGameplayTagContainer PreviewSourceContext;

	/**
	 * Target context tags for preview.
	 * 预览的目标情景标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Preview)
	FGameplayTagContainer PreviewTargetContext;
};