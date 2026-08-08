// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SigilAbilityTask_PlayMontageAndWaitForEvent.generated.h"

USTRUCT(BlueprintType)
struct FSigilPlayMontageAndWaitForEventTaskParams
{
	GENERATED_BODY()

	/**
	 * Set to override the name of this task, for later querying
	 * 为此任务设置重载名，以便以后查询
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	FName TaskInstanceName;

	/**
	 * The montage to play on the character
	 * 角色要播放的蒙太奇。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;

	/**
	 * Any gameplay events matching this tag will activate the EventReceived callback. If empty, all events will trigger callback
	 * 任何与此标签匹配的游戏事件都将激活 EventReceived 回调。如果为空，所有事件都将触发回调
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	FGameplayTagContainer EventTags;

	/**
	 * Change to play the montage faster or slower
	 * 更改蒙太奇的播放速度快慢
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	float Rate = 1.f;

	/**
	 * If not empty, named montage section to start from
	 * 如果不为空，则从命名的蒙太奇部分开始播放
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	FName StartSection = NAME_None;

	/**
	 * If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
	 * 如果为 "true"，那么Ability正常结束时，蒙太奇会中止。当该Ability被明确取消时，蒙太奇总是会停止。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	bool bStopWhenAbilityEnds = true;

	/**
	 * Change to modify size of root motion or set to 0 to block it entirely
	 * 更改以修改根运动的大小，或设置为 0 以完全阻止它
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	float AnimRootMotionTranslationScale = 1.f;

	/**
	 * Starting time offset in montage, this will be overridden by StartSection if that is also set
	 * 蒙太奇中的起始时间偏移，如果也设置了 StartSection，则会被 StartSection 覆盖
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	float StartTimeSeconds = 0.f;

	/**
	 * If true, you can receive OnInterrupted after an OnBlendOut started (otherwise OnInterrupted will not fire when interrupted, but you will not get OnComplete).
	 * 如果为 "true"，则可以在 OnBlendOut 开始后接收 OnInterrupted（否则，被中断时不会触发 OnInterrupted，但也不会收到 OnComplete）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGA")
	bool bAllowInterruptAfterBlendOut = false;
};

/**
 * This task combines PlayMontageAndWait and WaitForEvent into one task, so you can wait for multiple types of activations such as from a melee combo
 * Much of this code is copied from one of those two ability tasks
 * This is a good task to look at as an example when creating game-specific tasks
 * It is expected that each game will have a set of game-specific tasks to do what they want
 * 此任务将 PlayMontageAndWait 和 WaitForEvent 合并为一个任务，因此您可以等待多种类型的激活，例如来自近战连击的激活。
 * 本任务的大部分代码都是从这两个能力任务中的一个复制过来的
 * 在创建特定游戏任务时，这是一个很好的任务示例
 * 预计每个游戏都会有一套特定于游戏的任务来完成他们想要做的事情
 */
UCLASS()
class SIGILGAS_API USigilAbilityTask_PlayMontageAndWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	USigilAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer);

	/** Delegate type used, EventTag and Payload may be empty if it came from the montage callbacks */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

	/** The montage completely finished playing */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnCompleted;

	/** The montage started blending out */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnBlendOut;

	/** The montage was interrupted */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnInterrupted;

	/** The ability task was explicitly cancelled by another ability */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnCancelled;

	/** One of the triggering gameplay events happened */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate EventReceived;

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	/** Callback function for when the owning Gameplay Ability is cancelled */
	UFUNCTION()
	void OnGameplayAbilityCancelled();

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);

	/**
	 * See PlayMontageAndWaitForEventExt for details。
	 * 查看 PlayMontageAndWaitForEventExt 以获得更多注释。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USigilAbilityTask_PlayMontageAndWaitForEvent* PlayMontageAndWaitForEvent(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		UAnimMontage* MontageToPlay,
		FGameplayTagContainer EventTags,
		float Rate = 1.f,
		FName StartSection = NAME_None,
		bool bStopWhenAbilityEnds = true,
		float AnimRootMotionTranslationScale = 1.f,
		float StartTimeSeconds = 0.f,
		bool bAllowInterruptAfterBlendOut = false);

	/**
	 * Play a montage and wait for it end. If a gameplay event happens that matches EventTags (or EventTags is empty), the EventReceived delegate will fire with a tag and event data.
	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
	 * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing
	 * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled
	 * 播放蒙太奇并等待结束。如果发生了符合 EventTags 的游戏事件（或 EventTags 为空），EventReceived 委托就会触发，并带有标签和事件数据。
	 * 如果 StopWhenAbilityEnds 为 true，那么如果能力正常结束，蒙太奇将会中止。当能力被明确取消时，蒙太奇始终会停止。
	 * 在正常执行时，蒙太奇混合结束时会调用 OnBlendOut，完全结束时会调用 OnCompleted。
	 * 如果其他蒙太奇覆盖了该功能，则调用 OnInterrupted，如果取消了该功能或任务，则调用 OnCancelled。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USigilAbilityTask_PlayMontageAndWaitForEvent* PlayMontageAndWaitForEventExt(UGameplayAbility* OwningAbility, FSigilPlayMontageAndWaitForEventTaskParams Params);

	/**
	* The Blueprint node for this task, PlayMontageAndWaitForEvent, has some black magic from the plugin that automagically calls Activate()
	* inside of K2Node_LatentAbilityCall as stated in the AbilityTask.h. Ability logic written in C++ probably needs to call Activate() itself manually.
	*/
	virtual void Activate() override;

	/** Called when the ability is asked to cancel from an outside node. What this means depends on the individual task. By default, this does nothing other than ending the task. */
	virtual void ExternalCancel() override;

	virtual FString GetDebugString() const override;

	UFUNCTION(BlueprintCallable, Category="GGA|Tasks")
	void EndTaskByOwner();

protected:
	virtual void OnDestroy(bool AbilityEnded) override;

	/** Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not. */
	bool StopPlayingMontage();

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle InterruptedHandle;
	FDelegateHandle EventHandle;

	/** Montage that is playing */
	UPROPERTY()
	TObjectPtr<UAnimMontage> MontageToPlay;

	/** List of tags to match against gameplay events */
	UPROPERTY()
	FGameplayTagContainer EventTags;

	/** Playback rate */
	UPROPERTY()
	float Rate;

	/** Section to start montage from */
	UPROPERTY()
	FName StartSection;

	/** Modifies how root motion movement to apply */
	UPROPERTY()
	float AnimRootMotionTranslationScale;

	UPROPERTY()
	float StartTimeSeconds;

	/** Rather montage should be aborted if ability ends */
	UPROPERTY()
	bool bStopWhenAbilityEnds;

	UPROPERTY()
	bool bAllowInterruptAfterBlendOut;
};
