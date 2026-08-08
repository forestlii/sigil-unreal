// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Async/GIS_AsyncAction_WaitEquipmentSystem.h"

#include "GIS_EquipmentSystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_AsyncAction_WaitEquipmentSystem)

UGIS_AsyncAction_WaitEquipmentSystem* UGIS_AsyncAction_WaitEquipmentSystem::WaitEquipmentSystem(UObject* WorldContext, AActor* TargetActor)
{
	return CreateWaitAction<UGIS_AsyncAction_WaitEquipmentSystem>(WorldContext, TargetActor, 0.5, -1);
}

void UGIS_AsyncAction_WaitEquipmentSystem::OnExecutionAction()
{
	AActor* Actor = GetActor();

	if (UGIS_EquipmentSystemComponent* EquipmentSystem = UGIS_EquipmentSystemComponent::GetEquipmentSystemComponent(Actor))
	{
		Complete();
	}
}

UGIS_AsyncAction_WaitEquipmentSystem* UGIS_AsyncAction_WaitEquipmentSystemInitialized::WaitEquipmentSystemInitialized(UObject* WorldContext, AActor* TargetActor)
{
	return CreateWaitAction<UGIS_AsyncAction_WaitEquipmentSystemInitialized>(WorldContext, TargetActor, 0.5, -1);
}

void UGIS_AsyncAction_WaitEquipmentSystemInitialized::OnExecutionAction()
{
	// Already found.
	UGIS_EquipmentSystemComponent* ExistingOne = EquipmentSystemPtr.IsValid() ? EquipmentSystemPtr.Get() : nullptr;
	if (IsValid(ExistingOne))
	{
		return;
	}

	AActor* Actor = GetActor();
	if (UGIS_EquipmentSystemComponent* EquipmentSys = UGIS_EquipmentSystemComponent::GetEquipmentSystemComponent(Actor))
	{
		if (EquipmentSys->IsEquipmentSystemInitialized())
		{
			Complete();
			return;
		}
		EquipmentSystemPtr = EquipmentSys;
		EquipmentSystemPtr->OnEquipmentSystemInitializedEvent.AddDynamic(this, &ThisClass::OnSystemInitialized);
	}
}

void UGIS_AsyncAction_WaitEquipmentSystemInitialized::Cleanup()
{
	Super::Cleanup();
	UGIS_EquipmentSystemComponent* ExistingOne = EquipmentSystemPtr.IsValid() ? EquipmentSystemPtr.Get() : nullptr;

	if (IsValid(ExistingOne))
	{
		ExistingOne->OnEquipmentSystemInitializedEvent.RemoveAll(this);
	}
}

void UGIS_AsyncAction_WaitEquipmentSystemInitialized::OnSystemInitialized()
{
	Complete();
}
