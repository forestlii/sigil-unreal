// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FSigilCameraModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
