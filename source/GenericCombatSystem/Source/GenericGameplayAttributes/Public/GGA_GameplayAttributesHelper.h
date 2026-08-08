// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GGA_GameplayAttributeStructLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GGA_GameplayAttributesHelper.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="GGA Gameplay Attribute Function Library"))
class GENERICGAMEPLAYATTRIBUTES_API UGGA_GameplayAttributesHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Gets all gameplay attributes of all attribute sets (attributes declared in 'UAbilitySystemComponent' not included).
	 * 获取项目中的所有GameplayAttribute.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|GameplayAttribute")
	static const TArray<FGameplayAttribute>& GetAllGameplayAttributes();

private:
	static TArray<FGameplayAttribute> FindGameplayAttributes();

public:
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayAttribute")
	static void RegisterTagToAttribute(FGameplayTag Tag, FGameplayAttribute Attribute);

	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayAttribute")
	static void UnregisterTagToAttribute(FGameplayTag Tag, FGameplayAttribute Attribute);

	/**
	 * Convert gameplay tag to gameplay attribute.
	 * @param Tag The tag to query.
	 * @return The gameplay attribute associated with Tag.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static FGameplayAttribute TagToAttribute(FGameplayTag Tag);

	/**
	 * Convert gameplay tags to gameplay attributes.
	 * @param Tags The attribute tags to query.
	 * @return The gameplay attributes associated with Tags.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static TArray<FGameplayAttribute> TagsToAttributes(TArray<FGameplayTag> Tags);

	/**
	 * Convert gameplay attribute to gameplay tag.
	 * @param Attribute The attribute to query.
	 * @return The gameplay tag associated with Attribute.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static FGameplayTag AttributeToTag(FGameplayAttribute Attribute);

	/**
	 * Convert gameplay attributes to gameplay tags.
	 * @param Attributes The attributes to query.
	 * @return The gameplay tags associated with Attributes.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static TArray<FGameplayTag> AttributesToTags(TArray<FGameplayAttribute> Attributes);

	/**
	 * Check if tag is associated with attribute.
	 * @param Tag Tag to check
	 * @param Attribute Attribute to check
	 * @return true if they are associated.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static bool IsTagOfAttribute(FGameplayTag Tag, FGameplayAttribute Attribute);


	/**
	 * Check if attribute is associated with tag .
	 * @param Attribute Attribute to check
	 * @param Tag Tag to check
	 * @return true if they are associated.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static bool IsAttributeOfTag(FGameplayAttribute Attribute, FGameplayTag Tag);

	/**
	 * Set float attribute for actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayAttribute")
	static void SetFloatAttribute(const AActor* Actor, FGameplayAttribute Attribute, float NewValue);

	UFUNCTION(BlueprintCallable, Category = "GGA|GameplayAttribute", meta=(DisplayName="Set Float Attribute on Asc"))
	static void SetFloatAttributeOnAbilitySystemComponent(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, float NewValue);

	/** Returns the percentage of Attributes from the ability system component belonging to Actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute", meta=(DisplayName="Get Float Attribute Percentage(With Tag)"))
	static float GetFloatAttributePercentage(const AActor* Actor, FGameplayTag AttributeTagOne, FGameplayTag AttributeTagTwo, bool& bSuccessfullyFoundAttribute);

	/** Returns the percentage of Attributes from the ability system component belonging to Actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute", meta=(DisplayName="Get Float Attribute Percentage"))
	static float GetFloatAttributePercentage_Native(const AActor* Actor, FGameplayAttribute AttributeOne, FGameplayAttribute AttributeTwo, bool& bSuccessfullyFoundAttribute);

	/** Returns the value of Attribute from the ability system component belonging to Actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static float GetFloatAttribute(const AActor* Actor, FGameplayTag AttributeTag, bool& bSuccessfullyFoundAttribute);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static float GetFloatAttributeBase(const AActor* Actor, FGameplayTag AttributeTag, bool& bSuccessfullyFoundAttribute);

	/** Returns the value of Attribute from the ability system component AbilitySystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute", meta=(DisplayName="Get Float Attribute from Asc"))
	static float GetFloatAttributeFromAbilitySystemComponent(const UAbilitySystemComponent* AbilitySystem, FGameplayTag AttributeTag, bool& bSuccessfullyFoundAttribute);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute", meta=(DisplayName="Get Float Attribute Base from Asc"))
	static float GetFloatAttributeBaseFromAbilitySystemComponent(const UAbilitySystemComponent* AbilitySystem, FGameplayTag AttributeTag, bool& bSuccessfullyFoundAttribute);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static FString GetDebugString();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static FGameplayAttribute GetAttributeFromEvaluatedData(const FGameplayModifierEvaluatedData& EvaluatedData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static EGameplayModOp::Type GetModifierOpFromEvaluatedData(const FGameplayModifierEvaluatedData& EvaluatedData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayAttribute")
	static float GetMagnitudeFromEvaluatedData(const FGameplayModifierEvaluatedData& EvaluatedData);

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "GGA|GameplayAttribute")
	static float GetModifiedAttributeMagnitude(const TArray<FGGA_ModifiedAttribute>& ModifiedAttributes, FGameplayAttribute InAttribute);

protected:
	static TMap<FGameplayTag, FGameplayAttribute> TagToAttributeMapping;

	static TMap<FGameplayAttribute, FGameplayTag> AttributeToTagMapping;
};
