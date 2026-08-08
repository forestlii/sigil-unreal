// // Copyright 2025 RedMoonGames All Rights Reserved.
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "GameplayAbilitySpecHandle.h"
// #include "GameplayTagContainer.h"
//#include "Runtime/Launch/Resources/Version.h"
//#if ENGINE_MINOR_VERSION < 5
//#include "InstancedStruct.h"
//#else
//#include "StructUtils/InstancedStruct.h"
//#endif
// #include "UObject/Object.h"
// #include "GGA_CustomAbilityTask.generated.h"
//
// class UAbilitySystemComponent;
// class UGGA_AbilityTask_RunCustomAbilityTask;
// /**
//  * Inherited from this class to create ability task within BP/C++.
//  */
// UCLASS(Abstract, BlueprintType, Blueprintable)
// class GENERICGAMEPLAYABILITIES_API UGGA_CustomAbilityTask : public UObject
// {
// 	GENERATED_BODY()
//
// 	friend UGGA_AbilityTask_RunCustomAbilityTask;
//
// public:
// 	virtual UWorld* GetWorld() const override;
// 	virtual bool IsSupportedForNetworking() const override;
// 	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
//
// protected:
// 	/** Returns spec handle for owning ability */
// 	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|CustomAbilityTask")
// 	FGameplayAbilitySpecHandle GetAbilityHandle() const;
//
// 	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|CustomAbilityTask")
// 	UAbilitySystemComponent* GetAbilitySystemComponent() const;
//
// 	/** Returns true if the ability is a locally predicted ability running on a client. Usually this means we need to tell the server something. */
// 	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|CustomAbilityTask")
// 	bool IsPredictingClient() const;
//
// 	/** Returns true if we are executing the ability on the server for a non locally controlled client */
// 	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|CustomAbilityTask")
// 	bool IsForRemoteClient() const;
//
// 	/** Returns true if we are executing the ability on the locally controlled client */
// 	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|CustomAbilityTask")
// 	bool IsLocallyControlled() const;
//
// 	void SetSourceTask(UGGA_AbilityTask_RunCustomAbilityTask* InSourceTask);
//
// 	UFUNCTION(BlueprintNativeEvent, Category="GGA|CustomAbilityTask")
// 	void OnInitSimulatedTask();
//
// 	/** Called to trigger the actual task once the delegates have been set up
// 	*	Note that the default implementation does nothing and you don't have to call it */
// 	UFUNCTION(BlueprintNativeEvent, Category="GGA|CustomAbilityTask")
// 	void OnTaskActivate();
// 	virtual void OnTaskActivate_Implementation();
//
// 	UFUNCTION(BlueprintNativeEvent, Category="GGA|CustomAbilityTask")
// 	void OnTaskTick(float DeltaTime);
// 	virtual void OnTaskTick_Implementation(float DeltaTime);
//
// 	/**
// 	 * End and CleanUp the task - may be called by the task itself or by the task owner if the owner is ending. 
// 	 * since the function internally marks the task as "Pending Kill", and this may interfere with internal BP mechanics
// 	 */
// 	UFUNCTION(BlueprintNativeEvent, Category="GGA|CustomAbilityTask")
// 	void OnTaskDestroy(bool bInOwnerFinished);
//
// 	/**
// 	 * @param EventTag 
// 	 * @param Payload 
// 	 */
// 	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GGA|CustomAbilityTask")
// 	void BroadcastEvent(FGameplayTag EventTag, const FInstancedStruct& Payload, bool bEndTask = false);
//
// 	/** Called explicitly to end the task (usually by the task itself). Calls OnDestroy. 
// 	 *	@NOTE: you need to call EndTask before sending out any "on completed" delegates. 
// 	 *	If you don't the task will still be in an "active" state while the event receivers may
// 	 *	assume it's already "finished" */
// 	UFUNCTION(BlueprintCallable, Category="GGA|CustomAbilityTask")
// 	void EndTask();
//
// 	UPROPERTY()
// 	UGGA_AbilityTask_RunCustomAbilityTask* SourceTask{nullptr};
//
// 	/** If true, this task will receive TickTask calls from TasksComponent */
// 	UPROPERTY()
// 	bool bTickingTask{false};
//
// 	/** Should this task run on simulated clients? This should only be used in rare cases, such as movement tasks. Simulated Tasks do not broadcast their end delegates.  */
// 	UPROPERTY()
// 	bool bSimulatedTask{false};
// };
