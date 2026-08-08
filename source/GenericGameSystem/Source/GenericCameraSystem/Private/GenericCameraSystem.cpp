// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GenericCameraSystem.h"

#include "GameFramework/HUD.h"
#include "GCMS_CameraSystemComponent.h"

#define LOCTEXT_NAMESPACE "FGenericCameraSystemModule"

void FGenericCameraSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_EDITOR
#if ENABLE_DRAW_DEBUG
	if (!IsRunningDedicatedServer())
	{
		AHUD::OnShowDebugInfo.AddStatic(&UGCMS_CameraSystemComponent::OnShowDebugInfo);
	}
#endif // ENABLE_DRAW_DEBUG
#endif
}

void FGenericCameraSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGenericCameraSystemModule, GenericCameraSystem)
