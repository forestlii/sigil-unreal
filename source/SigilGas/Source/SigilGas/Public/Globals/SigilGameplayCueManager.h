// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "SigilGameplayCueManager.generated.h"

/**
 * GameplayCueManager that loads GameplayCue notifies on first use instead of async-loading every cue (and everything
 * they reference) when a map starts. Idea from GASShooter UGSGameplayCueManager (Copyright 2020 Dan Kestranek, MIT),
 * made configurable. Enable it with:
 *   [/Script/GameplayAbilities.AbilitySystemGlobals]
 *   GlobalGameplayCueManagerClass=/Script/SigilGas.SigilGameplayCueManager
 * and optionally
 *   [/Script/SigilGas.SigilGameplayCueManager]
 *   bAsyncLoadRuntimeObjectLibraries=True
 * to restore the engine default.
 * 只在首次请求时加载 GameplayCue，而不是在地图开始时异步加载全部 Cue 及其引用资产。
 * 思路来自 GASShooter UGSGameplayCueManager（Copyright 2020 Dan Kestranek，MIT），改为可配置；配置方式见上方英文注释。
 */
UCLASS(Config = Game)
class SIGILGAS_API USigilGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	/**
	 * When false (default) runtime object libraries are not async-loaded up front; cues load on first request.
	 * 为 false（默认）时不在启动时异步加载运行时对象库，Cue 在首次请求时加载。
	 */
	UPROPERTY(Config)
	bool bAsyncLoadRuntimeObjectLibraries = false;

	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override { return bAsyncLoadRuntimeObjectLibraries; }
};
