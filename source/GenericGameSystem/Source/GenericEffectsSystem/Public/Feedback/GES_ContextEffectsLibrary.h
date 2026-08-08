// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "GES_ContextEffectsEnumLibrary.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/DataAsset.h"
#include "UObject/WeakObjectPtr.h"
#include "GES_ContextEffectsStructLibrary.h"
#include "GES_ContextEffectsLibrary.generated.h"

class UNiagaraSystem;
class USoundBase;
struct FFrame;

/**
 * Instance of context effects for runtime use.
 * 用于运行时的情景效果实例。
 */
UCLASS(DisplayName="GES Context Effects Instance")
class GENERICEFFECTSSYSTEM_API UGES_ActiveContextEffects : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Tag identifying the effect.
	 * 标识效果的标签。
	 */
	UPROPERTY(VisibleAnywhere, Category="GES")
	FGameplayTag EffectTag;

	/**
	 * Query for source tags.
	 * 源标签查询。
	 */
	UPROPERTY(VisibleAnywhere, Category="GES")
	FGameplayTagQuery SourceTagQuery;

	/**
	 * Query for target tags.
	 * 目标标签查询。
	 */
	UPROPERTY(VisibleAnywhere, Category="GES")
	FGameplayTagQuery TargetTagQuery;

	/**
	 * Array of sound assets for the effect.
	 * 效果的音效资产数组。
	 */
	UPROPERTY(VisibleAnywhere, Category="GES")
	TArray<TObjectPtr<USoundBase>> Sounds;

	/**
	 * Array of Niagara systems for the effect.
	 * 效果的Niagara系统数组。
	 */
	UPROPERTY(VisibleAnywhere, Category="GES")
	TArray<TObjectPtr<UNiagaraSystem>> NiagaraSystems;

	/**
	 * Array of particle systems for the effect.
	 * 效果的粒子系统数组。
	 */
	UPROPERTY(VisibleAnywhere, Category="GES")
	TArray<TObjectPtr<UParticleSystem>> ParticleSystems;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FGES_ContextEffectLibraryLoadingComplete, TArray<UGES_ActiveContextEffects *>, ActiveContextEffects);

/**
 * Data asset containing context effects definitions.
 * 包含情景效果定义的数据资产。
 */
UCLASS(BlueprintType)
class GENERICEFFECTSSYSTEM_API UGES_ContextEffectsLibrary : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Array of context effects definitions.
	 * 情景效果定义数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GES", meta = (TitleProperty="EditorFriendlyName"))
	TArray<FGES_ContextEffects> ContextEffects;

	/**
	 * Retrieves effects for a given tag and context.
	 * 获取指定标签和情景的效果。
	 * @param Effect The effect tag. 效果标签。
	 * @param SourceContext The source context tags. 源情景标签。
	 * @param TargetContext The target context tags. 目标情景标签。
	 * @param Sounds The sound assets (output). 音效资产（输出）。
	 * @param NiagaraSystems The Niagara systems (output). Niagara系统（输出）。
	 * @param ParticleSystems The particle systems (output). 粒子系统（输出）。
	 */
	UFUNCTION(BlueprintCallable, Category="GES|ContextEffect")
	void GetEffects(const FGameplayTag Effect, const FGameplayTagContainer& SourceContext, const FGameplayTagContainer& TargetContext, TArray<USoundBase*>& Sounds,
	                TArray<UNiagaraSystem*>& NiagaraSystems, TArray<UParticleSystem*>& ParticleSystems);

	/**
	 * Loads the effects in the library.
	 * 加载库中的效果。
	 */
	UFUNCTION(BlueprintCallable, Category="GES|ContextEffect")
	void LoadEffects();

	/**
	 * Retrieves the load state of the effects library.
	 * 获取效果库的加载状态。
	 * @return The load state. 加载状态。
	 */
	EGES_ContextEffectsLibraryLoadState GetContextEffectsLibraryLoadState();

private:
	/**
	 * Internal method for loading effects.
	 * 加载效果的内部方法。
	 */
	void LoadEffectsInternal();

	/**
	 * Called when effect library loading is complete.
	 * 效果库加载完成时调用。
	 * @param InActiveContextEffects The loaded context effects. 已加载的情景效果。
	 */
	void OnContextEffectLibraryLoadingComplete(TArray<UGES_ActiveContextEffects*> InActiveContextEffects);

	/**
	 * Array of active context effects.
	 * 活跃情景效果数组。
	 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGES_ActiveContextEffects>> ActiveContextEffects;

	/**
	 * Current load state of the effects library.
	 * 效果库的当前加载状态。
	 */
	UPROPERTY(Transient)
	EGES_ContextEffectsLibraryLoadState EffectsLoadState = EGES_ContextEffectsLibraryLoadState::Unloaded;

#if WITH_EDITOR
	/**
	 * Pre-save processing for editor.
	 * 编辑器预保存处理。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};
