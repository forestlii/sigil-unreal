// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Async/AbilityAsync.h"
#include "SigilAsyncTask_AttributeChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayAttribute, Attribute, float, NewValue, float, OldValue);

/**
 * 蓝图节点，用于监听AbilitySystemComponent上的属性变化。
 * 一般用于UI。
 */
UCLASS()
class SIGILGAS_API USigilAsyncTask_AttributeChanged : public UAbilityAsync
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChanged OnAttributeChanged;

	// Listens for an attribute changing.
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (BlueprintInternalUseOnly = "true"))
	static USigilAsyncTask_AttributeChanged* ListenForAttributeChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute);

	// Listens for an attribute changing.
	// Version that takes in an array of Attributes. Check the Attribute output for which Attribute changed.
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (BlueprintInternalUseOnly = "true"))
	static USigilAsyncTask_AttributeChanged* ListenForAttributesChange(UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAttribute> Attributes);

	// You must call this function manually when you want the AsyncTask to end.
	// For UMG Widgets, you would call it in the Widget's Destruct event.
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta=(DeprecatedFunction, DeprecationMessage="Use EndAction"))
	void EndTask();

protected:
	virtual void Activate() override;
	virtual void EndAction() override;

	FGameplayAttribute AttributeToListenFor;
	TArray<FGameplayAttribute> AttributesToListenFor;

	void AttributeChanged(const FOnAttributeChangeData& Data);
};
