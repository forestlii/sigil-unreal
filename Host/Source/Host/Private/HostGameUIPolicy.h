// Copyright (c) 2026 Likeon. All Rights Reserved.
#pragma once

#include "UI/SigilGameUISubsystem.h"
#include "UI/SigilGameUIPolicy.h"
#include "HostGameUIPolicy.generated.h"

// 仅满足无本地玩家的 Host 自动化初始化合同，不提供游戏 UI 布局。
UCLASS(NotBlueprintable)
class UHostGameUIPolicy final : public USigilGameUIPolicy
{
	GENERATED_BODY()
};
