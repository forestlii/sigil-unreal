// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "GES_ContextEffectsStructLibrary.h"
#include "Subsystems/WorldSubsystem.h"
#include "GES_ContextEffectsSubsystem.generated.h"

class UGES_ContextEffectsPreviewSetting;
enum EPhysicalSurface : int;

class AActor;
class UAudioComponent;
class UGES_ContextEffectsLibrary;
class UNiagaraComponent;
class USceneComponent;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;

/**
 * Developer settings for context effects system.
 * 情景效果系统的开发者设置。
 */
UCLASS(config = Game, defaultconfig)
class GENERICEFFECTSSYSTEM_API UGES_ContextEffectsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Mapping of physical surface types to context tags.
	 * 物理表面类型到情景标签的映射。
	 */
	UPROPERTY(config, EditAnywhere, Category="GES")
	TMap<TEnumAsByte<EPhysicalSurface>, FGameplayTag> SurfaceTypeToContextMap;

#if WITH_EDITORONLY_DATA
	/**
	 * Enables preview in the editor.
	 * 在编辑器中启用预览。
	 */
	UPROPERTY(Config, EditAnywhere, Category="PreviewProperties")
	uint32 bPreviewInEditor : 1;

	/**
	 * Preview settings for context effects in the editor.
	 * 编辑器中情景效果的预览设置。
	 */
	UPROPERTY(config, EditAnywhere, Category="PreviewProperties", meta = (EditCondition = "bPreviewInEditor"))
	TSoftObjectPtr<UGES_ContextEffectsPreviewSetting> PreviewSetting;
#endif
};

/**
 * Set of context effects libraries for an actor.
 * 演员的情景效果库集合。
 */
UCLASS()
class GENERICEFFECTSSYSTEM_API UGES_ContextEffectsSet : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Set of context effects libraries.
	 * 情景效果库集合。
	 */
	UPROPERTY(Transient)
	TSet<TObjectPtr<UGES_ContextEffectsLibrary>> ContextEffectsLibraries;
};

/**
 * World subsystem for managing context effects.
 * 管理情景效果的世界子系统。
 */
UCLASS()
class GENERICEFFECTSSYSTEM_API UGES_ContextEffectsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Spawns context effects using a single effects library.
	 * 使用单个效果库生成情景效果。
	 * @param WorldContextObject The world context object. 世界上下文对象。
	 * @param EffectsLibrary The effects library to use. 要使用的效果库。
	 * @param Input The context effects input data. 情景效果输入数据。
	 * @param Output The context effects output data (output). 情景效果输出数据（输出）。
	 */
	UFUNCTION(BlueprintCallable, Category="GES|ContextEffects", meta=(WorldContext = "WorldContextObject"))
	void SpawnContextEffects(UObject* WorldContextObject, TSoftObjectPtr<UGES_ContextEffectsLibrary> EffectsLibrary, FGES_SpawnContextEffectsInput Input, FGES_SpawnContextEffectsOutput& Output);

	/**
	 * Spawns context effects for an actor with extended input.
	 * 为演员生成情景效果，使用扩展输入。
	 * @param SpawningActor The actor spawning the effects. 生成效果的演员。
	 * @param Input The context effects input data. 情景效果输入数据。
	 * @param Output The context effects output data (output). 情景效果输出数据（输出）。
	 */
	UFUNCTION(BlueprintCallable, Category="GES|ContextEffects")
	void SpawnContextEffectsExt(const AActor* SpawningActor, const FGES_SpawnContextEffectsInput& Input, FGES_SpawnContextEffectsOutput& Output);

	/**
	 * Retrieves the context tag for a given physical surface.
	 * 获取指定物理表面的情景标签。
	 * @param PhysicalSurface The physical surface type. 物理表面类型。
	 * @param Context The context tag (output). 情景标签（输出）。
	 * @return True if a context tag was found, false otherwise. 如果找到情景标签则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GES|ContextEffects")
	bool GetContextFromSurfaceType(TEnumAsByte<EPhysicalSurface> PhysicalSurface, FGameplayTag& Context);

	/**
	 * Loads and adds context effects libraries for an actor.
	 * 为演员加载并添加情景效果库。
	 * @param OwningActor The actor owning the libraries. 拥有库的演员。
	 * @param ContextEffectsLibraries The libraries to load. 要加载的库。
	 */
	UFUNCTION(BlueprintCallable, Category="GES|ContextEffects")
	void LoadAndAddContextEffectsLibraries(AActor* OwningActor, TSet<TSoftObjectPtr<UGES_ContextEffectsLibrary>> ContextEffectsLibraries);

	/**
	 * Unloads and removes context effects libraries for an actor.
	 * 为演员卸载并移除情景效果库。
	 * @param OwningActor The actor owning the libraries. 拥有库的演员。
	 */
	UFUNCTION(BlueprintCallable, Category="GES|ContextEffects")
	void UnloadAndRemoveContextEffectsLibraries(AActor* OwningActor);

private:
	/**
	 * Map of actors to their active context effects sets.
	 * 演员到其活跃情景效果集合的映射。
	 */
	UPROPERTY(Transient)
	TMap<TObjectPtr<AActor>, TObjectPtr<UGES_ContextEffectsSet>> ActiveActorEffectsMap;
};