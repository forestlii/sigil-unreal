// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "SigilAbilitySet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSigilAbilitySet, Log, All)

class UGameplayEffect;
class UGameplayAbility;
class UGAbilitySystemComponent;
class UObject;

/**
 * Struct for granting gameplay abilities in an ability set.
 * 用于在技能集赋予游戏技能的结构体。
 */
USTRUCT(BlueprintType)
struct SIGILGAS_API FSigilAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * The gameplay ability class to grant.
	 * 要赋予的技能类。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA", BlueprintReadWrite, meta=(AllowAbstract=false))
	TSoftClassPtr<UGameplayAbility> Ability = nullptr;

	/**
	 * The level of the ability to grant.
	 * 要赋予的技能等级。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA", BlueprintReadWrite)
	int32 AbilityLevel = 1;

	/**
	 * The input ID for activating/cancelling the ability.
	 * 用于激活/取消技能的输入ID。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA", BlueprintReadWrite)
	int32 InputID = -1;

	/**
	 * Dynamic tags to add to the ability.
	 * 添加到技能的动态标签。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA", BlueprintReadWrite)
	FGameplayTagContainer DynamicTags;

#if WITH_EDITORONLY_DATA
	/**
	 * Generates an editor-friendly name.
	 * 生成编辑器友好的名称。
	 */
	void MakeEditorFriendlyName();
	
	/**
	 * Editor-friendly name for the ability.
	 * 技能的编辑器友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;

	/**
	 * Toggles whether the ability is granted (editor-only, for debugging).
	 * 切换是否赋予技能（仅编辑器，用于调试）。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA")
	bool bAbilityEnabled{true};
#endif
};

/**
 * Struct for granting gameplay effects in an ability set.
 * 用于在技能集赋予游戏效果的结构体。
 */
USTRUCT(BlueprintType)
struct SIGILGAS_API FSigilAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	/**
	 * The gameplay effect class to grant.
	 * 要赋予的效果类。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GGA")
	TSoftClassPtr<UGameplayEffect> GameplayEffect = nullptr;

	/**
	 * The level of the gameplay effect to grant.
	 * 要赋予的效果等级。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GGA")
	float EffectLevel = 1.0f;

#if WITH_EDITORONLY_DATA
	/**
	 * Generates an editor-friendly name.
	 * 生成编辑器友好的名称。
	 */
	void MakeEditorFriendlyName();
	
	/**
	 * Editor-friendly name for the effect.
	 * 效果的编辑器友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;

	/**
	 * Toggles whether the effect is granted (editor-only, for debugging).
	 * 切换是否赋予效果（仅编辑器，用于调试）。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA")
	bool bEffectEnabled{true};
#endif
};

/**
 * Struct for granting attribute sets in an ability set.
 * 用于在技能集赋予属性集的结构体。
 */
USTRUCT(BlueprintType)
struct SIGILGAS_API FSigilAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	/**
	 * The attribute set class to grant.
	 * 要赋予的属性集类。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GGA")
	TSoftClassPtr<UAttributeSet> AttributeSet;

#if WITH_EDITORONLY_DATA
	/**
	 * Generates an editor-friendly name.
	 * 生成编辑器友好的名称。
	 */
	void MakeEditorFriendlyName();
	
	/**
	 * Editor-friendly name for the attribute set.
	 * 属性集的编辑器友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;

	/**
	 * Toggles whether the attribute set is granted (editor-only, for debugging).
	 * 切换是否赋予属性集（仅编辑器，用于调试）。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA")
	bool bAttributeSetEnabled{true};
#endif
};

/**
 * Struct for storing handles to granted abilities, effects, and attribute sets.
 * 存储已赋予的技能、效果和属性集句柄的结构体。
 */
USTRUCT(BlueprintType)
struct SIGILGAS_API FSigilAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	/**
	 * Adds an ability spec handle.
	 * 添加技能规格句柄。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 */
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);

	/**
	 * Adds a gameplay effect handle.
	 * 添加游戏效果句柄。
	 * @param Handle The gameplay effect handle. 游戏效果句柄。
	 */
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);

	/**
	 * Adds an attribute set.
	 * 添加属性集。
	 * @param Set The attribute set to add. 要添加的属性集。
	 */
	void AddAttributeSet(UAttributeSet* Set);

	/**
	 * Removes granted items from an ability system component.
	 * 从技能系统组件移除已赋予的项。
	 * @param ASC The ability system component. 技能系统组件。
	 */
	void TakeFromAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	/**
	 * Handles to granted abilities.
	 * 已赋予的技能句柄。
	 */
	UPROPERTY(BlueprintReadOnly, Category="GGA")
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	/**
	 * Handles to granted gameplay effects.
	 * 已赋予的游戏效果句柄。
	 */
	UPROPERTY(BlueprintReadOnly, Category="GGA")
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	/**
	 * Pointers to granted attribute sets.
	 * 已赋予的属性集指针。
	 */
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/**
 * Data asset for granting gameplay abilities, effects, and attribute sets.
 * 用于赋予游戏技能、效果和属性集的数据资产。
 */
UCLASS(BlueprintType, Const)
class SIGILGAS_API USigilAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the ability set.
	 * 技能集构造函数。
	 */
	USigilAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Grants the ability set to an ability system component.
	 * 将技能集赋予技能系统组件。
	 * @param ASC The ability system component. 技能系统组件。
	 * @param OutGrantedHandles Handles for granted items (output). 已赋予项的句柄（输出）。
	 * @param SourceObject Optional source object. 可选的源对象。
	 * @param OverrideLevel Optional level override. 可选的等级覆盖。
	 */
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, FSigilAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr, int32 OverrideLevel = -1) const;

	/**
	 * Grants an ability set to an ability system component (static).
	 * 将技能集赋予技能系统组件（静态）。
	 * @param AbilitySet The ability set to grant. 要赋予的技能集。
	 * @param ASC The ability system component. 技能系统组件。
	 * @param SourceObject Optional source object. 可选的源对象。
	 * @param OverrideLevel Optional level override. 可选的等级覆盖。
	 * @return Handles for granted items. 已赋予项的句柄。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySet", BlueprintAuthorityOnly)
	static FSigilAbilitySet_GrantedHandles GiveAbilitySetToAbilitySystem(TSoftObjectPtr<USigilAbilitySet> AbilitySet, UAbilitySystemComponent* ASC, UObject* SourceObject, int32 OverrideLevel = -1);

	/**
	 * Removes granted ability sets from an ability system component.
	 * 从技能系统组件移除已赋予的技能集。
	 * @param GrantedHandles Handles for granted items. 已赋予项的句柄。
	 * @param ASC The ability system component. 技能系统组件。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySet", BlueprintAuthorityOnly)
	static void TakeAbilitySetFromAbilitySystem(UPARAM(ref) FSigilAbilitySet_GrantedHandles& GrantedHandles, UAbilitySystemComponent* ASC);

protected:
	/**
	 * Gameplay abilities to grant.
	 * 要赋予的游戏技能。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GGA|AbilitySet", meta=(TitleProperty=EditorFriendlyName, NoElementDuplicate))
	TArray<FSigilAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	/**
	 * Gameplay effects to grant.
	 * 要赋予的游戏效果。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GGA|AbilitySet", meta=(TitleProperty=EditorFriendlyName, NoElementDuplicate))
	TArray<FSigilAbilitySet_GameplayEffect> GrantedGameplayEffects;

	/**
	 * Attribute sets to grant.
	 * 要赋予的属性集。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GGA|AbilitySet", meta=(TitleProperty=EditorFriendlyName, NoElementDuplicate))
	TArray<FSigilAbilitySet_AttributeSet> GrantedAttributes;

#if WITH_EDITOR
	/**
	 * Pre-save processing for editor.
	 * 编辑器预保存处理。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	/**
	 * Handles property changes in the editor.
	 * 处理编辑器中的属性更改。
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * Handles chain property changes in the editor.
	 * 处理编辑器中的链式属性更改。
	 */
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
};