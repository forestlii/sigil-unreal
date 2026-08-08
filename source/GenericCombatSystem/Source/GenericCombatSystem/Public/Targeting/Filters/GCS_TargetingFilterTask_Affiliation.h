// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionTypes.h"
#include "Tasks/TargetingFilterTask_BasicFilterTemplate.h"
#include "GCS_TargetingFilterTask_Affiliation.generated.h"

/**
 * Filters targets based on team affiliation.
 * 根据队伍归属过滤目标。
 */
UCLASS(meta=(DisplayName="GCS:FilterTask (Affiliation)"))
class GENERICCOMBATSYSTEM_API UGCS_TargetingFilterTask_Affiliation : public UTargetingFilterTask_BasicFilterTemplate
{
	GENERATED_BODY()

protected:
	/**
	 * Affiliation filter settings.
	 * 归属过滤设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	FAISenseAffiliationFilter DetectionByAffiliation;

	/**
	 * Whether to check for CombatTeamAgentInterface.
	 * 是否检查CombatTeamAgentInterface。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	bool bLookCombatTeamAgentInterface{true};

	/**
	 * Whether to check components for CombatTeamAgentInterface.
	 * 是否检查组件中的CombatTeamAgentInterface。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter", meta=(EditCondition="bLookCombatTeamAgentInterface", EditConditionHides))
	bool bLookCombatTeamAgentInterfaceInComponents{true};

	/**
	 * Whether to check for GenericTeamAgentInterface.
	 * 是否检查GenericTeamAgentInterface。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	bool bLookGenericTeamAgentInterface{true};

	/**
	 * Whether to check the controller for GenericTeamAgentInterface.
	 * 是否检查控制器的GenericTeamAgentInterface。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter", meta=(EditCondition="bLookGenericTeamAgentInterface", EditConditionHides))
	bool bLookController{true};

	/**
	 * Determines if a target should be filtered based on affiliation.
	 * 根据归属确定是否应过滤目标。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param TargetData The target data. 目标数据。
	 * @return True if the target should be filtered. 如果应过滤目标返回true。
	 */
	virtual bool ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const override;

	/**
	 * Gets the source team ID.
	 * 获取来源队伍ID。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param TargetData The target data. 目标数据。
	 * @return The source team ID. 来源队伍ID。
	 */
	virtual FGenericTeamId GetSourceTeamId(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const;

	/**
	 * Gets the target team ID.
	 * 获取目标队伍ID。
	 * @param TargetingHandle The targeting request handle. 目标请求句柄。
	 * @param TargetData The target data. 目标数据。
	 * @return The target team ID. 目标队伍ID。
	 */
	virtual FGenericTeamId GetTargetTeamId(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const;
};