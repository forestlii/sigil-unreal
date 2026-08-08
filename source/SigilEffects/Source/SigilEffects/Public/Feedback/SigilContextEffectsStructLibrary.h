// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "SigilContextEffectsEnumLibrary.h"
#include "UObject/Object.h"
#include "SigilContextEffectsStructLibrary.generated.h"

class UNiagaraComponent;
class UAudioComponent;
class UAnimSequenceBase;
class UParticleSystemComponent;

/**
 * Definition of a context effect.
 * 情景效果的定义。
 */
USTRUCT(BlueprintType, DisplayName="GES Context Effects Definition")
struct SIGILEFFECTS_API FSigilContextEffects
{
	GENERATED_BODY()

	/**
	 * Tag identifying the effect.
	 * 标识效果的标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GES")
	FGameplayTag EffectTag;

	/**
	 * Query for source tags.
	 * 源标签查询。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GES")
	FGameplayTagQuery SourceTagQuery;

	/**
	 * Query for target tags.
	 * 目标标签查询。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GES")
	FGameplayTagQuery TargetTagQuery;

	/**
	 * Array of effect assets (sounds, Niagara systems, particle systems).
	 * 效果资产数组（音效、Niagara系统、粒子系统）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GES", meta = (AllowedClasses = "/Script/Engine.SoundBase, /Script/Niagara.NiagaraSystem, /Script/Engine.ParticleSystem"))
	TArray<FSoftObjectPath> Effects;

	/**
	 * Deprecated context tags.
	 * 已废弃的情景标签。
	 */
	UPROPERTY(VisibleAnywhere, Category="GES", meta=(DisplayName="Context(Deprecated)"))
	FGameplayTagContainer Context;

#if WITH_EDITORONLY_DATA
	/**
	 * Editor-friendly name for the effect.
	 * 效果的编辑器友好名称。
	 */
	UPROPERTY(EditAnywhere, Category="GES", meta=(EditCondition=false, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

/**
 * Visual effects settings for animation notify.
 * 动画通知的视觉效果设置。
 */
USTRUCT(BlueprintType)
struct SIGILEFFECTS_API FSigilContextEffectAnimNotifyVFXSettings
{
	GENERATED_BODY()

	/**
	 * Scale for spawning visual effects.
	 * 生成视觉效果的缩放。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FX)
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);
};

/**
 * Audio settings for animation notify.
 * 动画通知的音频设置。
 */
USTRUCT(BlueprintType)
struct SIGILEFFECTS_API FSigilContextEffectAnimNotifyAudioSettings
{
	GENERATED_BODY()

	/**
	 * Volume multiplier for audio effects.
	 * 音频效果的音量倍增器。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Sound)
	float VolumeMultiplier = 1.0f;

	/**
	 * Pitch multiplier for audio effects.
	 * 音频效果的音高倍增器。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Sound)
	float PitchMultiplier = 1.0f;
};

/**
 * Trace settings for animation notify.
 * 动画通知的追踪设置。
 */
USTRUCT(BlueprintType)
struct SIGILEFFECTS_API FSigilContextEffectAnimNotifyTraceSettings
{
	GENERATED_BODY()

	/**
	 * Trace channel for surface detection.
	 * 用于表面检测的追踪通道。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Trace)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility;

	/**
	 * Vector offset for the end of the trace.
	 * 追踪结束位置的向量偏移。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Trace)
	FVector EndTraceLocationOffset = FVector::ZeroVector;

	/**
	 * Whether to ignore the actor during tracing.
	 * 追踪时是否忽略演员。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Trace)
	bool bIgnoreActor = true;
};

/**
 * Input structure for spawning context effects.
 * 生成情景效果的输入结构。
 */
USTRUCT(BlueprintType)
struct SIGILEFFECTS_API FSigilSpawnContextEffectsInput
{
	GENERATED_BODY()

public:
	/**
	 * Name of the effect to spawn.
	 * 要生成的效果名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES")
	FGameplayTag EffectName;

	/**
	 * Whether the effect is attached to a component.
	 * 效果是否附加到组件。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES")
	bool bAttached{true};

	/**
	 * Location for spawning if not attached.
	 * 如果未附加，生成位置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES", meta=(EditCondition="!bAttached", EditConditionHides))
	FVector Location{FVector::ZeroVector};

	/**
	 * Rotation for spawning if not attached.
	 * 如果未附加，生成旋转。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES", meta=(EditCondition="!bAttached", EditConditionHides))
	FRotator Rotation{ForceInit};

	/**
	 * Determines how source context is applied (merge or override).
	 * 确定源情景的应用方式（合并或覆盖）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Context")
	ESigilEffectsContextType SourceContextType{ESigilEffectsContextType::Merge};

	/**
	 * Optional source context tags.
	 * 可选的源情景标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Context")
	FGameplayTagContainer SourceContext;

	/**
	 * Optional target context tags.
	 * 可选的目标情景标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Context")
	FGameplayTagContainer TargetContext;

	/**
	 * Bone name for attachment if attached.
	 * 如果附加，附加的骨骼名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Attachment", meta=(EditCondition="bAttached", EditConditionHides))
	FName Bone{NAME_None};

	/**
	 * Component to attach the effect to.
	 * 附加效果的组件。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Attachment", meta=(EditCondition="bAttached", EditConditionHides))
	TObjectPtr<USceneComponent> ComponentToAttach{nullptr};

	/**
	 * Location offset for attached effects.
	 * 附加效果的位置偏移。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Attachment", meta=(EditCondition="bAttached", EditConditionHides))
	FVector LocationOffset{FVector::ZeroVector};

	/**
	 * Rotation offset for attached effects.
	 * 附加效果的旋转偏移。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Attachment", meta=(EditCondition="bAttached", EditConditionHides))
	FRotator RotationOffset{ForceInit};

	/**
	 * Optional animation sequence triggering the effect.
	 * 触发效果的可选动画序列。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES")
	TObjectPtr<const UAnimSequenceBase> AnimationSequence{nullptr};

	/**
	 * Scale for visual effects.
	 * 视觉效果的缩放。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Vfx")
	FVector VFXScale = FVector(1);

	/**
	 * Volume multiplier for audio effects.
	 * 音频效果的音量倍增器。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Sfx")
	float AudioVolume = 1;

	/**
	 * Pitch multiplier for audio effects.
	 * 音频效果的音高倍增器。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Sfx")
	float AudioPitch = 1;

	/**
	 * Whether the effect was triggered by a successful hit.
	 * 效果是否由成功命中触发。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Hit")
	bool bHitSuccess{false};

	/**
	 * Optional hit result for the effect.
	 * 效果的可选命中结果。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES|Hit")
	FHitResult HitResult;
};

/**
 * Output structure for spawned context effects.
 * 生成情景效果的输出结构。
 */
USTRUCT(BlueprintType)
struct SIGILEFFECTS_API FSigilSpawnContextEffectsOutput
{
	GENERATED_BODY()

	/**
	 * Array of spawned audio components.
	 * 生成的音频组件数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES")
	TArray<TObjectPtr<UAudioComponent>> AudioComponents;

	/**
	 * Array of spawned Niagara components.
	 * 生成的Niagara组件数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES")
	TArray<TObjectPtr<UNiagaraComponent>> NiagaraComponents;

	/**
	 * Array of spawned particle system components.
	 * 生成的粒子系统组件数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GES")
	TArray<TObjectPtr<UParticleSystemComponent>> ParticlesComponents;
};