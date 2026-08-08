// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GCS_CombatStructLibrary.h"
#include "Engine/DataAsset.h"
#include "GCS_AbilityActionSetSettings.generated.h"

/**
 * Data asset for defining ability action sets.
 * 定义能力动作集的数据资产。
 */
UCLASS(BlueprintType, Const, meta=(DisplayName="GCS Ability Action Set"))
class GENERICCOMBATSYSTEM_API UGCS_AbilityActionSetSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Selects the best ability actions based on tags.
	 * 根据标签选择最佳能力动作。
	 * @param SourceTags Tags for the source. 来源标签。
	 * @param TargetTags Tags for the target. 目标标签。
	 * @param AbilityTags Tags for the ability. 能力标签。
	 * @param Actions The matched ability actions (output). 匹配的能力动作（输出）。
	 * @return True if selection is successful. 如果选择成功返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "GCS", meta=(AutoCreateRefTerm="TargetTags"))
	bool SelectBestAbilityActions(const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, const FGameplayTagContainer& AbilityTags, TArray<FGCS_AbilityAction>& Actions) const;

	/**
	 * Array of ability action sets.
	 * 能力动作集数组。
	 */
	UPROPERTY(EditAnywhere, Category="GCS", meta=(TitleProperty="AbilityTag"))
	TArray<FGCS_AbilityActionSet> ActionSets;

#if WITH_EDITORONLY_DATA
	/**
	 * Called before saving in the editor.
	 * 编辑器中保存前调用。
	 * @param SaveContext The save context. 保存上下文。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};