//// Copyright 2025 RedMoonGames All Rights Reserved.
//
//
//#include "AbilityTasks/GGA_AbilityTask_RunCustomAbilityTask.h"
//
//#include "GameplayTasksComponent.h"
//#include "CustomTasks/GGA_CustomAbilityTask.h"
//#include "Net/UnrealNetwork.h"
//
//UGGA_AbilityTask_RunCustomAbilityTask::UGGA_AbilityTask_RunCustomAbilityTask()
//{
//}
//
//void UGGA_AbilityTask_RunCustomAbilityTask::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME(UGGA_AbilityTask_RunCustomAbilityTask, TaskInstance);
//}
//
//UGGA_AbilityTask_RunCustomAbilityTask* UGGA_AbilityTask_RunCustomAbilityTask::RunCustomAbilityTask(UGameplayAbility* OwningAbility, TSoftClassPtr<UGGA_CustomAbilityTask> AbilityTaskClass)
//{
//	if (TSubclassOf<UGGA_CustomAbilityTask> RealClass = AbilityTaskClass.LoadSynchronous())
//	{
//		UGGA_AbilityTask_RunCustomAbilityTask* MyObj = NewAbilityTask<UGGA_AbilityTask_RunCustomAbilityTask>(OwningAbility);
//
//		TObjectPtr<UGGA_CustomAbilityTask> CustomAbilityTask = NewObject<UGGA_CustomAbilityTask>(MyObj, RealClass);
//		CustomAbilityTask->SetSourceTask(MyObj);
//		MyObj->bSimulatedTask = CustomAbilityTask->bSimulatedTask;
//		MyObj->bTickingTask = CustomAbilityTask->bTickingTask;
//		return MyObj;
//	}
//	return nullptr;
//}
//
//void UGGA_AbilityTask_RunCustomAbilityTask::Activate()
//{
//	if (UGameplayTasksComponent* Component = GetGameplayTasksComponent())
//	{
//		if (IsSimulatedTask())
//		{
//			if (Component->IsUsingRegisteredSubObjectList() && Component->IsReadyForReplication())
//			{
//				Component->AddReplicatedSubObject(TaskInstance, COND_SkipOwner);
//			}
//		}
//	}
//	TaskInstance->OnTaskActivate();
//}
//
//void UGGA_AbilityTask_RunCustomAbilityTask::TickTask(float DeltaTime)
//{
//	TaskInstance->OnTaskTick(DeltaTime);
//}
//
//void UGGA_AbilityTask_RunCustomAbilityTask::OnDestroy(bool bInOwnerFinished)
//{
//	if (!bWasSuccessfullyDestroyed)
//	{
//		if (UGameplayTasksComponent* Component = GetGameplayTasksComponent())
//		{
//			if (IsSimulatedTask())
//			{
//				if (Component->IsUsingRegisteredSubObjectList())
//				{
//					Component->RemoveReplicatedSubObject(TaskInstance);
//				}
//			}
//		}
//		TaskInstance->OnTaskDestroy(bInOwnerFinished);
//	}
//	Super::OnDestroy(bInOwnerFinished);
//}
//
//void UGGA_AbilityTask_RunCustomAbilityTask::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
//{
//	Super::InitSimulatedTask(InGameplayTasksComponent);
//	TaskInstance->OnInitSimulatedTask();
//}
