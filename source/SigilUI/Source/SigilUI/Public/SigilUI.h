// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Module interface for the Generic UI System.
 * 通用UI系统的模块接口。
 */
class FSigilUIModule : public IModuleInterface
{
public:
    /**
     * Called when the module is loaded.
     * 模块加载时调用。
     */
    virtual void StartupModule() override;

    /**
     * Called when the module is unloaded.
     * 模块卸载时调用。
     */
    virtual void ShutdownModule() override;
};