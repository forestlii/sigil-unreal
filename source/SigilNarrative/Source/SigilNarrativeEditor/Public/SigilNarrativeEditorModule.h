// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

class SIGILNARRATIVEEDITOR_API FSigilNarrativeEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
