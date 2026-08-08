// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Exchange/GIS_CurrencyDefinition.h"

#include "GIS_InventorySubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CurrencyDefinition)

FGIS_CurrencyExchangeRate::FGIS_CurrencyExchangeRate(const UGIS_CurrencyDefinition* InCurrency, float InExchangeRate)
{
	Currency = InCurrency;
	ExchangeRate = InExchangeRate;
}


bool UGIS_CurrencyDefinition::TryGetExchangeRateTo(const UGIS_CurrencyDefinition* OtherCurrency, double& ExchangeRate) const
{
	if (OtherCurrency == nullptr)
	{
		ExchangeRate = -1;
		return false;
	}

	//same currency
	if (OtherCurrency == this)
	{
		ExchangeRate = 1;
		return true;
	}

	const FGIS_CurrencyExchangeRate RootExchangeRate = GetRootExchangeRate();
	const FGIS_CurrencyExchangeRate OtherRootExchangeRate = OtherCurrency->GetRootExchangeRate();
	if (RootExchangeRate.Currency != OtherRootExchangeRate.Currency)
	{
		ExchangeRate = -1;
		return false;
	}

	ExchangeRate = RootExchangeRate.ExchangeRate / OtherRootExchangeRate.ExchangeRate;
	return true;
}

FGIS_CurrencyExchangeRate UGIS_CurrencyDefinition::GetRootExchangeRate(double AdditionalExchangeRate) const
{
	if (ParentCurrency)
	{
		return ParentCurrency->GetRootExchangeRate(AdditionalExchangeRate * ExchangeRateToParent);
	}
	return FGIS_CurrencyExchangeRate(this, AdditionalExchangeRate);
}
