// Copyright 2025 RedMoonGames All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGenericGameSystemModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
