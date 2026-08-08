// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Drops/GIS_CurrencyDropper.h"

#include "GIS_CurrencySystemComponent.h"
#include "GIS_LogChannels.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CurrencyDropper)

void UGIS_CurrencyDropper::BeginPlay()
{
	MyCurrency = UGIS_CurrencySystemComponent::GetCurrencySystemComponent(GetOwner());
	if (MyCurrency == nullptr)
	{
		GIS_CLOG(Warning, "Mising currency system component!");
	}
	Super::BeginPlay();
}

void UGIS_CurrencyDropper::Drop()
{
	if (AActor* PickupActor = CreatePickupActorInstance())
	{
		if (UGIS_CurrencySystemComponent* CurrencySys = UGIS_CurrencySystemComponent::GetCurrencySystemComponent(PickupActor))
		{
			CurrencySys->SetCurrencies(MyCurrency->GetAllCurrencies());
		}
	}
}


#if WITH_EDITOR
EDataValidationResult UGIS_CurrencyDropper::IsDataValid(FDataValidationContext& Context) const
{
	if (PickupActorClass.IsNull())
	{
		Context.AddError(FText::FromString(FString::Format(TEXT("%s has no pickup actor class.!"), {*GetName()})));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
