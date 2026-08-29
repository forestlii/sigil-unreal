// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/Collections/SigilItemCollection.h"
#include "GameFramework/Actor.h"
#include "Items/SigilItemDefinition.h"
#include "Items/SigilItemInstance.h"
#include "Serialization/SigilSerializationStructLibrary.h"
#include "SigilInventoryFactory.h"
#include "SigilInventorySystemComponent.h"
#include "SigilInventoryTags.h"
#include "Tests/SigilInventoryLoadoutTestTypes.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInventoryDeserializeCollectionMissingItemTest,
	"SigilInventory.Serialization.MissingItem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInventoryServerLoadDefaultLoadoutsTest,
	"SigilInventory.Loadouts.ServerRpc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInventoryDeserializeCollectionMissingItemTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* Inventory = NewObject<USigilInventorySystemComponent>(Owner);
	USigilInventoryFactory* Factory = NewObject<USigilInventoryFactory>(GetTransientPackage());
	USigilItemCollectionDefinition* CollectionDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemCollectionDefinition* DefaultCollectionDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemDefinition* ItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemInstance* ValidItem = NewObject<USigilItemInstance>(Owner);

	if (!TestNotNull(TEXT("Transient owner should exist"), Owner)
		|| !TestNotNull(TEXT("Inventory should exist"), Inventory)
		|| !TestNotNull(TEXT("Inventory factory should exist"), Factory)
		|| !TestNotNull(TEXT("Collection definition should exist"), CollectionDefinition)
		|| !TestNotNull(TEXT("Default collection definition should exist"), DefaultCollectionDefinition)
		|| !TestNotNull(TEXT("Item definition should exist"), ItemDefinition)
		|| !TestNotNull(TEXT("Valid item should exist"), ValidItem))
	{
		return false;
	}

	const FGuid CollectionId = FGuid::NewGuid();
	const FGuid MissingItemId = FGuid::NewGuid();
	const FGuid MissingStackId = FGuid::NewGuid();
	const FGuid ValidItemId = FGuid::NewGuid();
	ValidItem->SetItemId(ValidItemId);
	ValidItem->SetDefinition(ItemDefinition);
	DefaultCollectionDefinition->CollectionTag = SigilCollectionTags::Main;
	TSoftObjectPtr<const USigilItemCollectionDefinition> DefaultCollectionReference(DefaultCollectionDefinition);
	if (!TestNotNull(
		TEXT("Existing default collection should be available for restored items"),
		Inventory->AddCollectionByDefinition(DefaultCollectionReference)))
	{
		return false;
	}

	FSigilCollectionRecord Record;
	Record.Id = CollectionId;
	Record.DefinitionAssetPath = FSoftObjectPath(CollectionDefinition).ToString();
	FSigilStackRecord& MissingStack = Record.StackRecords.AddDefaulted_GetRef();
	MissingStack.Id = MissingStackId;
	MissingStack.CollectionId = CollectionId;
	MissingStack.ItemId = MissingItemId;
	MissingStack.Amount = 1;
	FSigilStackRecord& ValidStack = Record.StackRecords.AddDefaulted_GetRef();
	ValidStack.Id = FGuid::NewGuid();
	ValidStack.CollectionId = CollectionId;
	ValidStack.ItemId = ValidItemId;
	ValidStack.Amount = 2;

	TMap<FGuid, USigilItemInstance*> ItemsMap;
	ItemsMap.Add(ValidItemId, ValidItem);
	AddExpectedError(
		FString::Printf(
			TEXT("USigilInventoryFactory::DeserializeCollection_Implementation: Skipping stack %s because ItemId %s is missing from deserialized inventory."),
			*MissingStackId.ToString(EGuidFormats::DigitsWithHyphens),
			*MissingItemId.ToString(EGuidFormats::DigitsWithHyphens)),
		EAutomationExpectedErrorFlags::Exact,
		1);
	Factory->DeserializeCollection(Inventory, Record, ItemsMap);

	bool bFoundValidItem = false;
	int32 RestoredItemCount = 0;
	for (USigilItemCollection* Collection : Inventory->GetItemCollections())
	{
		const TArray<USigilItemInstance*> RestoredItems = Collection->GetAllItems();
		RestoredItemCount += RestoredItems.Num();
		bFoundValidItem |= RestoredItems.Contains(ValidItem);
	}
	TestEqual(TEXT("Missing item stack should not create an item"), RestoredItemCount, 1);
	TestTrue(TEXT("A valid stack after the missing item should still restore through the inventory"), bFoundValidItem);
	return true;
}

bool FSigilInventoryServerLoadDefaultLoadoutsTest::RunTest(const FString& Parameters)
{
	USigilInventoryLoadoutTestComponent* Inventory =
		NewObject<USigilInventoryLoadoutTestComponent>(GetTransientPackage());
	if (!TestNotNull(TEXT("Inventory should exist"), Inventory))
	{
		return false;
	}

	Inventory->InvokeServerLoadDefaultLoadoutsImplementation();
	TestEqual(TEXT("Server implementation should invoke the local loadout path once"), Inventory->LocalLoadoutCallCount, 1);
	return true;
}

#endif
