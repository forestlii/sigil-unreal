// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Pickups/GIS_CurrencyPickupComponent.h"
#include "GIS_CurrencySystemComponent.h"
#include "GameFramework/Actor.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CurrencyPickupComponent)

void UGIS_CurrencyPickupComponent::BeginPlay()
{
	OwningCurrencySystem = UGIS_CurrencySystemComponent::GetCurrencySystemComponent(GetOwner());
	if (OwningCurrencySystem == nullptr)
	{
		GIS_CLOG(Warning, "Mising CurrencySystemComponent!");
	}
	Super::BeginPlay();
}

bool UGIS_CurrencyPickupComponent::Pickup(UGIS_InventorySystemComponent* Picker)
{
	if (!GetOwner()->HasAuthority())
	{
		GIS_CLOG(Warning, "has no authority!");
		return false;
	}
	if (OwningCurrencySystem == nullptr || !IsValid(OwningCurrencySystem))
	{
		GIS_CLOG(Warning, "mising CurrencySystemComponent!");
		return false;
	}
	if (Picker == nullptr || !IsValid(Picker))
	{
		GIS_CLOG(Warning, "passed-in invalid picker.");
		return false;
	}

	UGIS_CurrencySystemComponent* PickerCurrencySystem = Picker->GetCurrencySystem();
	if (PickerCurrencySystem == nullptr)
	{
		GIS_CLOG(Warning, "Picker:%s has no CurrencySystem!", *Picker->GetOwner()->GetName());
		return false;
	}

	if (PickerCurrencySystem->AddCurrencies(OwningCurrencySystem->GetAllCurrencies()))
	{
		OwningCurrencySystem->EmptyCurrencies();
		return true;
	}
	return false;
}
