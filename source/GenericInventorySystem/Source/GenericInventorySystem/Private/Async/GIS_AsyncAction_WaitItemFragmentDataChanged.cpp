// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Async/GIS_AsyncAction_WaitItemFragmentDataChanged.h"
#include "Engine/Engine.h"
#include "UObject/Object.h"
#include "GIS_ItemFragment.h"
#include "GIS_ItemInstance.h"
#include "GIS_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_AsyncAction_WaitItemFragmentDataChanged)

UGIS_AsyncAction_WaitItemFragmentDataChanged* UGIS_AsyncAction_WaitItemFragmentDataChanged::WaitItemFragmentStateChanged(UObject* WorldContext, UGIS_ItemInstance* ItemInstance,
                                                                                                                         TSoftClassPtr<UGIS_ItemFragment> FragmentClass)
{
	if (!IsValid(WorldContext))
	{
		GIS_LOG(Warning, "invalid world context!")
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World))
	{
		GIS_LOG(Warning, "can't get world from context:%s", *GetNameSafe(WorldContext));
		return nullptr;
	}

	if (!IsValid(ItemInstance))
	{
		GIS_LOG(Warning, "invalid item instance");
		return nullptr;
	}

	TSubclassOf<UGIS_ItemFragment> Class = FragmentClass.LoadSynchronous();
	if (Class == nullptr)
	{
		GIS_LOG(Warning, "invalid fragment class");
		return nullptr;
	}

	UGIS_AsyncAction_WaitItemFragmentDataChanged* NewAction = NewObject<UGIS_AsyncAction_WaitItemFragmentDataChanged>(GetTransientPackage(), StaticClass());
	NewAction->ItemInstance = ItemInstance;
	NewAction->FragmentClass = Class;
	NewAction->RegisterWithGameInstance(World->GetGameInstance());
	return NewAction;
}

void UGIS_AsyncAction_WaitItemFragmentDataChanged::Activate()
{
	UGIS_ItemInstance* Item = ItemInstance.IsValid() ? ItemInstance.Get() : nullptr;
	if (IsValid(Item))
	{
		Item->OnFragmentStateAddedEvent.AddDynamic(this, &ThisClass::OnFragmentStateChanged);
		Item->OnFragmentStateUpdatedEvent.AddDynamic(this, &ThisClass::OnFragmentStateChanged);
	}
}

void UGIS_AsyncAction_WaitItemFragmentDataChanged::Cancel()
{
	Super::Cancel();
	UGIS_ItemInstance* Item = ItemInstance.IsValid() ? ItemInstance.Get() : nullptr;
	if (IsValid(Item))
	{
		Item->OnFragmentStateAddedEvent.RemoveAll(this);
		Item->OnFragmentStateUpdatedEvent.RemoveAll(this);
		ItemInstance.Reset();
		FragmentClass = nullptr;
	}
}

void UGIS_AsyncAction_WaitItemFragmentDataChanged::OnFragmentStateChanged(const UGIS_ItemFragment* Fragment, const FInstancedStruct& State)
{
	if (ShouldBroadcastDelegates())
	{
		if (Fragment != nullptr && Fragment->GetClass() == FragmentClass)
		{
			OnStateChanged.Broadcast(Fragment, State);
		}
	}
}
