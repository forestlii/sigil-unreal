// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "SigilManaSet.generated.h"

namespace SigilManaSet
{
	
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mana)
	
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxMana)
	
		
}

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class SIGILGAS_API USigilManaSet : public UAttributeSet
{
	GENERATED_BODY()


public:

	USigilManaSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	// Current mana of an actor.(actor的当前魔法值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Attribute|ManaSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Mana{ 100 };
	ATTRIBUTE_ACCESSORS(ThisClass, Mana)
	
	// Max mana value of an actor.(actor的最大魔法值)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Attribute|ManaSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxMana{ 100 };
	ATTRIBUTE_ACCESSORS(ThisClass, MaxMana)
	

	

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetManaAttribute"), Category = "Attribute|ManaSet")
	static FGameplayAttribute Bp_GetManaAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetMana"), Category = "Attribute|ManaSet")
	float Bp_GetMana() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetMana"), Category = "Attribute|ManaSet")
	void Bp_SetMana(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitMana"), Category = "Attribute|ManaSet")
	void Bp_InitMana(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetMaxManaAttribute"), Category = "Attribute|ManaSet")
	static FGameplayAttribute Bp_GetMaxManaAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetMaxMana"), Category = "Attribute|ManaSet")
	float Bp_GetMaxMana() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetMaxMana"), Category = "Attribute|ManaSet")
	void Bp_SetMaxMana(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitMaxMana"), Category = "Attribute|ManaSet")
	void Bp_InitMaxMana(float NewValue);

	

		
protected:

	/** Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes. (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before) */
	virtual void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldValue);
	
};