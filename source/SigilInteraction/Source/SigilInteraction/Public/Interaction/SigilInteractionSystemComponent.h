// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilInteractionStructLibrary.h"
#include "SmartObjectSubsystem.h"
#include "Components/ActorComponent.h"
#include "SigilInteractionSystemComponent.generated.h"

class UCommonUserWidget;
class UGameplayBehavior;

/**
 * Delegate for interaction events.
 * 交互事件的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractionEventSignature);

/**
 * Delegate for changes in the interactable actor.
 * 可交互演员变更的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractableActorChangedSignature, AActor*, OldActor, AActor*, NewActor);

/**
 * Delegate for changes in the interacting state.
 * 交互状态变更的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractingStateChangedSignature, bool, bInteracting);

/**
 * Delegate for changes in the number of interactable actors.
 * 可交互演员数量变更的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableActorNumChangedSignature, int32, ActorsNum);

/**
 * Component for managing interactions with smart objects.
 * 管理与智能对象交互的组件。
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(GGS), meta=(BlueprintSpawnableComponent))
class SIGILINTERACTION_API USigilInteractionSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the interaction system component.
	 * 交互系统组件构造函数。
	 */
	USigilInteractionSystemComponent();

	/**
	 * Retrieves lifetime replicated properties.
	 * 获取生命周期内同步的属性。
	 * @param OutLifetimeProps The replicated properties. 同步的属性。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Retrieves the interaction system component from an actor.
	 * 从演员获取交互系统组件。
	 * @param Actor The actor to query. 要查询的演员。
	 * @return The interaction system component. 交互系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGS|InteractionSystem", meta=(DefaultToSelf="Actor"))
	static USigilInteractionSystemComponent* GetInteractionSystemComponent(const AActor* Actor);

	/**
	 * Cycles through interactable actors.
	 * 循环切换可交互演员。
	 * @param bNext Whether to cycle to the next actor. 是否切换到下一个演员。
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="GGS|InteractionSystem")
	void CycleInteractableActors(bool bNext);

	/**
	 * Triggers a search for potential interactable actors.
	 * 触发潜在可交互演员的搜索。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GGS|InteractionSystem")
	void SearchInteractableActors();

	/**
	 * Sets a new array of interactable actors.
	 * 设置新的可交互演员数组。
	 * @param NewActors The new interactable actors. 新的可交互演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GGS|InteractionSystem")
	void SetInteractableActors(TArray<AActor*> NewActors);

	/**
	 * Sets the number of interactable actors.
	 * 设置可交互演员的数量。
	 * @param NewNum The new number of interactable actors. 可交互演员的新数量。
	 */
	void SetInteractableActorsNum(int32 NewNum);

	/**
	 * Retrieves the array of interactable actors.
	 * 获取可交互演员数组。
	 * @return The interactable actors. 可交互演员。
	 */
	TArray<AActor*> GetInteractableActors() const { return InteractableActors; }

	/**
	 * Retrieves the number of interactable actors.
	 * 获取可交互演员的数量。
	 * @return The number of interactable actors. 可交互演员数量。
	 */
	int32 GetNumOfInteractableActors() const { return NumsOfInteractableActors; }

	/**
	 * Sets the current interactable actor.
	 * 设置当前可交互演员。
	 * @param InActor The actor to set. 要设置的演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GGS|InteractionSystem")
	void SetInteractableActor(AActor* InActor);

	/**
	 * Retrieves the current interactable actor.
	 * 获取当前可交互演员。
	 * @return The interactable actor. 可交互演员。
	 */
	AActor* GetInteractableActor() const { return InteractableActor; }

	/**
	 * Delegate for when the interactable actor changes.
	 * 可交互演员变更时的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FInteractableActorChangedSignature OnInteractableActorChangedEvent;

	/**
	 * Delegate for when the number of interactable actors changes.
	 * 可交互演员数量变更时的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FInteractableActorNumChangedSignature OnInteractableActorNumChangedEvent;

	/**
	 * Delegate for when the interacting state changes.
	 * 交互状态变更时的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FInteractingStateChangedSignature OnInteractingStateChangedEvent;

	/**
	 * Delegate for when the interaction options change.
	 * 交互选项变更时的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FInteractionEventSignature OnInteractionOptionsChangedEvent;

	/**
	 * Delegate for when a search for interactable actors is triggered.
	 * 触发可交互演员搜索时的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FInteractionEventSignature OnSearchInteractableActorsEvent;

	/**
	 * Retrieves the smart object request filter.
	 * 获取智能对象请求过滤器。
	 * @return The smart object request filter. 智能对象请求过滤器。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GGS|InteractionSystem")
	FSmartObjectRequestFilter GetSmartObjectRequestFilter();
	virtual FSmartObjectRequestFilter GetSmartObjectRequestFilter_Implementation();

	/**
	 * Starts an interaction with the specified option index.
	 * 开始与指定选项索引的交互。
	 * @param NewIndex The interaction option index. 交互选项索引。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GGS|InteractionSystem")
	virtual void StartInteraction(int32 NewIndex = 0);

	/**
	 * Ends the current interaction.
	 * 结束当前交互。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GGS|InteractionSystem")
	virtual void EndInteraction();

	/**
	 * Performs an instant interaction with the specified option index.
	 * 执行与指定选项索引的即时交互。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GGS|InteractionSystem")
	void InstantInteraction(int32 NewIndex = 0);

	/**
	 * Checks if an interaction is in progress.
	 * 检查是否正在进行交互。
	 * @return True if interacting, false otherwise. 如果正在交互返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGS|InteractionSystem")
	bool IsInteracting() const;

	/**
	 * Retrieves the current interacting option index.
	 * 获取当前交互选项索引。
	 * @return The interacting option index. 交互选项索引。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGS|InteractionSystem")
	int32 GetInteractingOption() const;

	/**
	 * Retrieves the current interaction options.
	 * 获取当前交互选项。
	 * @return The interaction options. 交互选项。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGS|InteractionSystem")
	const TArray<FSigilInteractionOption>& GetInteractionOptions() const { return InteractionOptions; }

protected:
	/**
	 * Called when the interactable actor changes.
	 * 可交互演员变更时调用。
	 * @param OldActor The previous interactable actor. 之前的可交互演员。
	 */
	UFUNCTION()
	virtual void OnInteractableActorChanged(AActor* OldActor);

	/**
	 * Called when the number of interactable actors changes.
	 * 可交互演员数量变更时调用。
	 */
	UFUNCTION()
	virtual void OnInteractableActorsNumChanged();

	/**
	 * Called when the potential interactable actors changes.
	 * 可交互演员变更时调用。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GGS|InteractionSystem")
	void OnInteractableActorsChanged();

	/**
	 * Called when a smart object event occurs.
	 * 智能对象事件发生时调用。
	 * @param EventData The smart object event data. 智能对象事件数据。
	 */
	UFUNCTION()
	virtual void OnSmartObjectEventCallback(const FSmartObjectEventData& EventData);

	/**
	 * Called when interaction options change.
	 * 交互选项变更时调用。
	 */
	UFUNCTION()
	virtual void OnInteractionOptionsChanged();

	/**
	 * Called when the interacting option index changes.
	 * 交互选项索引变更时调用。
	 * @param PrevOptionIndex The previous option index. 之前的选项索引。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GGS|InteractionSystem")
	void OnInteractingOptionChanged(int32 PrevOptionIndex);

	/**
	 * Refreshes interaction options based on smart object request results.
	 * 根据智能对象请求结果刷新交互选项。
	 */
	virtual void RefreshOptionsForActor();

	/**
	 * Array of potential interactable actors. Not replicated.
	 * 潜在可交互演员数组。未网络同步。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GGS|InteractionSystem")
	TArray<TObjectPtr<AActor>> InteractableActors;

	/**
	 * Number of potential interactable actors, replicated to owning client.
	 * 潜在可交互演员数量，同步到拥有客户端。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnInteractableActorsNumChanged, Category="GGS|InteractionSystem")
	int32 NumsOfInteractableActors{0};

	/**
	 * Current selected interactable actor, replicated for owner only.
	 * 当前选中的可交互演员，仅针对拥有者同步。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GGS|InteractionSystem", ReplicatedUsing=OnInteractableActorChanged)
	TObjectPtr<AActor> InteractableActor;

	/**
	 * Default filter for searching interactable smart objects.
	 * 搜索可交互智能对象的默认过滤器。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GGS|InteractionSystem")
	FSmartObjectRequestFilter DefaultRequestFilter;

	/**
	 * If checked, whenever potential interactable actors changes, the first actor in the list will be selected as currency interactable actor.
	 * 如果勾选，始终使用潜在交互演员中的第一个作为当前选择。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GGS|InteractionSystem")
	bool bNewActorHasPriority{false};

	/**
	 * Current available interaction options, replicated for owner only.
	 * 当前可用的交互选项，仅针对拥有者同步。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GGS|InteractionSystem", ReplicatedUsing=OnInteractionOptionsChanged)
	TArray<FSigilInteractionOption> InteractionOptions;

	/**
	 * Indicates if an interaction is in progress.
	 * 表示是否正在进行交互。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GGS|InteractionSystem")
	bool bInteracting{false};

	/**
	 * Current interacting option index (-1 if no interaction).
	 * 当前交互选项索引（无交互时为-1）。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GGS|InteractionSystem", ReplicatedUsing=OnInteractingOptionChanged)
	int32 InteractingOption{INDEX_NONE};

	/**
	 * Map of smart object slot handles to delegate handles.
	 * 智能对象槽句柄到委托句柄的映射。
	 */
	TMap<FSmartObjectSlotHandle, FDelegateHandle> SlotCallbacks;
};
