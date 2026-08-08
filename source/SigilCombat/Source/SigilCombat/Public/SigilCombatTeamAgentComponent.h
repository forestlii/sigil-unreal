// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilCombatTeamAgentInterface.h"
#include "Components/ActorComponent.h"
#include "SigilCombatTeamAgentComponent.generated.h"

/**
 * Component for managing combat team affiliations.
 * 管理战斗队伍归属的组件。
 */
UCLASS(ClassGroup=(GCS), meta=(BlueprintSpawnableComponent), AutoExpandCategories=(GCS))
class SIGILCOMBAT_API USigilCombatTeamAgentComponent : public UActorComponent, public ISigilCombatTeamAgentInterface
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	USigilCombatTeamAgentComponent();

	/**
	 * Retrieves lifetime replicated properties.
	 * 获取生命周期复制属性。
	 * @param OutLifetimeProps The lifetime properties. 生命周期属性。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Gets the delegate for team ID changes.
	 * 获取队伍ID更改的委托。
	 * @return The team ID changed delegate. 队伍ID更改委托。
	 */
	virtual FSigilCombatTeamIdChangedSignature* GetOnTeamIdChangedDelegate() override;

	/**
	 * Gets the current combat team ID.
	 * 获取当前战斗队伍ID。
	 * @return The combat team ID. 战斗队伍ID。
	 */
	virtual FGenericTeamId GetCombatTeamId_Implementation() const override;

	/**
	 * Sets the combat team ID.
	 * 设置战斗队伍ID。
	 * @param NewTeamId The new team ID. 新队伍ID。
	 */
	virtual void SetCombatTeamId_Implementation(FGenericTeamId NewTeamId) override;

	/**
	 * Handles replication of the combat team ID.
	 * 处理战斗队伍ID的复制。
	 * @param OldTeamID The previous team ID. 旧队伍ID。
	 */
	UFUNCTION()
	void OnRep_CombatTeamId(FGenericTeamId OldTeamID);

	/**
	 * Delegate for team ID change events.
	 * 队伍ID更改事件的委托。
	 */
	UPROPERTY(BlueprintAssignable, Category="GCS")
	FSigilCombatTeamIdChangedSignature OnTeamIdChangedEvent;

protected:
	/**
	 * Called when the game starts.
	 * 游戏开始时调用。
	 */
	virtual void BeginPlay() override;

	/**
	 * The current team ID of this agent.
	 * 此代理的当前队伍ID。
	 */
	UPROPERTY(EditAnywhere, Category="GCS", ReplicatedUsing=OnRep_CombatTeamId)
	FGenericTeamId CombatTeamId;

	/**
	 * Whether to assign the team ID to the controller.
	 * 是否将队伍ID分配给控制器。
	 */
	UPROPERTY(EditAnywhere, Category="GCS")
	bool bAssignTeamIdToController{true};

public:
	/**
	 * Called every frame.
	 * 每帧调用。
	 * @param DeltaTime Time since last frame. 上一帧以来的时间。
	 * @param TickType The type of tick. tick类型。
	 * @param ThisTickFunction The tick function. tick函数。
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};