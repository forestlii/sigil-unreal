#include "GenericGameplayAbilitiesEditor.h"

#include "GGA_AttributeGroupNameCustomization.h"

#define LOCTEXT_NAMESPACE "FGenericGameplayAbilitiesEditorModule"

void FGenericGameplayAbilitiesEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("GGA_AttributeGroupName", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGGA_AttributeGroupNameCustomization::MakeInstance));
}

void FGenericGameplayAbilitiesEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout("GGA_AttributeGroupName");
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGenericGameplayAbilitiesEditorModule, GenericGameplayAbilitiesEditor)