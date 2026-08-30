// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/Collections/SigilItemCollection.h"
#include "Core/Collections/SigilItemMultiStackCollection.h"
#include "Core/Collections/SigilItemRestriction_StackSizeLimit.h"
#include "GameFramework/Actor.h"
#include "Items/SigilItemDefinition.h"
#include "Items/SigilItemInstance.h"
#include "SigilInventorySystemComponent.h"
#include "SigilInventoryTags.h"
#include "Tests/SigilInventoryTask3TestTypes.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace
{
USigilItemCollection* AddDefaultCollection(
	USigilInventorySystemComponent* Inventory,
	USigilItemCollectionDefinition* Definition)
{
	Definition->CollectionTag = SigilCollectionTags::Main;
	return Inventory->AddCollectionByDefinition(
		TSoftObjectPtr<const USigilItemCollectionDefinition>(Definition));
}

USigilItemInstance* CreateItem(AActor* Owner, const USigilItemDefinition* Definition)
{
	USigilItemInstance* Item = NewObject<USigilItemInstance>(Owner);
	Item->SetItemId(FGuid::NewGuid());
	Item->SetDefinition(Definition);
	return Item;
}

bool SetDefaultStackSizeLimit(USigilItemRestriction_StackSizeLimit* Restriction, const int32 Limit)
{
	FIntProperty* Property = FindFProperty<FIntProperty>(
		Restriction->GetClass(),
		TEXT("DefaultStackSizeLimit"));
	if (!Property)
	{
		return false;
	}

	Property->SetPropertyValue_InContainer(Restriction, Limit);
	return true;
}

int32 GetDroppedAmount(const TArray<FSigilItemInfo>& ItemInfos)
{
	int32 Total = 0;
	for (const FSigilItemInfo& ItemInfo : ItemInfos)
	{
		Total += ItemInfo.Amount;
	}
	return Total;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInventoryPickupPartialTransferTest,
	"SigilInventory.Pickup.PartialTransfer.KeepsSourceRemainder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInventoryPickupZeroTransferTest,
	"SigilInventory.Pickup.PartialTransfer.ZeroActualTransferFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInventoryPickupMultiStackConservationTest,
	"SigilInventory.Pickup.PartialTransfer.MultiStackShortRemovalConservesQuantity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInventoryPickupTargetRemoveRestrictionTest,
	"SigilInventory.Pickup.PartialTransfer.TargetRemoveRestrictionDoesNotInflateTransfer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInventoryPickupUnrelatedMutationTest,
	"SigilInventory.Pickup.PartialTransfer.UnrelatedSynchronousMutationDoesNotAffectTransfer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilCraftingRemoveIngredientsSuccessTest,
	"SigilInventory.Crafting.RemoveIngredients.AllRemovedReturnsTrue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilCraftingRemoveIngredientsFailureTest,
	"SigilInventory.Crafting.RemoveIngredients.InsufficientReturnsFalse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilRandomDropEmptyCollectionTest,
	"SigilInventory.Drop.Random.EmptyCollectionIsSafe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilRandomDropAmountBoundsTest,
	"SigilInventory.Drop.Random.AmountStaysWithinBounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilRandomDropCumulativeBoundaryTest,
	"SigilInventory.Drop.Random.CumulativeWeightBoundaryStaysInRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInventoryPickupPartialTransferTest::RunTest(const FString& Parameters)
{
	AActor* SourceOwner = NewObject<AActor>(GetTransientPackage());
	AActor* DestinationOwner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* SourceInventory = NewObject<USigilInventorySystemComponent>(SourceOwner);
	USigilInventorySystemComponent* DestinationInventory = NewObject<USigilInventorySystemComponent>(DestinationOwner);
	USigilItemCollectionDefinition* SourceDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemCollectionDefinition* DestinationDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemRestriction_StackSizeLimit* StackLimit = NewObject<USigilItemRestriction_StackSizeLimit>(DestinationDefinition);
	USigilItemDefinition* ItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());

	if (!TestTrue(TEXT("Destination stack limit should be configured"), SetDefaultStackSizeLimit(StackLimit, 5)))
	{
		return false;
	}
	DestinationDefinition->Restrictions.Add(StackLimit);

	USigilItemCollection* SourceCollection = AddDefaultCollection(SourceInventory, SourceDefinition);
	USigilItemCollection* DestinationCollection = AddDefaultCollection(DestinationInventory, DestinationDefinition);
	USigilItemInstance* SourceItem = CreateItem(SourceOwner, ItemDefinition);
	USigilItemInstance* DestinationItem = CreateItem(DestinationOwner, ItemDefinition);
	USigilInventoryPickupTask3TestComponent* Pickup =
		NewObject<USigilInventoryPickupTask3TestComponent>(SourceOwner);

	if (!TestNotNull(TEXT("Source collection should exist"), SourceCollection)
		|| !TestNotNull(TEXT("Destination collection should exist"), DestinationCollection)
		|| !TestNotNull(TEXT("Source item should exist"), SourceItem)
		|| !TestNotNull(TEXT("Destination item should exist"), DestinationItem)
		|| !TestNotNull(TEXT("Pickup should exist"), Pickup))
	{
		return false;
	}

	TestEqual(TEXT("Source fixture should contain five items"), SourceCollection->AddItem(SourceItem, 5).Amount, 5);
	TestEqual(TEXT("Destination fixture should contain three items"), DestinationCollection->AddItem(DestinationItem, 3).Amount, 3);
	Pickup->SetInventoryForTest(SourceInventory);
	Pickup->Activate(true);
	Pickup->OnPickupSuccess.AddDynamic(Pickup, &UActorComponent::Deactivate);

	TestTrue(TEXT("A partial transfer should succeed"), Pickup->AddPickupToCollectionForTest(DestinationCollection));
	TestEqual(TEXT("Destination should receive only its remaining capacity"), DestinationCollection->GetItemAmount(DestinationItem), 5);
	TestEqual(TEXT("Source should retain the exact untransferred remainder"), SourceCollection->GetItemAmount(SourceItem), 3);
	TestFalse(TEXT("A successful partial transfer should broadcast success"), Pickup->IsActive());
	return true;
}

bool FSigilInventoryPickupZeroTransferTest::RunTest(const FString& Parameters)
{
	AActor* SourceOwner = NewObject<AActor>(GetTransientPackage());
	AActor* DestinationOwner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* SourceInventory = NewObject<USigilInventorySystemComponent>(SourceOwner);
	USigilInventorySystemComponent* DestinationInventory = NewObject<USigilInventorySystemComponent>(DestinationOwner);
	USigilItemCollectionDefinition* SourceDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemCollectionDefinition* DestinationDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemRestriction_StackSizeLimit* StackLimit = NewObject<USigilItemRestriction_StackSizeLimit>(DestinationDefinition);
	USigilItemDefinition* ItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());

	if (!TestTrue(TEXT("Destination stack limit should be configured"), SetDefaultStackSizeLimit(StackLimit, 2)))
	{
		return false;
	}
	DestinationDefinition->Restrictions.Add(StackLimit);

	USigilItemCollection* SourceCollection = AddDefaultCollection(SourceInventory, SourceDefinition);
	USigilItemCollection* DestinationCollection = AddDefaultCollection(DestinationInventory, DestinationDefinition);
	USigilItemInstance* SourceItem = CreateItem(SourceOwner, ItemDefinition);
	USigilItemInstance* DestinationItem = CreateItem(DestinationOwner, ItemDefinition);
	USigilInventoryPickupTask3TestComponent* Pickup =
		NewObject<USigilInventoryPickupTask3TestComponent>(SourceOwner);

	if (!TestNotNull(TEXT("Source collection should exist"), SourceCollection)
		|| !TestNotNull(TEXT("Destination collection should exist"), DestinationCollection)
		|| !TestNotNull(TEXT("Pickup should exist"), Pickup))
	{
		return false;
	}

	TestEqual(TEXT("Source fixture should contain one item"), SourceCollection->AddItem(SourceItem, 1).Amount, 1);
	TestEqual(TEXT("Destination fixture should already be full"), DestinationCollection->AddItem(DestinationItem, 2).Amount, 2);
	Pickup->SetInventoryForTest(SourceInventory);
	Pickup->Activate(true);
	Pickup->OnPickupSuccess.AddDynamic(Pickup, &UActorComponent::Deactivate);

	TestFalse(TEXT("A zero-item transfer should fail"), Pickup->AddPickupToCollectionForTest(DestinationCollection));
	TestEqual(TEXT("A failed transfer should leave the source unchanged"), SourceCollection->GetItemAmount(SourceItem), 1);
	TestEqual(TEXT("A failed transfer should leave the destination unchanged"), DestinationCollection->GetItemAmount(DestinationItem), 2);
	TestTrue(TEXT("A zero-item transfer should not broadcast success"), Pickup->IsActive());
	return true;
}

bool FSigilInventoryPickupMultiStackConservationTest::RunTest(const FString& Parameters)
{
	AActor* SourceOwner = NewObject<AActor>(GetTransientPackage());
	AActor* DestinationOwner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* SourceInventory = NewObject<USigilInventorySystemComponent>(SourceOwner);
	USigilInventorySystemComponent* DestinationInventory = NewObject<USigilInventorySystemComponent>(DestinationOwner);
	USigilItemCollectionDefinition* SourceDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemMultiStackCollectionDefinition* DestinationDefinition =
		NewObject<USigilItemMultiStackCollectionDefinition>(GetTransientPackage());
	USigilInventoryRemoveAmountLimitTestRestriction* RemoveLimit =
		NewObject<USigilInventoryRemoveAmountLimitTestRestriction>(SourceDefinition);
	USigilItemDefinition* ItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());

	RemoveLimit->SetMaxRemoveAmountForTest(20);
	SourceDefinition->Restrictions.Add(RemoveLimit);
	DestinationDefinition->DefaultStackSizeLimit = 10;

	USigilItemCollection* SourceCollection = AddDefaultCollection(SourceInventory, SourceDefinition);
	USigilItemCollection* DestinationCollection = AddDefaultCollection(DestinationInventory, DestinationDefinition);
	USigilItemInstance* SourceItem = CreateItem(SourceOwner, ItemDefinition);
	USigilItemInstance* DestinationItem = CreateItem(DestinationOwner, ItemDefinition);
	USigilInventoryPickupTask3TestComponent* Pickup =
		NewObject<USigilInventoryPickupTask3TestComponent>(SourceOwner);

	if (!TestNotNull(TEXT("Source collection should exist"), SourceCollection)
		|| !TestNotNull(TEXT("Multi-stack destination collection should exist"), DestinationCollection)
		|| !TestNotNull(TEXT("Source item should exist"), SourceItem)
		|| !TestNotNull(TEXT("Destination item should exist"), DestinationItem)
		|| !TestNotNull(TEXT("Pickup should exist"), Pickup))
	{
		return false;
	}

	TestEqual(TEXT("Source fixture should contain twenty-five items"), SourceCollection->AddItem(SourceItem, 25).Amount, 25);
	TestEqual(TEXT("Destination fixture should start with one compatible item"), DestinationCollection->AddItem(DestinationItem, 1).Amount, 1);
	Pickup->SetInventoryForTest(SourceInventory);
	Pickup->Activate(true);
	Pickup->OnPickupSuccess.AddDynamic(Pickup, &UActorComponent::Deactivate);

	TestTrue(TEXT("A transfer limited by source removal should still move the conserved amount"), Pickup->AddPickupToCollectionForTest(DestinationCollection));
	const int32 DestinationAmount = DestinationCollection->GetItemAmount(SourceItem);
	const int32 SourceAmount = SourceCollection->GetItemAmount(SourceItem);
	TestEqual(TEXT("Destination should retain its fixture plus the twenty items actually removed from the source"), DestinationAmount, 21);
	TestEqual(TEXT("Source should retain the five items its removal restriction rejected"), SourceAmount, 5);
	TestEqual(TEXT("Pickup should preserve the original total quantity"), DestinationAmount + SourceAmount, 26);
	TestFalse(TEXT("A conserved transfer should broadcast success"), Pickup->IsActive());
	return true;
}

bool FSigilInventoryPickupTargetRemoveRestrictionTest::RunTest(const FString& Parameters)
{
	AActor* SourceOwner = NewObject<AActor>(GetTransientPackage());
	AActor* DestinationOwner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* SourceInventory = NewObject<USigilInventorySystemComponent>(SourceOwner);
	USigilInventorySystemComponent* DestinationInventory = NewObject<USigilInventorySystemComponent>(DestinationOwner);
	USigilItemCollectionDefinition* SourceDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemCollectionDefinition* DestinationDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilInventoryRemoveAmountLimitTestRestriction* SourceRemoveLimit =
		NewObject<USigilInventoryRemoveAmountLimitTestRestriction>(SourceDefinition);
	USigilInventoryRemoveAmountLimitTestRestriction* DestinationRemoveLimit =
		NewObject<USigilInventoryRemoveAmountLimitTestRestriction>(DestinationDefinition);
	USigilItemDefinition* ItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());

	SourceRemoveLimit->SetMaxRemoveAmountForTest(20);
	DestinationRemoveLimit->SetMaxRemoveAmountForTest(0);
	SourceDefinition->Restrictions.Add(SourceRemoveLimit);
	DestinationDefinition->Restrictions.Add(DestinationRemoveLimit);

	USigilItemCollection* SourceCollection = AddDefaultCollection(SourceInventory, SourceDefinition);
	USigilItemCollection* DestinationCollection = AddDefaultCollection(DestinationInventory, DestinationDefinition);
	USigilItemInstance* SourceItem = CreateItem(SourceOwner, ItemDefinition);
	USigilItemInstance* DestinationItem = CreateItem(DestinationOwner, ItemDefinition);
	USigilInventoryPickupTask3TestComponent* Pickup =
		NewObject<USigilInventoryPickupTask3TestComponent>(SourceOwner);

	if (!TestNotNull(TEXT("Source collection should exist"), SourceCollection)
		|| !TestNotNull(TEXT("Destination collection should exist"), DestinationCollection)
		|| !TestNotNull(TEXT("Pickup should exist"), Pickup))
	{
		return false;
	}

	TestEqual(TEXT("Source fixture should contain twenty-five items"), SourceCollection->AddItem(SourceItem, 25).Amount, 25);
	TestEqual(TEXT("Destination fixture should start with one compatible item"), DestinationCollection->AddItem(DestinationItem, 1).Amount, 1);
	Pickup->SetInventoryForTest(SourceInventory);
	Pickup->Activate(true);
	Pickup->OnPickupSuccess.AddDynamic(Pickup, &UActorComponent::Deactivate);

	TestTrue(TEXT("A source-limited transfer should succeed without target rollback"), Pickup->AddPickupToCollectionForTest(DestinationCollection));
	const int32 DestinationAmount = DestinationCollection->GetItemAmount(SourceItem);
	const int32 SourceAmount = SourceCollection->GetItemAmount(SourceItem);
	TestEqual(TEXT("Destination should receive only the twenty items allowed from the source"), DestinationAmount, 21);
	TestEqual(TEXT("Source should retain the five items rejected by its removal restriction"), SourceAmount, 5);
	TestEqual(TEXT("A target remove restriction should not inflate the logical item total"), DestinationAmount + SourceAmount, 26);
	TestFalse(TEXT("A conserved source-first transfer should broadcast success"), Pickup->IsActive());
	return true;
}

bool FSigilInventoryPickupUnrelatedMutationTest::RunTest(const FString& Parameters)
{
	AActor* SourceOwner = NewObject<AActor>(GetTransientPackage());
	AActor* DestinationOwner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* SourceInventory = NewObject<USigilInventorySystemComponent>(SourceOwner);
	USigilInventorySystemComponent* DestinationInventory = NewObject<USigilInventorySystemComponent>(DestinationOwner);
	USigilItemCollectionDefinition* SourceDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilInventoryPickupSynchronousMutationTestCollectionDefinition* DestinationDefinition =
		NewObject<USigilInventoryPickupSynchronousMutationTestCollectionDefinition>(GetTransientPackage());
	USigilItemRestriction_StackSizeLimit* StackLimit =
		NewObject<USigilItemRestriction_StackSizeLimit>(DestinationDefinition);
	USigilItemDefinition* PickedItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemDefinition* UnrelatedItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());

	if (!TestTrue(TEXT("Destination stack limit should be configured"), SetDefaultStackSizeLimit(StackLimit, 5)))
	{
		return false;
	}
	DestinationDefinition->Restrictions.Add(StackLimit);

	USigilItemCollection* SourceCollection = AddDefaultCollection(SourceInventory, SourceDefinition);
	USigilInventoryPickupSynchronousMutationTestCollection* DestinationCollection =
		Cast<USigilInventoryPickupSynchronousMutationTestCollection>(
			AddDefaultCollection(DestinationInventory, DestinationDefinition));
	USigilItemInstance* SourceItem = CreateItem(SourceOwner, PickedItemDefinition);
	USigilItemInstance* DestinationItem = CreateItem(DestinationOwner, PickedItemDefinition);
	USigilItemInstance* UnrelatedItem = CreateItem(DestinationOwner, UnrelatedItemDefinition);
	USigilInventoryPickupTask3TestComponent* Pickup =
		NewObject<USigilInventoryPickupTask3TestComponent>(SourceOwner);

	if (!TestNotNull(TEXT("Source collection should exist"), SourceCollection)
		|| !TestNotNull(TEXT("Synchronous-mutation destination should exist"), DestinationCollection)
		|| !TestNotNull(TEXT("Pickup should exist"), Pickup))
	{
		return false;
	}

	TestEqual(TEXT("Source fixture should contain five picked items"), SourceCollection->AddItem(SourceItem, 5).Amount, 5);
	TestEqual(
		TEXT("Destination fixture should contain three compatible items"),
		DestinationCollection->AddItem(FSigilItemInfo(DestinationItem, 3)).Amount,
		3);
	DestinationCollection->ConfigureSynchronousAddForTest(UnrelatedItem, 3);
	Pickup->SetInventoryForTest(SourceInventory);
	Pickup->Activate(true);
	Pickup->OnPickupSuccess.AddDynamic(Pickup, &UActorComponent::Deactivate);

	TestTrue(TEXT("A partial transfer should succeed despite unrelated synchronous mutation"), Pickup->AddPickupToCollectionForTest(DestinationCollection));
	const int32 DestinationPickedAmount = DestinationCollection->GetItemAmount(SourceItem);
	const int32 SourcePickedAmount = SourceCollection->GetItemAmount(SourceItem);
	TestEqual(TEXT("Destination should receive only its two-item capacity for the picked item"), DestinationPickedAmount, 5);
	TestEqual(TEXT("Unrelated mutation should not increase the picked amount removed from the source"), SourcePickedAmount, 3);
	TestEqual(TEXT("The picked logical item total should remain conserved"), DestinationPickedAmount + SourcePickedAmount, 8);
	TestEqual(TEXT("The synchronous unrelated item mutation should still occur"), DestinationCollection->GetItemAmount(UnrelatedItem), 3);
	TestFalse(TEXT("A conserved transfer with unrelated mutation should broadcast success"), Pickup->IsActive());
	return true;
}

bool FSigilCraftingRemoveIngredientsSuccessTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* Inventory = NewObject<USigilInventorySystemComponent>(Owner);
	USigilItemCollectionDefinition* CollectionDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemCollection* Collection = AddDefaultCollection(Inventory, CollectionDefinition);
	USigilItemDefinition* FirstDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemDefinition* SecondDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemInstance* FirstItem = CreateItem(Owner, FirstDefinition);
	USigilItemInstance* SecondItem = CreateItem(Owner, SecondDefinition);
	USigilCraftingTask3TestComponent* Crafting = NewObject<USigilCraftingTask3TestComponent>(Owner);

	if (!TestNotNull(TEXT("Collection should exist"), Collection)
		|| !TestNotNull(TEXT("Crafting component should exist"), Crafting))
	{
		return false;
	}

	TestEqual(TEXT("First ingredient fixture should contain two items"), Collection->AddItem(FirstItem, 2).Amount, 2);
	TestEqual(TEXT("Second ingredient fixture should contain three items"), Collection->AddItem(SecondItem, 3).Amount, 3);
	const TArray<FSigilItemInfo> Ingredients{
		FSigilItemInfo(FirstItem, 2),
		FSigilItemInfo(SecondItem, 3)
	};

	TestTrue(TEXT("Removing every requested ingredient should return true"), Crafting->RemoveItemIngredientsForTest(Inventory, Ingredients));
	TestEqual(TEXT("First ingredient should be fully removed"), Collection->GetItemAmount(FirstItem), 0);
	TestEqual(TEXT("Second ingredient should be fully removed"), Collection->GetItemAmount(SecondItem), 0);
	return true;
}

bool FSigilCraftingRemoveIngredientsFailureTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* Inventory = NewObject<USigilInventorySystemComponent>(Owner);
	USigilItemCollectionDefinition* CollectionDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemCollection* Collection = AddDefaultCollection(Inventory, CollectionDefinition);
	USigilItemDefinition* InsufficientDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemDefinition* LaterDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemInstance* InsufficientItem = CreateItem(Owner, InsufficientDefinition);
	USigilItemInstance* LaterItem = CreateItem(Owner, LaterDefinition);
	USigilCraftingTask3TestComponent* Crafting = NewObject<USigilCraftingTask3TestComponent>(Owner);

	if (!TestNotNull(TEXT("Collection should exist"), Collection)
		|| !TestNotNull(TEXT("Crafting component should exist"), Crafting))
	{
		return false;
	}

	TestEqual(TEXT("Insufficient fixture should contain one item"), Collection->AddItem(InsufficientItem, 1).Amount, 1);
	TestEqual(TEXT("Later fixture should contain two items"), Collection->AddItem(LaterItem, 2).Amount, 2);
	const TArray<FSigilItemInfo> Ingredients{
		FSigilItemInfo(InsufficientItem, 2),
		FSigilItemInfo(LaterItem, 2)
	};

	TestFalse(TEXT("An insufficient ingredient should return false"), Crafting->RemoveItemIngredientsForTest(Inventory, Ingredients));
	TestEqual(TEXT("The available portion of the failed ingredient remains non-transactional"), Collection->GetItemAmount(InsufficientItem), 0);
	TestEqual(TEXT("Ingredients after the first failure should not be processed"), Collection->GetItemAmount(LaterItem), 2);
	return true;
}

bool FSigilRandomDropEmptyCollectionTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* Inventory = NewObject<USigilInventorySystemComponent>(Owner);
	Owner->AddInstanceComponent(Inventory);
	USigilItemCollectionDefinition* CollectionDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemCollection* Collection = AddDefaultCollection(Inventory, CollectionDefinition);
	USigilRandomItemDropperTask3TestComponent* Dropper = NewObject<USigilRandomItemDropperTask3TestComponent>(Owner);
	Dropper->SetDropConfigForTest(SigilCollectionTags::Main, 1, 1);

	if (!TestNotNull(TEXT("Empty collection should exist"), Collection)
		|| !TestNotNull(TEXT("Dropper should exist"), Dropper))
	{
		return false;
	}

	TestTrue(TEXT("An empty collection should safely produce no drops"), Dropper->GetItemsToDropForTest().IsEmpty());
	return true;
}

bool FSigilRandomDropAmountBoundsTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* Inventory = NewObject<USigilInventorySystemComponent>(Owner);
	Owner->AddInstanceComponent(Inventory);
	USigilItemCollectionDefinition* CollectionDefinition = NewObject<USigilItemCollectionDefinition>(GetTransientPackage());
	USigilItemCollection* Collection = AddDefaultCollection(Inventory, CollectionDefinition);
	USigilItemDefinition* ItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemInstance* Item = CreateItem(Owner, ItemDefinition);
	USigilRandomItemDropperTask3TestComponent* Dropper = NewObject<USigilRandomItemDropperTask3TestComponent>(Owner);
	Dropper->SetDropConfigForTest(SigilCollectionTags::Main, 2, 3);

	if (!TestNotNull(TEXT("Collection should exist"), Collection)
		|| !TestNotNull(TEXT("Dropper should exist"), Dropper))
	{
		return false;
	}
	TestEqual(TEXT("Weighted item fixture should contain ten items"), Collection->AddItem(Item, 10).Amount, 10);

	FMath::RandInit(6678);
	const int32 FirstDroppedAmount = GetDroppedAmount(Dropper->GetItemsToDropForTest());
	const int32 SecondDroppedAmount = GetDroppedAmount(Dropper->GetItemsToDropForTest());
	TestTrue(TEXT("First call should drop at least MinAmount"), FirstDroppedAmount >= 2);
	TestTrue(TEXT("First call should drop at most MaxAmount"), FirstDroppedAmount <= 3);
	TestEqual(TEXT("The fixed seed should exercise the inclusive approved maximum"), FirstDroppedAmount, 3);
	TestTrue(TEXT("Second call should independently drop at least MinAmount"), SecondDroppedAmount >= 2);
	TestTrue(TEXT("Second call should independently drop at most MaxAmount"), SecondDroppedAmount <= 3);

	Collection->RemoveAll();
	TestTrue(
		TEXT("A later call after clearing the same source collection should return empty"),
		Dropper->GetItemsToDropForTest().IsEmpty());
	return true;
}

bool FSigilRandomDropCumulativeBoundaryTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	USigilItemDefinition* FirstDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemDefinition* SecondDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemInstance* FirstItem = CreateItem(Owner, FirstDefinition);
	USigilItemInstance* SecondItem = CreateItem(Owner, SecondDefinition);
	USigilRandomItemDropperTask3TestComponent* Dropper = NewObject<USigilRandomItemDropperTask3TestComponent>(Owner);
	const TArray<FSigilItemInfo> CumulativeWeights{
		FSigilItemInfo(FirstItem, 1),
		FSigilItemInfo(SecondItem, 2)
	};

	FMath::RandInit(6678);
	const FSigilItemInfo& Selection = Dropper->GetRandomItemInfoForTest(CumulativeWeights, 2);
	TestEqual(TEXT("The final cumulative-weight boundary should select the second item"), Selection.Item.Get(), SecondItem);
	return true;
}

#endif
