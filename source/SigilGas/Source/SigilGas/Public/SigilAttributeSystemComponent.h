// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilGameplayAttributeStructLibrary.h"
#include "Components/ActorComponent.h"
#include "SigilAttributeSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSigilOnPostAttributeChangeSignature, UAttributeSet*, AttributeSet, const FGameplayAttribute&, Attribute, const float, OldValue, const float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSigilOnAttributeChangedSignature, UAttributeSet*, AttributeSet, const FGameplayAttribute&, Attribute, const float, NewValue, float, PrevValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSigilOnPostGameplayEffectExecuteSignature, const FSigilGameplayEffectModCallbackData&, Data);

/**
 * The main component for reacting to changes in gameplay attributes.
 * 对游戏属性变化做出反应的主要组件。
 */
UCLASS(ClassGroup=(GGA), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class SIGILGAS_API USigilAttributeSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USigilAttributeSystemComponent();

	void ReceivePreAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float& NewValue);

	void ReceivePostAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float OldValue, float NewValue);

	bool ReceivePreGameplayEffectExecute(UAttributeSet* AttributeSet, FGameplayEffectModCallbackData& Data);

	void ReceivePostGameplayEffectExecute(UAttributeSet* AttributeSet, const FGameplayEffectModCallbackData& Data);

	void ReceiveAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, const float& NewValue, const float& OldValue);


	UPROPERTY(BlueprintAssignable, Category="Event")
	FSigilOnPostGameplayEffectExecuteSignature OnPostGameplayEffectExecute;

	UPROPERTY(BlueprintAssignable, Category="Event")
	FSigilOnPostAttributeChangeSignature OnPostAttributeChange;

	/**
	 * Called whenever an attribute changed, And this event will be fired on both serve and client.
	 * 属性发生任意变化后调用，此事件会在服务端和客户端都进行调用。
	 */
	UPROPERTY(BlueprintAssignable, Category="Event")
	FSigilOnAttributeChangedSignature OnAttributeChanged;

protected:
	/**
	 *	Called just before any modification happens to an attribute. This is lower level than PreAttributeChange/PostAttributeChange.
	 *	There is no additional context provided here since anything can trigger this. Executed effects, duration based effects, effects being removed, immunity being applied, stacking rules changing, etc.
	 *	This function is meant to enforce things like "Health = Clamp(Health, 0, MaxHealth)" and NOT things like "trigger this extra thing if damage is applied, etc".
	 *	You should return your modified new value.
	 *	
	 *  在对属性进行任何修改之前调用。它比 PreAttributeChange/PostAttributeChange 更底层。
	 *  这里没有提供额外的上下文，因为任何事情都可能触发它。已执行的效果、基于持续时间的效果、效果被移除、豁免被应用、堆叠规则改变等。
	 *  该函数旨在执行 “Health = Clamp(Health,0,MaxHealth) ”之类的操作，而不是 “如果受到伤害，则触发该额外操作 ”之类的操作。
	 *	你应返回修改后的新值。	
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GGA")
	float HandlePreAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float NewValue);
	virtual float HandlePreAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float NewValue);

	/**
	 * Called just after any modification happens to an attribute.
	 * 在属性被修改后调用。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GGA")
	void HandlePostAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float OldValue, float NewValue);
	virtual void HandlePostAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float OldValue, float NewValue);

	/**
	 *	Called just after a GameplayEffect is executed to modify the base value of an attribute. No more changes can be made.
	 *	Note this is only called during an 'execute'. E.g., a modification to the 'base value' of an attribute. It is not called during an application of a GameplayEffect, such as a 5 ssecond +10 movement speed buff.
	 *	在执行 GameplayEffect 之后调用，以修改属性的基本值。不能再做任何更改。
	 *  注意只有在 “执行 ”过程中才会调用。例如，修改属性的 “基本值”。在应用 GameplayEffect（如 5 秒 +10 移动速度的缓冲）时不会调用。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GGA")
	void HandlePostGameplayEffectExecute(const FSigilGameplayEffectModCallbackData& Payload);
	virtual void HandlePostGameplayEffectExecute_Implementation(const FSigilGameplayEffectModCallbackData& Payload);


	/**
	 * Called whenever an attribute changed, And this event will be fired on both serve and client.
	 * 属性发生任意变化后调用，此事件会在服务端和客户端都进行调用。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GGA")
	void HandleAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, const float& NewValue, const float& OldValue);
	virtual void HandleAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, const float& NewValue, const float& OldValue);

	// Called when the game starts
	virtual void BeginPlay() override;
};
