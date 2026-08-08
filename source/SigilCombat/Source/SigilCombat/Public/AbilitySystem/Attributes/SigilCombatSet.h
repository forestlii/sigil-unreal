// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "SigilCombatSet.generated.h"

namespace SigilCombatSet
{
	
	SIGILCOMBAT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage)
	
	SIGILCOMBAT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageNegation)
	
	SIGILCOMBAT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GuardDamageNegation)
	
	SIGILCOMBAT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(StaminaDamage)
	
	SIGILCOMBAT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(StaminaDamageNegation)
	
		
}

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class SIGILCOMBAT_API USigilCombatSet : public UAttributeSet
{
	GENERATED_BODY()


public:

	USigilCombatSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	// The damage that will apply to target
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Damage, Category = "Attribute|CombatSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Damage{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, Damage)
	
	// The damage reduction(percentage) for incoming health damage
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageNegation, Category = "Attribute|CombatSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DamageNegation{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, DamageNegation)
	
	// The damage reduction(percentage) for incoming health damage while guarding
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GuardDamageNegation, Category = "Attribute|CombatSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData GuardDamageNegation{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, GuardDamageNegation)
	
	// The stamina damage that will apply to target
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaminaDamage, Category = "Attribute|CombatSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData StaminaDamage{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, StaminaDamage)
	
	// The damage reduction(percentage) for incoming stamina damage
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaminaDamageNegation, Category = "Attribute|CombatSet", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData StaminaDamageNegation{ 0.0 };
	ATTRIBUTE_ACCESSORS(ThisClass, StaminaDamageNegation)
	

	

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetDamageAttribute"), Category = "Attribute|CombatSet")
	static FGameplayAttribute Bp_GetDamageAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetDamage"), Category = "Attribute|CombatSet")
	float Bp_GetDamage() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetDamage"), Category = "Attribute|CombatSet")
	void Bp_SetDamage(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitDamage"), Category = "Attribute|CombatSet")
	void Bp_InitDamage(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetDamageNegationAttribute"), Category = "Attribute|CombatSet")
	static FGameplayAttribute Bp_GetDamageNegationAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetDamageNegation"), Category = "Attribute|CombatSet")
	float Bp_GetDamageNegation() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetDamageNegation"), Category = "Attribute|CombatSet")
	void Bp_SetDamageNegation(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitDamageNegation"), Category = "Attribute|CombatSet")
	void Bp_InitDamageNegation(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetGuardDamageNegationAttribute"), Category = "Attribute|CombatSet")
	static FGameplayAttribute Bp_GetGuardDamageNegationAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetGuardDamageNegation"), Category = "Attribute|CombatSet")
	float Bp_GetGuardDamageNegation() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetGuardDamageNegation"), Category = "Attribute|CombatSet")
	void Bp_SetGuardDamageNegation(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitGuardDamageNegation"), Category = "Attribute|CombatSet")
	void Bp_InitGuardDamageNegation(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetStaminaDamageAttribute"), Category = "Attribute|CombatSet")
	static FGameplayAttribute Bp_GetStaminaDamageAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetStaminaDamage"), Category = "Attribute|CombatSet")
	float Bp_GetStaminaDamage() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetStaminaDamage"), Category = "Attribute|CombatSet")
	void Bp_SetStaminaDamage(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitStaminaDamage"), Category = "Attribute|CombatSet")
	void Bp_InitStaminaDamage(float NewValue);

	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="GetStaminaDamageNegationAttribute"), Category = "Attribute|CombatSet")
	static FGameplayAttribute Bp_GetStaminaDamageNegationAttribute();

	UFUNCTION(BlueprintPure,meta=(DisplayName="GetStaminaDamageNegation"), Category = "Attribute|CombatSet")
	float Bp_GetStaminaDamageNegation() const;

	UFUNCTION(BlueprintCallable,meta=(DisplayName="SetStaminaDamageNegation"), Category = "Attribute|CombatSet")
	void Bp_SetStaminaDamageNegation(float NewValue);

	UFUNCTION(BlueprintCallable,meta=(DisplayName="InitStaminaDamageNegation"), Category = "Attribute|CombatSet")
	void Bp_InitStaminaDamageNegation(float NewValue);

	

		
protected:

	/** Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes. (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before) */
	virtual void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	UFUNCTION()
	virtual void OnRep_Damage(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_DamageNegation(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_GuardDamageNegation(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_StaminaDamage(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	virtual void OnRep_StaminaDamageNegation(const FGameplayAttributeData& OldValue);
	
};