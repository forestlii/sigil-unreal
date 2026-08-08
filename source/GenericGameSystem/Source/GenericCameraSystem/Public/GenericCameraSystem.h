// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FGenericCameraSystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
