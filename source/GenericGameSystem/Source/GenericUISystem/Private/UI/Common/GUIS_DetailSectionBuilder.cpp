// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Common/GUIS_DetailSectionsBuilder.h"

TArray<TSoftClassPtr<UGUIS_ListEntryDetailSection>> UGUIS_DetailSectionsBuilder::GatherDetailSections_Implementation(const UObject* Data)
{
	TArray<TSoftClassPtr<UGUIS_ListEntryDetailSection>> Sections;
	return Sections;
}


TArray<TSoftClassPtr<UGUIS_ListEntryDetailSection>> UGUIS_DetailSectionBuilder_Class::GatherDetailSections_Implementation(const UObject* Data)
{
	TArray<TSoftClassPtr<UGUIS_ListEntryDetailSection>> Sections;

	// Find extensions for it using the super chain of the setting so that we get any
	// class based extensions for this setting.
	for (UClass* Class = Data->GetClass(); Class; Class = Class->GetSuperClass())
	{
		FGUIS_EntryDetailsClassSections* ExtensionForClass = SectionsForClasses.Find(Class);
		if (ExtensionForClass)
		{
			Sections.Append(ExtensionForClass->Sections);
		}
	}

	return Sections;
}