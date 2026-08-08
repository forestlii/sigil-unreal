// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GCS_AttackResult.h"
#include "UObject/Object.h"
#include "GCS_AttackResultProcessor.generated.h"

/**
 * Base class for processing attack results.
 * 处理攻击结果的基类。
 */
UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, Abstract, Const)
class GENERICCOMBATSYSTEM_API UGCS_AttackResultProcessor : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Processes an incoming attack result.
	 * 处理传入的攻击结果。
	 * @param AttackResult The attack result to process. 要处理的攻击结果。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS")
	virtual void ProcessIncomingAttackResult(const FGCS_AttackResult& AttackResult);

	/**
	 * Gets the world context for the processor.
	 * 获取处理器的世界上下文。
	 * @return The world context. 世界上下文。
	 */
	virtual UWorld* GetWorld() const override;

protected:
	/**
	 * Handles the incoming attack result.
	 * 处理传入的攻击结果。
	 * @param AttackResult The attack result to handle. 要处理的攻击结果。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS")
	void HandleIncomingAttackResult(const FGCS_AttackResult& AttackResult) const;
	virtual void HandleIncomingAttackResult_Implementation(const FGCS_AttackResult& AttackResult) const;

	/**
	 * Gets the owning actor.
	 * 获取所属演员。
	 * @return The owning actor. 所属演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS")
	AActor* GetOwningActor() const;

	/**
	 * Gets the owning ability system component.
	 * 获取所属能力系统组件。
	 * @return The ability system component. 能力系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS")
	UAbilitySystemComponent* GetOwningAbilitySystemComponent() const;

	/**
	 * Gets the editor-friendly name for the processor.
	 * 获取处理器的编辑器友好名称。
	 * @return The editor-friendly name. 编辑器友好名称。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GCS")
	FString GetEditorFriendlyName() const;
	virtual FString GetEditorFriendlyName_Implementation() const;

#if WITH_EDITORONLY_DATA
	/**
	 * Editor-friendly name for the processor.
	 * 处理器的编辑器友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category="GCS")
	FString EditorFriendlyName;

	/**
	 * Called before saving in the editor.
	 * 编辑器中保存前调用。
	 * @param SaveContext The save context. 保存上下文。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};

/**
 * Attack result processor with tag requirements.
 * 具有标签要求的攻击结果处理器。
 */
UCLASS(Abstract)
class GENERICCOMBATSYSTEM_API UGCS_AttackResultProcessor_WithTagRequirement : public UGCS_AttackResultProcessor
{
	GENERATED_BODY()

public:
	/**
	 * Processes an incoming attack result with tag requirements.
	 * 处理具有标签要求的传入攻击结果。
	 * @param AttackResult The attack result to process. 要处理的攻击结果。
	 */
	virtual void ProcessIncomingAttackResult(const FGCS_AttackResult& AttackResult) override;

protected:
	/**
	 * Source tag query for filtering attack results.
	 * 用于过滤攻击结果的来源标签查询。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GCS")
	FGameplayTagQuery SourceTagQuery;

	/**
	 * Target tag query for filtering attack results.
	 * 用于过滤攻击结果的目标标签查询。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GCS")
	FGameplayTagQuery TargetTagQuery;

	/**
	 * Gets the description of the source tag query.
	 * 获取来源标签查询的描述。
	 * @return The source tag query description. 来源标签查询描述。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS")
	FString GetSourceTagQueryDesc() const;

	/**
	 * Gets the description of the target tag query.
	 * 获取目标标签查询的描述。
	 * @return The target tag query description. 目标标签查询描述。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS")
	FString GetTargetTagQueryDesc() const;
};

/**
 * Processor for handling death-related attack results.
 * 处理与死亡相关的攻击结果的处理器。
 */
UCLASS()
class GENERICCOMBATSYSTEM_API UGCS_AttackResultProcessor_Death : public UGCS_AttackResultProcessor
{
	GENERATED_BODY()

public:
	/**
	 * Handles death-related attack results.
	 * 处理与死亡相关的攻击结果。
	 * @param AttackResult The attack result to handle. 要处理的攻击结果。
	 */
	virtual void HandleIncomingAttackResult_Implementation(const FGCS_AttackResult& AttackResult) const override;
};

/**
 * Processor for converting attack results to gameplay events.
 * 将攻击结果转换为游戏事件的处理器。
 * @note Only executes for server pawn or local controller pawn.
 * @注意 仅对服务器Pawn或本地控制器Pawn执行。
 */
UCLASS()
class GENERICCOMBATSYSTEM_API UGCS_AttackResultProcessor_GameplayEvent : public UGCS_AttackResultProcessor_WithTagRequirement
{
	GENERATED_BODY()

protected:
	/**
	 * Handles attack results by converting to gameplay events.
	 * 通过转换为游戏事件处理攻击结果。
	 * @param AttackResult The attack result to handle. 要处理的攻击结果。
	 */
	virtual void HandleIncomingAttackResult_Implementation(const FGCS_AttackResult& AttackResult) const override;

	/**
	 * Gets the editor-friendly name for the processor.
	 * 获取处理器的编辑器友好名称。
	 * @return The editor-friendly name. 编辑器友好名称。
	 */
	virtual FString GetEditorFriendlyName_Implementation() const override;

	/**
	 * Whether to send the event to the attacker.
	 * 是否将事件发送给攻击者。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GCS")
	bool bSendToAttacker{false};

	/**
	 * Gameplay tags to trigger as events.
	 * 作为事件触发的游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GCS")
	TArray<FGameplayTag> EventTriggers;
};

/**
 * Processor for triggering gameplay cues from attack results.
 * 从攻击结果触发游戏提示的处理器。
 * @note Cues do not replicate as attack results are replicated.
 * @注意 提示不复制，因为攻击结果已复制。
 */
UCLASS()
class GENERICCOMBATSYSTEM_API UGCS_AttackResultProcessor_GameplayCue : public UGCS_AttackResultProcessor
{
	GENERATED_BODY()

protected:
	/**
	 * Handles attack results by triggering gameplay cues.
	 * 通过触发游戏提示处理攻击结果。
	 * @param AttackResult The attack result to handle. 要处理的攻击结果。
	 */
	virtual void HandleIncomingAttackResult_Implementation(const FGCS_AttackResult& AttackResult) const override;

	/**
	 * Gets the editor-friendly name for the processor.
	 * 获取处理器的编辑器友好名称。
	 * @return The editor-friendly name. 编辑器友好名称。
	 */
	virtual FString GetEditorFriendlyName_Implementation() const override;

	/**
	 * Modifies gameplay cue parameters before execution.
	 * 在执行前修改游戏提示参数。
	 * @param ParametersToModify The parameters to modify. 要修改的参数。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="GCS")
	void ModifyGameplayCueParametersBeforeExecute(UPARAM(ref) FGameplayCueParameters& ParametersToModify) const;

	/**
	 * Fallback gameplay cues if none are found in the attack result.
	 * 如果攻击结果中未找到提示，则使用备用游戏提示。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GCS", meta=(Categories="GameplayCue", DisplayName="Fallback Gameplay Cues"))
	TArray<FGameplayTag> GameplayCues;

	/**
	 * Tag for finding raw magnitude in TaggedValues.
	 * 在TaggedValues中查找原始幅度的标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GCS")
	FGameplayTag RawMagnitudeTag;

	/**
	 * Tag for finding normalized magnitude in TaggedValues.
	 * 在TaggedValues中查找归一化幅度的标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GCS")
	FGameplayTag NormalizedMagnitudeTag;
};