// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "AS_Health.generated.h"

namespace AS_Health
{
	
	GENERICGAMEPLAYATTRIBUTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Health)
	
	GENERICGAMEPLAYATTRIBUTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxHealth)
	
	
	GENERICGAMEPLAYATTRIBUTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(IncomingHealing)
	
	GENERICGAMEPLAYATTRIBUTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(IncomingDamage)
		
}

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class GENERICGAMEPLAYATTRIBUTES_API UAS_Health : public UAttributeSet
{
	GENERATED_BODY()


public:

	UAS_Health();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	// Current health of an actor.(actor的当前生命值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attribute|HealthSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health{ 100 };
	ATTRIBUTE_ACCESSORS(ThisClass, Health)
	
	// Max health value of an actor.(actor的最大生命值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attribute|HealthSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth{ 100 };
	ATTRIBUTE_ACCESSORS(ThisClass, MaxHealth)
	

	
	// Incoming healing. This is mapped directly to +Health.(即将到来的恢复值，映射为+Health)
	UPROPERTY(BlueprintReadOnly,Category = "Attribute|HealthSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingHealing{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingHealing)
	
	// Incoming damage. This is mapped directly to -Health(即将到来的伤害值，映射为-Health)
	UPROPERTY(BlueprintReadOnly,Category = "Attribute|HealthSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingDamage{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingDamage)
	

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetHealthAttribute"), Category = "Attribute|HealthSet")
	static FGameplayAttribute Bp_GetHealthAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetHealth"), Category = "Attribute|HealthSet")
	float Bp_GetHealth() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetHealth"), Category = "Attribute|HealthSet")
	void Bp_SetHealth(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitHealth"), Category = "Attribute|HealthSet")
	void Bp_InitHealth(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetMaxHealthAttribute"), Category = "Attribute|HealthSet")
	static FGameplayAttribute Bp_GetMaxHealthAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetMaxHealth"), Category = "Attribute|HealthSet")
	float Bp_GetMaxHealth() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetMaxHealth"), Category = "Attribute|HealthSet")
	void Bp_SetMaxHealth(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitMaxHealth"), Category = "Attribute|HealthSet")
	void Bp_InitMaxHealth(float NewValue);

	

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetIncomingHealingAttribute"), Category = "Attribute|HealthSet")
	static FGameplayAttribute Bp_GetIncomingHealingAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetIncomingHealing"), Category = "Attribute|HealthSet")
	float Bp_GetIncomingHealing() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetIncomingHealing"), Category = "Attribute|HealthSet")
	void Bp_SetIncomingHealing(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetIncomingDamageAttribute"), Category = "Attribute|HealthSet")
	static FGameplayAttribute Bp_GetIncomingDamageAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetIncomingDamage"), Category = "Attribute|HealthSet")
	float Bp_GetIncomingDamage() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetIncomingDamage"), Category = "Attribute|HealthSet")
	void Bp_SetIncomingDamage(float NewValue);

		
protected:

	/** Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes. (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before) */
	virtual void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	
};