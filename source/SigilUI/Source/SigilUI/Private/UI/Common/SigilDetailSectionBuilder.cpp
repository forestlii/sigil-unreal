// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Common/SigilDetailSectionsBuilder.h"

TArray<TSoftClassPtr<USigilListEntryDetailSection>> USigilDetailSectionsBuilder::GatherDetailSections_Implementation(const UObject* Data)
{
	TArray<TSoftClassPtr<USigilListEntryDetailSection>> Sections;
	return Sections;
}


TArray<TSoftClassPtr<USigilListEntryDetailSection>> USigilDetailSectionBuilder_Class::GatherDetailSections_Implementation(const UObject* Data)
{
	TArray<TSoftClassPtr<USigilListEntryDetailSection>> Sections;

	// Find extensions for it using the super chain of the setting so that we get any
	// class based extensions for this setting.
	for (UClass* Class = Data->GetClass(); Class; Class = Class->GetSuperClass())
	{
		FSigilEntryDetailsClassSections* ExtensionForClass = SectionsForClasses.Find(Class);
		if (ExtensionForClass)
		{
			Sections.Append(ExtensionForClass->Sections);
		}
	}

	return Sections;
}