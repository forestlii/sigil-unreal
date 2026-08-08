// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AsyncTasks/GGA_AsyncTask_AttributeChanged.h"

UGGA_AsyncTask_AttributeChanged* UGGA_AsyncTask_AttributeChanged::ListenForAttributeChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute)
{
	if (!IsValid(AbilitySystemComponent) || !Attribute.IsValid())
	{
		return nullptr;
	}

	UGGA_AsyncTask_AttributeChanged* WaitForAttributeChangedTask = NewObject<UGGA_AsyncTask_AttributeChanged>();
	WaitForAttributeChangedTask->SetAbilityActor(AbilitySystemComponent->GetAvatarActor());
	WaitForAttributeChangedTask->AttributeToListenFor = Attribute;

	return WaitForAttributeChangedTask;
}

UGGA_AsyncTask_AttributeChanged* UGGA_AsyncTask_AttributeChanged::ListenForAttributesChange(UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAttribute> Attributes)
{
	if (!IsValid(AbilitySystemComponent) || Attributes.IsEmpty())
	{
		return nullptr;
	}

	UGGA_AsyncTask_AttributeChanged* WaitForAttributeChangedTask = NewObject<UGGA_AsyncTask_AttributeChanged>();
	WaitForAttributeChangedTask->SetAbilityActor(AbilitySystemComponent->GetAvatarActor());

	WaitForAttributeChangedTask->AttributesToListenFor = Attributes;

	return WaitForAttributeChangedTask;
}

void UGGA_AsyncTask_AttributeChanged::EndTask()
{
	EndAction();
}

void UGGA_AsyncTask_AttributeChanged::Activate()
{
	Super::Activate();
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (AttributeToListenFor.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeToListenFor).AddUObject(this, &ThisClass::AttributeChanged);
		}
		for (const FGameplayAttribute& Attribute : AttributesToListenFor)
		{
			if (Attribute.IsValid())
			{
				ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::AttributeChanged);
			}
		}
	}
	else
	{
		EndAction();
	}
}

void UGGA_AsyncTask_AttributeChanged::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (AttributeToListenFor.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeToListenFor).RemoveAll(this);
		}

		for (FGameplayAttribute Attribute : AttributesToListenFor)
		{
			if (AttributeToListenFor.IsValid())
			{
				ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
			}
		}
	}
	Super::EndAction();
}

void UGGA_AsyncTask_AttributeChanged::AttributeChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}
