// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SigilAbilityTagRelationshipMapping.generated.h"

/**
 * Struct defining the relationship between different ability tags.
 * 定义不同技能标签之间关系的结构体。
 */
USTRUCT()
struct SIGILGAS_API FSigilAbilityTagRelationship
{
	GENERATED_BODY()

	/**
	 * The ability tag these rules apply to.
	 * 这些规则适用的技能标签。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities")
	FGameplayTag AbilityTag;

	/**
	 * Tags of abilities blocked while this ability is active.
	 * 此技能激活时被阻挡的技能标签。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities")
	FGameplayTagContainer AbilityTagsToBlock;

	/**
	 * Tags of abilities cancelled when this ability is executed.
	 * 此技能执行时被取消的技能标签。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities")
	FGameplayTagContainer AbilityTagsToCancel;

	/**
	 * Tags required on the activating actor/component to activate this ability.
	 * 激活此技能所需的演员/组件标签。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities")
	FGameplayTagContainer ActivationRequiredTags;

	/**
	 * Tags that block activation if present on the activating actor/component.
	 * 如果演员/组件具有这些标签，将阻止激活。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities")
	FGameplayTagContainer ActivationBlockedTags;

#if WITH_EDITORONLY_DATA
	/**
	 * Description for developers in the editor.
	 * 编辑器中用于开发者的描述。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities")
	FString DevDescription;
	
	/**
	 * Editor-friendly name for the relationship.
	 * 关系在编辑器中的友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

/**
 * Struct defining ability tag relationships with a tag query condition.
 * 定义带有标签查询条件的技能标签关系的结构体。
 */
USTRUCT()
struct FSigilAbilityTagRelationshipsWithQuery
{
	GENERATED_BODY()

	/**
	 * Tag query to determine when these relationships apply.
	 * 用于确定何时应用这些关系的标签查询。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities")
	FGameplayTagQuery ActorTagQuery;

	/**
	 * Array of ability tag relationships.
	 * 技能标签关系数组。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAbilityTagRelationship> AbilityTagRelationships;

#if WITH_EDITORONLY_DATA
	/**
	 * Editor-friendly name for the query.
	 * 查询在编辑器中的友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

/**
 * Data asset for mapping ability tag relationships (block or cancel).
 * 用于映射技能标签关系（阻挡或取消）的数据资产。
 */
UCLASS()
class SIGILGAS_API USigilAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

private:
	/**
	 * List of ability tag relationships.
	 * 技能标签关系列表。
	 */
	UPROPERTY(EditAnywhere, Category = "GGA|Abilities", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAbilityTagRelationship> AbilityTagRelationships;

	/**
	 * Conditional ability tag relationships based on tag queries.
	 * 基于标签查询的条件技能标签关系。
	 */
	UPROPERTY(EditAnywhere, Category = "GCS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilAbilityTagRelationshipsWithQuery> Layered;

public:
	/**
	 * Retrieves tags to block and cancel based on actor and ability tags.
	 * 根据演员和技能标签获取要阻挡和取消的标签。
	 * @param ActorTags Tags on the actor. 演员标签。
	 * @param AbilityTags Tags of the ability. 技能标签。
	 * @param OutTagsToBlock Tags to block (output). 要阻挡的标签（输出）。
	 * @param OutTagsToCancel Tags to cancel (output). 要取消的标签（输出）。
	 */
	void GetAbilityTagsToBlockAndCancelV2(const FGameplayTagContainer& ActorTags, const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock,
	                                      FGameplayTagContainer* OutTagsToCancel) const;

	/**
	 * Retrieves tags to block and cancel based on ability tags.
	 * 根据技能标签获取要阻挡和取消的标签。
	 * @param AbilityTags Tags of the ability. 技能标签。
	 * @param OutTagsToBlock Tags to block (output). 要阻挡的标签（输出）。
	 * @param OutTagsToCancel Tags to cancel (output). 要取消的标签（输出）。
	 */
	void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const;

	/**
	 * Retrieves required and blocked activation tags based on actor and ability tags.
	 * 根据演员和技能标签获取所需和阻止的激活标签。
	 * @param ActorTags Tags on the actor. 演员标签。
	 * @param AbilityTags Tags of the ability. 技能标签。
	 * @param OutActivationRequired Required activation tags (output). 所需激活标签（输出）。
	 * @param OutActivationBlocked Blocked activation tags (output). 阻止激活标签（输出）。
	 */
	void GetRequiredAndBlockedActivationTagsV2(const FGameplayTagContainer& ActorTags, const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired,
	                                           FGameplayTagContainer* OutActivationBlocked) const;

	/**
	 * Retrieves required and blocked activation tags based on ability tags.
	 * 根据技能标签获取所需和阻止的激活标签。
	 * @param AbilityTags Tags of the ability. 技能标签。
	 * @param OutActivationRequired Required activation tags (output). 所需激活标签（输出）。
	 * @param OutActivationBlocked Blocked activation tags (output). 阻止激活标签（输出）。
	 */
	void GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const;

	/**
	 * Checks if an ability is cancelled by a specific action tag.
	 * 检查技能是否被特定动作标签取消。
	 * @param AbilityTags Tags of the ability. 技能标签。
	 * @param ActionTag The action tag to check. 要检查的动作标签。
	 * @return True if the ability is cancelled, false otherwise. 如果技能被取消则返回true，否则返回false。
	 */
	bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;

#if WITH_EDITOR
	/**
	 * Pre-save processing for editor.
	 * 编辑器预保存处理。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};