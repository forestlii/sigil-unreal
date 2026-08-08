// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilGasEditor.h"

#include "SigilAttributeGroupNameCustomization.h"

#define LOCTEXT_NAMESPACE "FSigilGasEditorModule"

void FSigilGasEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("SigilAttributeGroupName", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSigilAttributeGroupNameCustomization::MakeInstance));
}

void FSigilGasEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout("SigilAttributeGroupName");
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSigilGasEditorModule, SigilGasEditor)