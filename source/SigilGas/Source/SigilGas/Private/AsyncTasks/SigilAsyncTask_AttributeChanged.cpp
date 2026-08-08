// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AsyncTasks/SigilAsyncTask_AttributeChanged.h"

USigilAsyncTask_AttributeChanged* USigilAsyncTask_AttributeChanged::ListenForAttributeChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute)
{
	if (!IsValid(AbilitySystemComponent) || !Attribute.IsValid())
	{
		return nullptr;
	}

	USigilAsyncTask_AttributeChanged* WaitForAttributeChangedTask = NewObject<USigilAsyncTask_AttributeChanged>();
	WaitForAttributeChangedTask->SetAbilityActor(AbilitySystemComponent->GetAvatarActor());
	WaitForAttributeChangedTask->AttributeToListenFor = Attribute;

	return WaitForAttributeChangedTask;
}

USigilAsyncTask_AttributeChanged* USigilAsyncTask_AttributeChanged::ListenForAttributesChange(UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAttribute> Attributes)
{
	if (!IsValid(AbilitySystemComponent) || Attributes.IsEmpty())
	{
		return nullptr;
	}

	USigilAsyncTask_AttributeChanged* WaitForAttributeChangedTask = NewObject<USigilAsyncTask_AttributeChanged>();
	WaitForAttributeChangedTask->SetAbilityActor(AbilitySystemComponent->GetAvatarActor());

	WaitForAttributeChangedTask->AttributesToListenFor = Attributes;

	return WaitForAttributeChangedTask;
}

void USigilAsyncTask_AttributeChanged::EndTask()
{
	EndAction();
}

void USigilAsyncTask_AttributeChanged::Activate()
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

void USigilAsyncTask_AttributeChanged::EndAction()
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

void USigilAsyncTask_AttributeChanged::AttributeChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}
