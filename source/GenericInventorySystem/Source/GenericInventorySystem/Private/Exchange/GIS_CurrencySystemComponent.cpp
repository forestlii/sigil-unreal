// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Exchange/GIS_CurrencySystemComponent.h"
#include "Engine/World.h"
#include "GIS_InventorySubsystem.h"
#include "GameFramework/Actor.h"
#include "UObject/Object.h"
#include "GIS_InventoryTags.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CurrencySystemComponent)

UGIS_CurrencySystemComponent::UGIS_CurrencySystemComponent(): Container(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	bWantsInitializeComponent = true;
}

UGIS_CurrencySystemComponent* UGIS_CurrencySystemComponent::GetCurrencySystemComponent(const AActor* Actor)
{
	return IsValid(Actor) ? Actor->FindComponentByClass<UGIS_CurrencySystemComponent>() : nullptr;
}

void UGIS_CurrencySystemComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Container);
}

void UGIS_CurrencySystemComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		return;
	}

	Container.OwningComponent = this;
}

TArray<FGIS_CurrencyEntry> UGIS_CurrencySystemComponent::GetAllCurrencies() const
{
	TArray<FGIS_CurrencyEntry> Ret;
	for (const FGIS_CurrencyEntry& Item : Container.Entries)
	{
		Ret.Add(FGIS_CurrencyEntry(Item.Definition, Item.Amount));
	}
	return Ret;
}

void UGIS_CurrencySystemComponent::SetCurrencies(const TArray<FGIS_CurrencyEntry>& InCurrencyInfos)
{
	TArray<FGIS_CurrencyEntry> NewEntries = InCurrencyInfos.FilterByPredicate([](const FGIS_CurrencyEntry& Item)
	{
		return Item.Definition != nullptr && Item.Amount > 0;
	});

	Container.Entries.Empty();
	CurrencyMap.Empty();
	Container.Entries = NewEntries;
	for (const FGIS_CurrencyEntry& NewItem : NewEntries)
	{
		CurrencyMap.Add(NewItem.Definition, NewItem.Amount);
	}
	Container.MarkArrayDirty();
}

void UGIS_CurrencySystemComponent::EmptyCurrencies()
{
	Container.Entries.Empty();
	CurrencyMap.Empty();
	Container.MarkArrayDirty();
}

bool UGIS_CurrencySystemComponent::GetCurrency(TSoftObjectPtr<const UGIS_CurrencyDefinition> CurrencyDefinition, FGIS_CurrencyEntry& OutCurrencyInfo) const
{
	if (CurrencyDefinition.IsNull())
	{
		return false;
	}

	const UGIS_CurrencyDefinition* Definition = CurrencyDefinition.LoadSynchronous();

	return GetCurrencyInternal(Definition, OutCurrencyInfo);
}

bool UGIS_CurrencySystemComponent::GetCurrencies(TArray<TSoftObjectPtr<UGIS_CurrencyDefinition>> CurrencyDefinitions, TArray<FGIS_CurrencyEntry>& OutCurrencyInfos) const
{
	TArray<TObjectPtr<const UGIS_CurrencyDefinition>> Definitions;
	for (TSoftObjectPtr<const UGIS_CurrencyDefinition> Currency : CurrencyDefinitions)
	{
		if (const UGIS_CurrencyDefinition* Definition = !Currency.IsNull() ? Currency.LoadSynchronous() : nullptr)
		{
			Definitions.AddUnique(Definition);
		}
	}
	return GetCurrenciesInternal(Definitions, OutCurrencyInfos);
}

bool UGIS_CurrencySystemComponent::AddCurrency(FGIS_CurrencyEntry CurrencyInfo)
{
	return AddCurrencyInternal(CurrencyInfo);
}

bool UGIS_CurrencySystemComponent::RemoveCurrency(FGIS_CurrencyEntry CurrencyInfo)
{
	return RemoveCurrencyInternal(CurrencyInfo);
}

bool UGIS_CurrencySystemComponent::HasCurrency(FGIS_CurrencyEntry CurrencyInfo) const
{
	return HasCurrencyInternal(CurrencyInfo);
}

bool UGIS_CurrencySystemComponent::HasCurrencies(const TArray<FGIS_CurrencyEntry>& CurrencyInfos)
{
	bool bOk = true;
	for (auto& Currency : CurrencyInfos)
	{
		if (!HasCurrencyInternal(Currency))
		{
			bOk = false;
			break;
		}
	}
	return bOk;
}

bool UGIS_CurrencySystemComponent::AddCurrencies(const TArray<FGIS_CurrencyEntry>& CurrencyInfos)
{
	for (const FGIS_CurrencyEntry& Currency : CurrencyInfos)
	{
		AddCurrencyInternal(Currency);
	}
	return true;
}

bool UGIS_CurrencySystemComponent::RemoveCurrencies(const TArray<FGIS_CurrencyEntry>& CurrencyInfos)
{
	for (const FGIS_CurrencyEntry& Currency : CurrencyInfos)
	{
		RemoveCurrencyInternal(Currency);
	}
	return true;
}

void UGIS_CurrencySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	AddInitialCurrencies();
}

void UGIS_CurrencySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UGIS_CurrencySystemComponent::OnCurrencyEntryAdded(const FGIS_CurrencyEntry& Entry, int32 Idx)
{
	CurrencyMap.Add(Entry.Definition, Entry.Amount);
	OnCurrencyChangedEvent.Broadcast(Entry.Definition, 0, Entry.Amount);
}

void UGIS_CurrencySystemComponent::OnCurrencyEntryRemoved(const FGIS_CurrencyEntry& Entry, int32 Idx)
{
	CurrencyMap.Remove(Entry.Definition);
	OnCurrencyChangedEvent.Broadcast(Entry.Definition, Entry.PrevAmount, 0);
}

void UGIS_CurrencySystemComponent::OnCurrencyEntryUpdated(const FGIS_CurrencyEntry& Entry, int32 Idx, float OldAmount, float NewAmount)
{
	CurrencyMap.Emplace(Entry.Definition, NewAmount);
	OnCurrencyChangedEvent.Broadcast(Entry.Definition, OldAmount, NewAmount);
}


void UGIS_CurrencySystemComponent::OnCurrencyChanged(TObjectPtr<const UGIS_CurrencyDefinition> Currency, float OldValue, float NewValue)
{
	OnCurrencyChangedEvent.Broadcast(Currency, OldValue, NewValue);
}


void UGIS_CurrencySystemComponent::AddInitialCurrencies_Implementation()
{
	if (GetOwner()->HasAuthority())
	{
		//TODO check records from save game.
		if (!DefaultCurrencies.IsEmpty())
		{
			AddCurrencies(DefaultCurrencies);
		}
	}
}

bool UGIS_CurrencySystemComponent::GetCurrencyInternal(const TObjectPtr<const UGIS_CurrencyDefinition>& Currency, FGIS_CurrencyEntry& OutCurrencyInfo) const
{
	if (CurrencyMap.Contains(Currency))
	{
		OutCurrencyInfo = FGIS_CurrencyEntry(Currency, CurrencyMap[Currency]);
		return true;
	}
	return false;
}

bool UGIS_CurrencySystemComponent::GetCurrenciesInternal(const TArray<TObjectPtr<const UGIS_CurrencyDefinition>>& Currencies, TArray<FGIS_CurrencyEntry>& OutCurrencies) const
{
	TArray<FGIS_CurrencyEntry> Result;
	for (int32 i = 0; i < Currencies.Num(); i++)
	{
		FGIS_CurrencyEntry Info;
		if (GetCurrencyInternal(Currencies[i], Info))
		{
			Result.Add(Info);
		}
	}

	OutCurrencies = Result;

	return !OutCurrencies.IsEmpty();
}

bool UGIS_CurrencySystemComponent::AddCurrencyInternal(const FGIS_CurrencyEntry& CurrencyInfo, bool bNotify)
{
	if (!CurrencyInfo.Definition || CurrencyInfo.Amount <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid currency definition was passed."), ELogVerbosity::Warning);
		return false;
	}

	for (int32 i = 0; i < Container.Entries.Num(); i++)
	{
		// handle adding to existing value.
		FGIS_CurrencyEntry& Entry = Container.Entries[i];
		if (Entry.Definition == CurrencyInfo.Definition)
		{
			const float OldValue = Entry.Amount;
			const float NewValue = Entry.Amount + CurrencyInfo.Amount;
			Entry.Amount = NewValue;
			OnCurrencyEntryUpdated(Entry, i, OldValue, NewValue);
			Container.MarkItemDirty(Entry);
			return true;
		}
	}

	int32 Idx = Container.Entries.AddDefaulted();
	Container.Entries[Idx] = CurrencyInfo;
	OnCurrencyEntryAdded(Container.Entries[Idx], Idx);
	Container.MarkItemDirty(Container.Entries[Idx]);
	return true;
}

bool UGIS_CurrencySystemComponent::RemoveCurrencyInternal(const FGIS_CurrencyEntry& CurrencyInfo, bool bNotify)
{
	if (!CurrencyInfo.Definition || CurrencyInfo.Amount <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveItem"), ELogVerbosity::Warning);
		return false;
	}

	for (int32 i = 0; i < Container.Entries.Num(); i++)
	{
		FGIS_CurrencyEntry& Entry = Container.Entries[i];
		if (Entry.Definition == CurrencyInfo.Definition)
		{
			if (Entry.Amount <= CurrencyInfo.Amount)
			{
				OnCurrencyEntryRemoved(Entry, i);
				Container.Entries.RemoveAt(i);
				Container.MarkArrayDirty();
			}
			else
			{
				const float OldValue = Entry.Amount;
				const float NewValue = Entry.Amount - CurrencyInfo.Amount;
				Entry.Amount = NewValue;
				OnCurrencyEntryUpdated(Entry, i, OldValue, NewValue);
				Container.MarkItemDirty(Entry);
			}
			return true;
		}
	}
	return false;
}

bool UGIS_CurrencySystemComponent::HasCurrencyInternal(const FGIS_CurrencyEntry& CurrencyInfo) const
{
	if (CurrencyMap.Contains(CurrencyInfo.Definition) && CurrencyInfo.Amount > 0)
	{
		return CurrencyMap[CurrencyInfo.Definition] >= CurrencyInfo.Amount;
	}
	return false;
}
