// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GGA_GameplayAttributesHelper.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "UObject/UObjectIterator.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

TMap<FGameplayTag, FGameplayAttribute> UGGA_GameplayAttributesHelper::TagToAttributeMapping = {};
TMap<FGameplayAttribute, FGameplayTag> UGGA_GameplayAttributesHelper::AttributeToTagMapping = {};

const TArray<FGameplayAttribute>& UGGA_GameplayAttributesHelper::GetAllGameplayAttributes()
{
	static TArray<FGameplayAttribute> Attributes = FindGameplayAttributes();
	return Attributes;
}

TArray<FGameplayAttribute> UGGA_GameplayAttributesHelper::FindGameplayAttributes()
{
	TArray<FGameplayAttribute> Attributes;
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		if (Class->IsChildOf(UAttributeSet::StaticClass())/*&& !Class->ClassGeneratedBy*/)
		{
			for (TFieldIterator<FProperty> PropertyIt(Class, EFieldIteratorFlags::ExcludeSuper); PropertyIt;
				 ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				Attributes.Add(FGameplayAttribute(Property));
			}
		}
	}

	return Attributes;
}

void UGGA_GameplayAttributesHelper::RegisterTagToAttribute(FGameplayTag Tag, FGameplayAttribute Attribute)
{
	if (Tag.IsValid() && Attribute.IsValid())
	{
		if (!TagToAttributeMapping.Contains(Tag) && !AttributeToTagMapping.Contains(Attribute))
		{
			TagToAttributeMapping.Emplace(Tag, Attribute);
			AttributeToTagMapping.Emplace(Attribute, Tag);
		}
	}
}

void UGGA_GameplayAttributesHelper::UnregisterTagToAttribute(FGameplayTag Tag, FGameplayAttribute Attribute)
{
	if (Tag.IsValid() && Attribute.IsValid())
	{
		if (TagToAttributeMapping.Contains(Tag) && AttributeToTagMapping.Contains(Attribute))
		{
			TagToAttributeMapping.Remove(Tag);
			AttributeToTagMapping.Remove(Attribute);
		}
	}
}

FGameplayAttribute UGGA_GameplayAttributesHelper::TagToAttribute(FGameplayTag Tag)
{
	return Tag.IsValid() && TagToAttributeMapping.Contains(Tag) ? TagToAttributeMapping[Tag] : FGameplayAttribute();
}

TArray<FGameplayAttribute> UGGA_GameplayAttributesHelper::TagsToAttributes(TArray<FGameplayTag> Tags)
{
	TArray<FGameplayAttribute> Attributes;
	for (int32 i = 0; i < Tags.Num(); i++)
	{
		if (TagToAttributeMapping.Contains(Tags[i]))
		{
			Attributes.Add(TagToAttributeMapping[Tags[i]]);
		}
	}
	return Attributes;
}

FGameplayTag UGGA_GameplayAttributesHelper::AttributeToTag(FGameplayAttribute Attribute)
{
	if (Attribute.IsValid())
	{
		if (AttributeToTagMapping.Contains(Attribute))
		{
			return AttributeToTagMapping[Attribute];
		}
	}
	return FGameplayTag::EmptyTag;
}

TArray<FGameplayTag> UGGA_GameplayAttributesHelper::AttributesToTags(TArray<FGameplayAttribute> Attributes)
{
	TArray<FGameplayTag> Tags;
	for (int32 i = 0; i < Attributes.Num(); i++)
	{
		if (AttributeToTagMapping.Contains(Attributes[i]))
		{
			Tags.Add(AttributeToTagMapping[Attributes[i]]);
		}
	}
	return Tags;
}

bool UGGA_GameplayAttributesHelper::IsTagOfAttribute(FGameplayTag Tag, FGameplayAttribute Attribute)
{
	if (Tag.IsValid() && Attribute.IsValid())
	{
		if (TagToAttributeMapping.Contains(Tag))
		{
			return TagToAttributeMapping[Tag] == Attribute;
		}
	}
	return false;
}

bool UGGA_GameplayAttributesHelper::IsAttributeOfTag(FGameplayAttribute Attribute, FGameplayTag Tag)
{
	if (Tag.IsValid() && Attribute.IsValid())
	{
		if (AttributeToTagMapping.Contains(Attribute))
		{
			return AttributeToTagMapping[Attribute] == Tag;
		}
	}
	return false;
}

void UGGA_GameplayAttributesHelper::SetFloatAttribute(const AActor* Actor, FGameplayAttribute Attribute, float NewValue)
{
	if (UAbilitySystemComponent* Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		Asc->SetNumericAttributeBase(Attribute, NewValue);
	}
}

void UGGA_GameplayAttributesHelper::SetFloatAttributeOnAbilitySystemComponent(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, float NewValue)
{
	if (IsValid(AbilitySystem))
	{
		AbilitySystem->SetNumericAttributeBase(Attribute, NewValue);
	}
}

float UGGA_GameplayAttributesHelper::GetFloatAttributePercentage(const AActor* Actor, FGameplayTag AttributeTagOne, FGameplayTag AttributeTagTwo, bool& bSuccessfullyFoundAttribute)
{
	bool bFoundOne = true;
	bool bFoundTwo = true;
	float One = GetFloatAttribute(Actor, AttributeTagOne, bFoundOne);
	float Two = GetFloatAttribute(Actor, AttributeTagTwo, bFoundTwo);
	bSuccessfullyFoundAttribute = bFoundOne && bFoundTwo;
	if (!bFoundOne || !bFoundTwo || Two == 0)
	{
		return 0;
	}

	return One / Two;
}

float UGGA_GameplayAttributesHelper::GetFloatAttributePercentage_Native(const AActor* Actor, FGameplayAttribute AttributeOne, FGameplayAttribute AttributeTwo, bool& bSuccessfullyFoundAttribute)
{
	bool bFoundOne = true;
	bool bFoundTwo = true;
	float One = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Actor, AttributeOne, bFoundOne);
	float Two = UAbilitySystemBlueprintLibrary::GetFloatAttribute(Actor, AttributeTwo, bFoundTwo);

	bSuccessfullyFoundAttribute = bFoundOne && bFoundTwo;
	
	if (!bFoundOne || !bFoundTwo || Two == 0)
	{
		return 0;
	}

	return One / Two;
}

float UGGA_GameplayAttributesHelper::GetFloatAttribute(const AActor* Actor, FGameplayTag AttributeTag, bool& bSuccessfullyFoundAttribute)
{
	return UAbilitySystemBlueprintLibrary::GetFloatAttribute(Actor, TagToAttribute(AttributeTag), bSuccessfullyFoundAttribute);
}

float UGGA_GameplayAttributesHelper::GetFloatAttributeBase(const AActor* Actor, FGameplayTag AttributeTag, bool& bSuccessfullyFoundAttribute)
{
	return UAbilitySystemBlueprintLibrary::GetFloatAttributeBase(Actor, TagToAttribute(AttributeTag), bSuccessfullyFoundAttribute);
}

float UGGA_GameplayAttributesHelper::GetFloatAttributeFromAbilitySystemComponent(const UAbilitySystemComponent* AbilitySystem, FGameplayTag AttributeTag, bool& bSuccessfullyFoundAttribute)
{
	return UAbilitySystemBlueprintLibrary::GetFloatAttributeFromAbilitySystemComponent(AbilitySystem, TagToAttribute(AttributeTag), bSuccessfullyFoundAttribute);
}

float UGGA_GameplayAttributesHelper::GetFloatAttributeBaseFromAbilitySystemComponent(const UAbilitySystemComponent* AbilitySystem, FGameplayTag AttributeTag, bool& bSuccessfullyFoundAttribute)
{
	return UAbilitySystemBlueprintLibrary::GetFloatAttributeBaseFromAbilitySystemComponent(AbilitySystem, TagToAttribute(AttributeTag), bSuccessfullyFoundAttribute);
}

FString UGGA_GameplayAttributesHelper::GetDebugString()
{
	FString Output;
	Output.Append(TEXT("TagToAttributeMapping:\r"));
	for (auto& Pair : TagToAttributeMapping)
	{
		Output.Append(FString::Format(TEXT("Tag:{0}->Attribute:{1} \r"), {Pair.Key.ToString(), Pair.Value.AttributeName}));
	}
	return Output;
}

FGameplayAttribute UGGA_GameplayAttributesHelper::GetAttributeFromEvaluatedData(const FGameplayModifierEvaluatedData& EvaluatedData)
{
	return EvaluatedData.Attribute;
}

EGameplayModOp::Type UGGA_GameplayAttributesHelper::GetModifierOpFromEvaluatedData(const FGameplayModifierEvaluatedData& EvaluatedData)
{
	return EvaluatedData.ModifierOp;
}

float UGGA_GameplayAttributesHelper::GetMagnitudeFromEvaluatedData(const FGameplayModifierEvaluatedData& EvaluatedData)
{
	return EvaluatedData.Magnitude;
}

float UGGA_GameplayAttributesHelper::GetModifiedAttributeMagnitude(const TArray<FGGA_ModifiedAttribute>& ModifiedAttributes, FGameplayAttribute InAttribute)
{
	float Delta = 0.f;
	for (const FGGA_ModifiedAttribute& Mod : ModifiedAttributes)
	{
		if (Mod.Attribute == InAttribute)
		{
			Delta += Mod.TotalMagnitude;
		}
	}
	return Delta;
}
