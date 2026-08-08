// // Copyright 2025 RedMoonGames All Rights Reserved.
//
//
// #include "CustomTasks/GGA_CustomAbilityTask.h"
//
// #include "AbilitySystemComponent.h"
// #include "AbilityTasks/GGA_AbilityTask_RunCustomAbilityTask.h"
//
// UWorld* UGGA_CustomAbilityTask::GetWorld() const
// {
// 	if (UObject* Outer = GetOuter())
// 	{
// 		return Outer->GetWorld();
// 	}
// 	if (true)
// 	{
// 	}
// 	return nullptr;
// }
//
// bool UGGA_CustomAbilityTask::IsSupportedForNetworking() const
// {
// 	return true;
// }
//
// void UGGA_CustomAbilityTask::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
// {
// 	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
//
// 	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
// 	{
// 		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
// 	}
// }
//
// FGameplayAbilitySpecHandle UGGA_CustomAbilityTask::GetAbilityHandle() const
// {
// 	return SourceTask->GetAbilitySpecHandle();
// }
//
// UAbilitySystemComponent* UGGA_CustomAbilityTask::GetAbilitySystemComponent() const
// {
// 	if (UAbilitySystemComponent* ASC = SourceTask->AbilitySystemComponent.Get())
// 	{
// 		return ASC;
// 	}
// 	return nullptr;
// }
//
// bool UGGA_CustomAbilityTask::IsPredictingClient() const
// {
// 	return SourceTask->IsPredictingClient();
// }
//
// bool UGGA_CustomAbilityTask::IsForRemoteClient() const
// {
// 	return SourceTask->IsForRemoteClient();
// }
//
// bool UGGA_CustomAbilityTask::IsLocallyControlled() const
// {
// 	return SourceTask->IsLocallyControlled();
// }
//
// void UGGA_CustomAbilityTask::SetSourceTask(UGGA_AbilityTask_RunCustomAbilityTask* InSourceTask)
// {
// 	check(InSourceTask != nullptr);
// 	SourceTask = InSourceTask;
// }
//
// void UGGA_CustomAbilityTask::OnInitSimulatedTask_Implementation()
// {
// }
//
// void UGGA_CustomAbilityTask::BroadcastEvent_Implementation(FGameplayTag EventTag, const FInstancedStruct& Payload, bool bEndTask)
// {
// 	if (SourceTask->ShouldBroadcastAbilityTaskDelegates())
// 	{
// 		SourceTask->OnEvent.Broadcast(EventTag, Payload);
// 	}
// 	if (bEndTask)
// 	{
// 		SourceTask->EndTask();
// 	}
// }
//
// void UGGA_CustomAbilityTask::EndTask()
// {
// 	SourceTask->EndTask();
// }
//
// void UGGA_CustomAbilityTask::OnTaskActivate_Implementation()
// {
// }
//
// void UGGA_CustomAbilityTask::OnTaskTick_Implementation(float DeltaTime)
// {
// }
//
// void UGGA_CustomAbilityTask::OnTaskDestroy_Implementation(bool bInOwnerFinished)
// {
// }
