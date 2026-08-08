// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilCamera.h"

#include "GameFramework/HUD.h"
#include "SigilCameraSystemComponent.h"

#define LOCTEXT_NAMESPACE "FSigilCameraModule"

void FSigilCameraModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_EDITOR
#if ENABLE_DRAW_DEBUG
	if (!IsRunningDedicatedServer())
	{
		AHUD::OnShowDebugInfo.AddStatic(&USigilCameraSystemComponent::OnShowDebugInfo);
	}
#endif // ENABLE_DRAW_DEBUG
#endif
}

void FSigilCameraModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSigilCameraModule, SigilCamera)
