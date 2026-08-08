// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilLocomotionStructLibrary.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimInstance.h"
#include "SigilAnimLayer.generated.h"

class USigilAnimLayer;
class USigilMovementSystemComponent;
class USigilMainAnimInstance;
class APawn;


/**
 * Base class for all anim layer setting.
 * 所有动画层设置的基类。
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, Const, CollapseCategories)
class SIGILMOVEMENT_API USigilAnimLayerSetting : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|AnimationLayer")
	bool GetOverrideAnimLayerClass(TSubclassOf<USigilAnimLayer>& OutLayerClass) const;

	/**
	 * You can override this method to provide custom data validation,If return false, Editor will promote whatever error text you returned.
	 * 你可以覆写此函数以实现自定义的数据校验，如果返回false，那么编辑器会提示你提供的错误信息。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GMS|AnimationLayer", meta=(DisplayName="Is Data Valid"))
	bool K2_IsDataValid(FText& ErrorText) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};


/**
 * Base class for all anim layer.
 * @attention Any class inherited from this class should only be linked to SigilMainAnimInstance(The main anim instance).
 * 所有动画层的基类。
 * @注意从该类继承的任何类都只能链接到 SigilMainAnimInstance（主动画实例）。
 */
UCLASS(BlueprintType, Abstract)
class SIGILMOVEMENT_API USigilAnimLayer : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	TObjectPtr<APawn> PawnOwner;

	UPROPERTY(Transient)
	TObjectPtr<USigilMovementSystemComponent> MSC;

private:
	UPROPERTY(VisibleAnywhere, Category="State", Transient)
	TWeakObjectPtr<USigilMainAnimInstance> Parent;

public:
	USigilAnimLayer();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|AnimationLayer", Meta = (BlueprintThreadSafe, ReturnDisplayName = "Parent"))
	USigilMainAnimInstance* GetParent() const;

	/**
	 * This event should be called when this anim layer is linked to the main anim instance, which is a good place to do something like ‘Beginplay’.
	 * 此事件应该在该动画层被链接到主动画实例时调用，这里适合做一些类似“Beginplay”的事情。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|AnimationLayer")
	void OnLinked();
	virtual void OnLinked_Implementation();

	/**
	 * This event should be called when this anim layer is unlinked from the main anim instance, which is a good place to do something like ‘Endplay’.
	 * 此事件应该在该动画层从主动画实例上取消链接时调用，这里适合做一些类似“Endplay”的事情。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GMS|AnimationLayer")
	void OnUnlinked();
	virtual void OnUnlinked_Implementation();

	/**
	 * This is where you can feed your data(anim asset references etc.) into anim layer. 在这里您可以向动画层输入数据（动画资产引用等）。
	 * @param Setting The setting object for this anim layer, Cast to your desired type. 此动画层的设置对象，可根据需要选择类型。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|AnimationLayer")
	void ApplySetting(const USigilAnimLayerSetting* Setting);
	virtual void ApplySetting_Implementation(const USigilAnimLayerSetting* Setting);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|AnimationLayer")
	void ResetSetting();
	virtual void ResetSetting_Implementation();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;

	/**
	 *  If you need to know whether an anim state node in this anim instance is active (relevant) or not by checking NodeRelevantTags in main anim instance,
	 *  then you need to map the anim state names to GameplayTag.
	 *  如果你需要通过主动画蓝图中的NodeRelevantTags得知该动画实例中的动画状态节点是否激活（相关），那么你需要将动画状态名称映射为GameplayTag
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings", meta=(TitleProperty="Tag"))
	TArray<FSigilAnimStateNameToTag> AnimStateNameToTagMapping;
};
