// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Async/GIS_AsyncAction_WaitInventorySystem.h"

#include "GIS_InventorySystemComponent.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_AsyncAction_WaitInventorySystem)

UGIS_AsyncAction_WaitInventorySystem* UGIS_AsyncAction_WaitInventorySystem::WaitInventorySystem(UObject* WorldContext, AActor* TargetActor)
{
	return CreateWaitAction<UGIS_AsyncAction_WaitInventorySystem>(WorldContext, TargetActor, 0.5, -1);
}

void UGIS_AsyncAction_WaitInventorySystem::OnExecutionAction()
{
	AActor* Actor = GetActor();

	if (UGIS_InventorySystemComponent* Inventory = UGIS_InventorySystemComponent::GetInventorySystemComponent(Actor))
	{
		Complete();
	}
}

UGIS_AsyncAction_WaitInventorySystem* UGIS_AsyncAction_WaitInventorySystemInitialized::WaitInventorySystemInitialized(UObject* WorldContext, AActor* TargetActor)
{
	return CreateWaitAction<UGIS_AsyncAction_WaitInventorySystemInitialized>(WorldContext, TargetActor, 0.5, -1);
}

void UGIS_AsyncAction_WaitInventorySystemInitialized::OnExecutionAction()
{
	// Already found.
	UGIS_InventorySystemComponent* ExistingOne = InventorySysPtr.IsValid() ? InventorySysPtr.Get() : nullptr;
	if (IsValid(ExistingOne))
	{
		return;
	}

	AActor* Actor = GetActor();
	if (UGIS_InventorySystemComponent* InventorySys = UGIS_InventorySystemComponent::GetInventorySystemComponent(Actor))
	{
		if (InventorySys->IsInventoryInitialized())
		{
			Complete();
			return;
		}
		InventorySysPtr = InventorySys;
		InventorySysPtr->OnInventorySystemInitializedEvent.AddDynamic(this, &ThisClass::OnSystemInitialized);
	}
}

void UGIS_AsyncAction_WaitInventorySystemInitialized::Cleanup()
{
	Super::Cleanup();
	UGIS_InventorySystemComponent* ExistingOne = InventorySysPtr.IsValid() ? InventorySysPtr.Get() : nullptr;
	if (IsValid(ExistingOne))
	{
		ExistingOne->OnInventorySystemInitializedEvent.RemoveAll(this);
	}
}

void UGIS_AsyncAction_WaitInventorySystemInitialized::OnSystemInitialized()
{
	Complete();
}
