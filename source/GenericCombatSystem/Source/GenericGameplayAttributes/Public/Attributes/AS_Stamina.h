// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "AS_Stamina.generated.h"

namespace AS_Stamina
{
	
	GENERICGAMEPLAYATTRIBUTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina)
	
	GENERICGAMEPLAYATTRIBUTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxStamina)
	
	
	GENERICGAMEPLAYATTRIBUTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(IncomingHealing)
	
	GENERICGAMEPLAYATTRIBUTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(IncomingDamage)
		
}

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class GENERICGAMEPLAYATTRIBUTES_API UAS_Stamina : public UAttributeSet
{
	GENERATED_BODY()


public:

	UAS_Stamina();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	// Current stamina of an actor.(actor的当前生命值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Attribute|StaminaSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Stamina{ 100 };
	ATTRIBUTE_ACCESSORS(ThisClass, Stamina)
	
	// Max stamina value of an actor.(actor的最大生命值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Attribute|StaminaSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxStamina{ 100 };
	ATTRIBUTE_ACCESSORS(ThisClass, MaxStamina)
	

	
	// Incoming healing. This is mapped directly to +Stamina.(即将到来的恢复值，映射为+Stamina)
	UPROPERTY(BlueprintReadOnly,Category = "Attribute|StaminaSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingHealing{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingHealing)
	
	// Incoming damage. This is mapped directly to -Stamina(即将到来的伤害值，映射为-Stamina)
	UPROPERTY(BlueprintReadOnly,Category = "Attribute|StaminaSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingDamage{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingDamage)
	

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetStaminaAttribute"), Category = "Attribute|StaminaSet")
	static FGameplayAttribute Bp_GetStaminaAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetStamina"), Category = "Attribute|StaminaSet")
	float Bp_GetStamina() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetStamina"), Category = "Attribute|StaminaSet")
	void Bp_SetStamina(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitStamina"), Category = "Attribute|StaminaSet")
	void Bp_InitStamina(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetMaxStaminaAttribute"), Category = "Attribute|StaminaSet")
	static FGameplayAttribute Bp_GetMaxStaminaAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetMaxStamina"), Category = "Attribute|StaminaSet")
	float Bp_GetMaxStamina() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetMaxStamina"), Category = "Attribute|StaminaSet")
	void Bp_SetMaxStamina(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitMaxStamina"), Category = "Attribute|StaminaSet")
	void Bp_InitMaxStamina(float NewValue);

	

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetIncomingHealingAttribute"), Category = "Attribute|StaminaSet")
	static FGameplayAttribute Bp_GetIncomingHealingAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetIncomingHealing"), Category = "Attribute|StaminaSet")
	float Bp_GetIncomingHealing() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetIncomingHealing"), Category = "Attribute|StaminaSet")
	void Bp_SetIncomingHealing(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetIncomingDamageAttribute"), Category = "Attribute|StaminaSet")
	static FGameplayAttribute Bp_GetIncomingDamageAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetIncomingDamage"), Category = "Attribute|StaminaSet")
	float Bp_GetIncomingDamage() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetIncomingDamage"), Category = "Attribute|StaminaSet")
	void Bp_SetIncomingDamage(float NewValue);

		
protected:

	/** Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes. (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before) */
	virtual void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	
};