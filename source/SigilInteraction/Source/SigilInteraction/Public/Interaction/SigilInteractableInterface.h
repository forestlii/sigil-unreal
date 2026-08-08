// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SigilInteractableInterface.generated.h"

/**
 * Interface for actors to handle interaction events.
 * 处理交互事件的演员接口。
 */
UINTERFACE()
class SIGILINTERACTION_API USigilInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implementation class for interactable actors.
 * 可交互演员的实现类。
 */
class SIGILINTERACTION_API ISigilInteractableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the display name for the interactable actor.
	 * 获取可交互演员的显示名称。
	 * @return The display name. 显示名称。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GGS|Interaction")
	FText GetInteractionDisplayName() const;
	virtual FText GetInteractionDisplayNameText_Implementation() const;

	/**
	 * Called when the actor is selected by the interaction system.
	 * 演员被交互系统选中时调用。
	 * @param Instigator The instigating actor, usually the player. 发起者，通常是玩家。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GGS|Interaction")
	void OnInteractionSelected(AActor* Instigator);

	/**
	 * Called when the actor is deselected by the interaction system.
	 * 演员被交互系统取消选中时调用。
	 * @param Instigator The instigating actor. 发起者。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GGS|Interaction")
	void OnInteractionDeselected(AActor* Instigator);

	/**
	 * Called when interaction with the actor starts.
	 * 与演员交互开始时调用。
	 * @param Instigator The instigating actor. 发起者。
	 * @param Index The interaction option index. 交互选项索引。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GGS|Interaction")
	void OnInteractionStarted(AActor* Instigator, int32 Index);

	/**
	 * Called when interaction with the actor ends.
	 * 与演员交互结束时调用。
	 * @param Instigator The instigating actor. 发起者。
	 * @param Index The interaction option index. 交互选项索引。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GGS|Interaction")
	void OnInteractionEnded(AActor* Instigator, int32 Index);

	/**
	 * Called when an interaction option is selected.
	 * 交互选项被选中时调用。
	 * @param Instigator The instigating actor. 发起者。
	 * @param OptionIndex The selected option index. 选中的选项索引。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GGS|Interaction")
	void OnInteractionOptionSelected(AActor* Instigator, int32 OptionIndex);
};