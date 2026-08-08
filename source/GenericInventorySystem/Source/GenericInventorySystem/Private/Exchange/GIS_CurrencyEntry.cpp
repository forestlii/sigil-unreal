// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_CurrencyEntry.h"
#include "GIS_CurrencyDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CurrencyEntry)
FGIS_CurrencyEntry::FGIS_CurrencyEntry()
{
	Definition = nullptr;
	Amount = 0;
}

FGIS_CurrencyEntry::FGIS_CurrencyEntry(const TObjectPtr<const UGIS_CurrencyDefinition>& InDefinition, float InAmount)
{
	Definition = InDefinition;
	Amount = InAmount;
}

FGIS_CurrencyEntry::FGIS_CurrencyEntry(float InAmount, const TObjectPtr<const UGIS_CurrencyDefinition>& InDefinition)
{
	Definition = InDefinition;
	Amount = InAmount;
}

bool FGIS_CurrencyEntry::Equals(const FGIS_CurrencyEntry& Other) const
{
	return Amount == Other.Amount && Definition == Other.Definition;
}

FString FGIS_CurrencyEntry::ToString() const
{
	return FString::Format(TEXT("{0} {1}"), {Definition ? Definition->GetName() : TEXT("None"), Amount});
}

bool FGIS_CurrencyEntry::operator==(const FGIS_CurrencyEntry& Rhs) const
{
	return Equals(Rhs);
}

bool FGIS_CurrencyEntry::operator!=(const FGIS_CurrencyEntry& Rhs) const
{
	return !Equals(Rhs);
}
