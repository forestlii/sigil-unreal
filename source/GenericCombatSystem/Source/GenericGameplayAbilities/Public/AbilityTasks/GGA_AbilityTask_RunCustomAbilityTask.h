//// Copyright 2025 RedMoonGames All Rights Reserved.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Abilities/Tasks/AbilityTask.h"
//#include "Runtime/Launch/Resources/Version.h"
//#if ENGINE_MINOR_VERSION < 5
//#include "InstancedStruct.h"
//#else
//#include "StructUtils/InstancedStruct.h"
//#endif
//#include "GGA_AbilityTask_RunCustomAbilityTask.generated.h"
//
//
//class UGGA_CustomAbilityTask;
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGGA_CustomAbilityTaskDelegate, const FGameplayTag, EventTag, const FInstancedStruct&, Payload);
//
///**
// * A special ability task runs GGA_CustomAbilityTask internally.
// */
//UCLASS()
//class GENERICGAMEPLAYABILITIES_API UGGA_AbilityTask_RunCustomAbilityTask : public UAbilityTask
//{
//	GENERATED_BODY()
//
//public:
//	UGGA_AbilityTask_RunCustomAbilityTask();
//
//	UPROPERTY(BlueprintAssignable)
//	FGGA_CustomAbilityTaskDelegate OnEvent;
//
//	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
//
//	// Like WaitDelay but only delays one frame (tick).
//	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
//	static UGGA_AbilityTask_RunCustomAbilityTask* RunCustomAbilityTask(UGameplayAbility* OwningAbility, TSoftClassPtr<UGGA_CustomAbilityTask> AbilityTaskClass);
//
//	virtual void Activate() override;
//	virtual void TickTask(float DeltaTime) override;
//	virtual void OnDestroy(bool bInOwnerFinished) override;
//
//	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;
//
//private:
//	UPROPERTY(Replicated)
//	TObjectPtr<UGGA_CustomAbilityTask> TaskInstance{nullptr};
//};
