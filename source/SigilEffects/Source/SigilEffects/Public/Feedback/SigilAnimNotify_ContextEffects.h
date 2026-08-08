// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GameplayTagContainer.h"
#include "SigilContextEffectsStructLibrary.h"
#include "Engine/EngineTypes.h"
#include "SigilAnimNotify_ContextEffects.generated.h"

class USigilAnimNotify_ContextEffects;

/**
 * Base class for customizing spawn behavior of context effects.
 * 自定义情景效果生成行为的基类。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, Const)
class USigilContextEffectsSpawnParametersProvider : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Provides spawn parameters for context effects.
	 * 为情景效果提供生成参数。
	 * @param InMeshComp The skeletal mesh component. 骨骼网格组件。
	 * @param InNotifyNotify The context effects notify. 情景效果通知。
	 * @param InAnimation The animation sequence. 动画序列。
	 * @param OutSpawnLocation The spawn location (output). 生成位置（输出）。
	 * @param OutSpawnRotation The spawn rotation (output). 生成旋转（输出）。
	 * @return True if parameters were provided, false otherwise. 如果提供了参数则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GES|AnimNotify")
	bool ProvideParameters(USkeletalMeshComponent* InMeshComp, const USigilAnimNotify_ContextEffects* InNotifyNotify, UAnimSequenceBase* InAnimation,
	                       FVector& OutSpawnLocation, FRotator& OutSpawnRotation) const;
};

/**
 * Animation notify for playing context effects.
 * 用于播放情景效果的动画通知。
 */
UCLASS(const, hidecategories = Object, CollapseCategories, Config = Game, meta = (DisplayName = "Play Context Effects"))
class SIGILEFFECTS_API USigilAnimNotify_ContextEffects : public UAnimNotify
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the context effects animation notify.
	 * 情景效果动画通知构造函数。
	 */
	USigilAnimNotify_ContextEffects(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Called after the object is loaded.
	 * 对象加载后调用。
	 */
	virtual void PostLoad() override;

#if WITH_EDITOR
	/**
	 * Handles property changes in the editor.
	 * 处理编辑器中的属性更改。
	 * @param PropertyChangedEvent The property change event. 属性更改事件。
	 */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/**
	 * Retrieves the display name for the notify.
	 * 获取通知的显示名称。
	 * @return The notify name. 通知名称。
	 */
	virtual FString GetNotifyName_Implementation() const override;

	/**
	 * Called when the notify is triggered during animation.
	 * 动画期间通知触发时调用。
	 * @param MeshComp The skeletal mesh component. 骨骼网格组件。
	 * @param Animation The animation sequence. 动画序列。
	 * @param EventReference The animation notify event reference. 动画通知事件引用。
	 */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

#if WITH_EDITOR
	/**
	 * Validates associated assets in the editor.
	 * 在编辑器中验证相关资产。
	 */
	virtual void ValidateAssociatedAssets() override;
#endif

#if WITH_EDITOR
	/**
	 * Sets parameters for the context effects notify in the editor.
	 * 在编辑器中设置情景效果通知的参数。
	 * @param EffectIn The effect tag. 效果标签。
	 * @param LocationOffsetIn The location offset. 位置偏移。
	 * @param RotationOffsetIn The rotation offset. 旋转偏移。
	 * @param VFXPropertiesIn The VFX settings. VFX设置。
	 * @param AudioPropertiesIn The audio settings. 音频设置。
	 * @param bAttachedIn Whether to attach to mesh. 是否附加到网格。
	 * @param SocketNameIn The socket name for attachment. 附加的插槽名称。
	 * @param bPerformTraceIn Whether to perform a trace. 是否执行追踪。
	 * @param TracePropertiesIn The trace settings. 追踪设置。
	 */
	UFUNCTION(BlueprintCallable, Category="GES|AnimNotify")
	void SetParameters(FGameplayTag EffectIn, FVector LocationOffsetIn, FRotator RotationOffsetIn,
	                   FSigilContextEffectAnimNotifyVFXSettings VFXPropertiesIn, FSigilContextEffectAnimNotifyAudioSettings AudioPropertiesIn,
	                   bool bAttachedIn, FName SocketNameIn, bool bPerformTraceIn, FSigilContextEffectAnimNotifyTraceSettings TracePropertiesIn);
#endif

	/**
	 * The effect to play.
	 * 要播放的效果。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AnimNotify", meta = (DisplayName = "Effect", ExposeOnSpawn = true))
	FGameplayTag Effect;

	/**
	 * Location offset for effect spawning (socket if attached, mesh if not).
	 * 效果生成的位置偏移（附加时为插槽，否则为网格）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AnimNotify", meta = (ExposeOnSpawn = true))
	FVector LocationOffset = FVector::ZeroVector;

	/**
	 * Rotation offset for effect spawning.
	 * 效果生成的旋转偏移。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AnimNotify", meta = (ExposeOnSpawn = true))
	FRotator RotationOffset = FRotator::ZeroRotator;

	/**
	 * Visual effects settings for the notify.
	 * 通知的视觉效果设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AnimNotify", meta = (ExposeOnSpawn = true))
	FSigilContextEffectAnimNotifyVFXSettings VFXProperties;

	/**
	 * Audio settings for the notify.
	 * 通知的音频设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AnimNotify", meta = (ExposeOnSpawn = true))
	FSigilContextEffectAnimNotifyAudioSettings AudioProperties;

	/**
	 * Whether to attach the effect to the mesh component.
	 * 是否将效果附加到网格组件。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttachmentProperties", meta = (DisplayName="Attach To Mesh", ExposeOnSpawn = true))
	uint32 bAttached : 1;

	/**
	 * Optional provider for custom spawn location/rotation if not attached.
	 * 如果未附加，可选的自定义生成位置/旋转提供者。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="AttachmentProperties", meta=(EditCondition="!bAttached"))
	TObjectPtr<USigilContextEffectsSpawnParametersProvider> SpawnParametersProvider;

	/**
	 * Socket name to attach the effect to.
	 * 附加效果的插槽名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttachmentProperties", meta=(EditCondition="bAttached"))
	FName SocketName{NAME_None};

	/**
	 * Whether to perform a trace for surface type conversion.
	 * 是否执行追踪以进行表面类型转换。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AnimNotify", meta = (ExposeOnSpawn = true))
	uint32 bPerformTrace : 1;

	/**
	 * Trace settings for the notify.
	 * 通知的追踪设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AnimNotify", meta = (ExposeOnSpawn = true, EditCondition = "bPerformTrace"))
	FSigilContextEffectAnimNotifyTraceSettings TraceProperties;

#if WITH_EDITORONLY_DATA
	/**
	 * Performs a preview of the context effects in the editor.
	 * 在编辑器中执行情景效果预览。
	 * @param InOwningActor The owning actor. 拥有演员。
	 * @param InSourceContext The source context tags. 源情景标签。
	 * @param InTargetContext The target context tags. 目标情景标签。
	 * @param InMeshComp The skeletal mesh component. 骨骼网格组件。
	 */
	void PerformEditorPreview(AActor* InOwningActor, FGameplayTagContainer& InSourceContext, FGameplayTagContainer& InTargetContext, USkeletalMeshComponent* InMeshComp);
#endif
};