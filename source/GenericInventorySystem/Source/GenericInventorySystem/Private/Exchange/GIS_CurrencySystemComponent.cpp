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

#pragma region Static Calculations (To be continued)
#if 0
bool UGIS_CurrencySystemComponent::AddCurrencyInternal(const FGIS_CurrencyEntry& CurrencyInfo, bool bNotify)
{
	if (!CurrencyInfo.Definition || CurrencyInfo.Amount <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid currency definition was passed."), ELogVerbosity::Warning);
		return false;
	}

	auto discreteCurrencyAmounts = ConvertToDiscrete(CurrencyInfo.Definition, CurrencyInfo.Amount);


	auto currencyAmountCopy = Container.Entries;

	auto added = DiscreteAddition(currencyAmountCopy, discreteCurrencyAmounts);

	SetCurrencyInfos(added);
	return true;
}

int32 UGIS_CurrencySystemComponent::FindIndexWithCurrency(const UGIS_CurrencyDefinition* Currency, const TArray<FGIS_CurrencyEntry>& List)
{
	for (int32 i = 0; i < List.Num(); i++)
	{
		if (List[i].Definition == Currency)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UGIS_CurrencySystemComponent::FindOrCreateCurrencyIndex(const UGIS_CurrencyDefinition* Currency, TArray<FGIS_CurrencyEntry>& Result)
{
	int32 Index = FindIndexWithCurrency(Currency, Result);
	if (Index != INDEX_NONE)
	{
		return Index;
	}

	Result.Add(FGIS_CurrencyEntry(0.0f, Currency));
	return Result.Num() - 1;
}

TArray<FGIS_CurrencyEntry> UGIS_CurrencySystemComponent::DiscreteAddition(const TArray<FGIS_CurrencyEntry>& Lhs, const TArray<FGIS_CurrencyEntry>& Rhs)
{
	TArray<FGIS_CurrencyEntry> Result;
	TArray<FGIS_CurrencyEntry> TempArray = Lhs; // 复制 Lhs 作为初始结果

	for (int32 i = 0; i < Rhs.Num(); i++)
	{
		// 交替使用 Result 和 TempArray 避免重复分配
		TArray<FGIS_CurrencyEntry>& Target = (i % 2 == 0) ? Result : TempArray;
		TempArray = DiscreteAddition(TempArray, Rhs[i].Definition, Rhs[i].Amount);
	}

	// 确保最终结果存储在 Result 中
	if (TempArray != Result)
	{
		Result = TempArray;
	}

	return Result;
}

TArray<FGIS_CurrencyEntry> UGIS_CurrencySystemComponent::DiscreteAddition(const TArray<FGIS_CurrencyEntry>& CurrencyAmounts, const UGIS_CurrencyDefinition* Currency, float Amount)
{
	TArray<FGIS_CurrencyEntry> Result = CurrencyAmounts; // 复制输入数组
	while (true)
	{
		int32 Index = FindOrCreateCurrencyIndex(Currency, Result);

		float Sum = Amount + Result[Index].Amount;
		float Mod = FMath::Fmod(Sum, Currency->MaxAmount + 1.0f);
		Result[Index] = FGIS_CurrencyEntry(Mod, Currency);

		float Overflow = Sum - Mod;
		if (Overflow <= 0.0f)
		{
			break;
		}

		const UGIS_CurrencyDefinition* OverflowCurrency = Currency->OverflowCurrency;
		double Rate;
		if (!OverflowCurrency || !Currency->TryGetExchangeRateTo(OverflowCurrency, Rate))
		{
			return SetAllFractionToMax(CurrencyAmounts, Currency);
		}

		double OverflowDouble = Overflow * Rate; // 使用 Rate 而非 Amount
		int32 OverflowInt = FMath::TruncToInt(OverflowDouble);

		double Diff = OverflowDouble - OverflowInt;
		if (Diff > 0.0)
		{
			TArray<FGIS_CurrencyEntry> DiscreteDiff = ConvertToDiscrete(OverflowCurrency, Diff);
			Result = DiscreteAddition(Result, DiscreteDiff);
		}

		Currency = OverflowCurrency;
		Amount = static_cast<float>(OverflowInt);
	}

	return Result;
}

TArray<FGIS_CurrencyEntry> UGIS_CurrencySystemComponent::SetAllFractionToMax(const TArray<FGIS_CurrencyEntry>& CurrencyAmounts, const UGIS_CurrencyDefinition* Currency)
{
	TArray<FGIS_CurrencyEntry> Result = CurrencyAmounts;

	while (Currency != nullptr)
	{
		int32 Index = FindOrCreateCurrencyIndex(Currency, Result);
		Result[Index] = FGIS_CurrencyEntry(Currency->MaxAmount, Currency);
		Currency = Currency->FractionCurrency;
	}

	return Result;
}

TArray<FGIS_CurrencyEntry> UGIS_CurrencySystemComponent::MaxedOutAmount(const UGIS_CurrencyDefinition* Currency)
{
	TArray<FGIS_CurrencyEntry> Result;
	int32 Index = 0;

	while (Currency != nullptr)
	{
		Result.SetNum(Index + 1);
		Result[Index] = FGIS_CurrencyEntry(Currency->MaxAmount, Currency);
		Index++;
		Currency = Currency->FractionCurrency;
	}

	return Result;
}

TArray<FGIS_CurrencyEntry> UGIS_CurrencySystemComponent::ConvertToDiscrete(const UGIS_CurrencyDefinition* Currency, double Amount)
{
	TArray<FGIS_CurrencyEntry> Result;
	int32 Index = 0;

	TArray<FGIS_CurrencyEntry> OverflowCurrencies = ConvertOverflow(Currency, Amount);

	bool bMaxed = false;
	for (int32 i = OverflowCurrencies.Num() - 1; i >= 0; i--)
	{
		Result.SetNum(Index + 1);
		if (Currency->FractionCurrency == OverflowCurrencies[i].Definition)
		{
			bMaxed = true;
		}
		Result[Index] = OverflowCurrencies[i];
		Index++;
	}

	if (!bMaxed)
	{
		TArray<FGIS_CurrencyEntry> FractionCurrencies = ConvertFraction(Currency, Amount);
		for (int32 i = 0; i < FractionCurrencies.Num(); i++)
		{
			Result.SetNum(Index + 1);
			Result[Index] = FractionCurrencies[i];
			Index++;
		}
	}

	return Result;
}

TArray<FGIS_CurrencyEntry> UGIS_CurrencySystemComponent::ConvertOverflow(const UGIS_CurrencyDefinition* Currency, double Amount)
{
	TArray<FGIS_CurrencyEntry> Result;
	int32 Index = 0;
	const UGIS_CurrencyDefinition* NextCurrency = Currency;

	Amount = FMath::TruncToDouble(Amount);

	while (NextCurrency != nullptr && Amount > 0.0)
	{
		double Mod = fmod(Amount, NextCurrency->MaxAmount + 1.0);
		float IntMod = static_cast<float>(Mod);
		if (IntMod > 0.0f)
		{
			Result.SetNum(Index + 1);
			Result[Index] = FGIS_CurrencyEntry(IntMod, NextCurrency);
			Index++;
		}

		if (Amount - IntMod <= 0.0)
		{
			break;
		}

		double Rate;
		if (!NextCurrency->OverflowCurrency || !NextCurrency->TryGetExchangeRateTo(NextCurrency->OverflowCurrency, Rate))
		{
			return MaxedOutAmount(NextCurrency);
		}

		Amount -= Mod;
		Amount *= Rate;
		NextCurrency = NextCurrency->OverflowCurrency;
	}

	return Result;
}

TArray<FGIS_CurrencyEntry> UGIS_CurrencySystemComponent::ConvertFraction(const UGIS_CurrencyDefinition* Currency, double Amount)
{
	TArray<FGIS_CurrencyEntry> Result;
	if (FMath::IsNearlyZero(fmod(Amount, 1.0)))
	{
		return Result;
	}

	int32 Index = 0;
	const UGIS_CurrencyDefinition* NextCurrency = Currency;
	double DecimalAmount = fmod(Amount, 1.0);

	while (NextCurrency != nullptr && DecimalAmount > 0.0)
	{
		if (!NextCurrency->FractionCurrency)
		{
			break;
		}

		double Rate;
		if (!NextCurrency->TryGetExchangeRateTo(NextCurrency->FractionCurrency, Rate))
		{
			break;
		}

		NextCurrency = NextCurrency->FractionCurrency;
		DecimalAmount *= Rate;
		int32 Floor = static_cast<int32>(FMath::Floor(DecimalAmount));

		if (Floor > 0)
		{
			Result.SetNum(Index + 1);
			Result[Index] = FGIS_CurrencyEntry(static_cast<float>(Floor), NextCurrency);
			Index++;
		}

		DecimalAmount -= Floor;
	}

	return Result;
}
#endif
#pragma endregion
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
