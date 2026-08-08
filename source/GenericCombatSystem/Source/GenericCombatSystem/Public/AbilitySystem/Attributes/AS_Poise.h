// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "AS_Poise.generated.h"

namespace AS_Poise
{
	
	GENERICCOMBATSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Poise)
	
	GENERICCOMBATSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxPoise)
	
	GENERICCOMBATSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(PoiseRecover)
	
		
}

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class GENERICCOMBATSYSTEM_API UAS_Poise : public UAttributeSet
{
	GENERATED_BODY()


public:

	UAS_Poise();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	// Current Poise value of an actor.(actor的当前抗打击值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Poise, Category = "Attribute|PoiseSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Poise{ 3 };
	ATTRIBUTE_ACCESSORS(ThisClass, Poise)
	
	// Max Poise value of an actor.(actor的最大抗打击值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoise, Category = "Attribute|PoiseSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxPoise{ 3 };
	ATTRIBUTE_ACCESSORS(ThisClass, MaxPoise)
	
	// How many Poise to recover per second.(每秒恢复抗打击值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PoiseRecover, Category = "Attribute|PoiseSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PoiseRecover{ 1 };
	ATTRIBUTE_ACCESSORS(ThisClass, PoiseRecover)
	

	

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetPoiseAttribute"), Category = "Attribute|PoiseSet")
	static FGameplayAttribute Bp_GetPoiseAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetPoise"), Category = "Attribute|PoiseSet")
	float Bp_GetPoise() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetPoise"), Category = "Attribute|PoiseSet")
	void Bp_SetPoise(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitPoise"), Category = "Attribute|PoiseSet")
	void Bp_InitPoise(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetMaxPoiseAttribute"), Category = "Attribute|PoiseSet")
	static FGameplayAttribute Bp_GetMaxPoiseAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetMaxPoise"), Category = "Attribute|PoiseSet")
	float Bp_GetMaxPoise() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetMaxPoise"), Category = "Attribute|PoiseSet")
	void Bp_SetMaxPoise(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitMaxPoise"), Category = "Attribute|PoiseSet")
	void Bp_InitMaxPoise(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetPoiseRecoverAttribute"), Category = "Attribute|PoiseSet")
	static FGameplayAttribute Bp_GetPoiseRecoverAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetPoiseRecover"), Category = "Attribute|PoiseSet")
	float Bp_GetPoiseRecover() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetPoiseRecover"), Category = "Attribute|PoiseSet")
	void Bp_SetPoiseRecover(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitPoiseRecover"), Category = "Attribute|PoiseSet")
	void Bp_InitPoiseRecover(float NewValue);

	

		
protected:

	/** Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes. (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before) */
	virtual void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	UFUNCTION()
	virtual void OnRep_Poise(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_MaxPoise(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_PoiseRecover(const FGameplayAttributeData& OldValue);
	
};