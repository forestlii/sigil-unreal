// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "UObject/Interface.h"
#include "SigilCombatTeamAgentInterface.generated.h"

/**
 * Delegate for team ID change events.
 * 队伍ID更改事件的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSigilCombatTeamIdChangedSignature, UObject*, ObjectChangingTeam, FGenericTeamId, OldTeamID, FGenericTeamId, NewTeamID);

/**
 * Interface for combat team agents.
 * 战斗队伍代理的接口。
 */
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class USigilCombatTeamAgentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for managing combat team affiliations.
 * 管理战斗队伍归属的接口。
 */
class SIGILCOMBAT_API ISigilCombatTeamAgentInterface
{
	GENERATED_BODY()

public:
	/**
	 * Sets the combat team ID.
	 * 设置战斗队伍ID。
	 * @note Default implementation converts to GenericTeamAgentInterface.
	 * @注意 默认实现转换为GenericTeamAgentInterface。
	 * @param NewTeamId The new team ID. 新队伍ID。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|CombatTeam")
	void SetCombatTeamId(FGenericTeamId NewTeamId);
	virtual void SetCombatTeamId_Implementation(FGenericTeamId NewTeamId);

	/**
	 * Gets the current combat team ID.
	 * 获取当前战斗队伍ID。
	 * @return The combat team ID. 战斗队伍ID。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|CombatTeam")
	FGenericTeamId GetCombatTeamId() const;
	virtual FGenericTeamId GetCombatTeamId_Implementation() const;

	/**
	 * Gets the delegate for team ID changes.
	 * 获取队伍ID更改的委托。
	 * @return The team ID changed delegate. 队伍ID更改委托。
	 */
	virtual FSigilCombatTeamIdChangedSignature* GetOnTeamIdChangedDelegate() = 0;

	/**
	 * Conditionally broadcasts team change events.
	 * 有条件地广播队伍更改事件。
	 * @param This The combat team agent interface. 战斗队伍代理接口。
	 * @param OldTeamID The previous team ID. 旧队伍ID。
	 * @param NewTeamID The new team ID. 新队伍ID。
	 */
	static void ConditionalBroadcastTeamChanged(TScriptInterface<ISigilCombatTeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);

	/**
	 * Gets the team changed delegate with validation.
	 * 获取经过验证的队伍更改委托。
	 * @return The team changed delegate. 队伍更改委托。
	 */
	FSigilCombatTeamIdChangedSignature& GetTeamChangedDelegateChecked()
	{
		FSigilCombatTeamIdChangedSignature* Result = GetOnTeamIdChangedDelegate();
		check(Result);
		return *Result;
	}
};