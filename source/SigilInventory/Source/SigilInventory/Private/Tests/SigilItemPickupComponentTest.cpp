// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Items/SigilItemDefinition.h"
#include "Items/SigilItemInstance.h"
#include "Misc/ScopeExit.h"
#include "Pickups/SigilItemPickupComponent.h"
#include "Pickups/SigilWorldItemComponent.h"
#include "SigilInventorySystemComponent.h"
#include "SigilInventoryTags.h"
#include "SigilItemSlotCollection.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace
{
bool SetPickupCollectionTag(USigilItemPickupComponent* Pickup, const FGameplayTag CollectionTag)
{
	FStructProperty* Property = FindFProperty<FStructProperty>(Pickup->GetClass(), TEXT("CollectionTag"));
	if (!Property)
	{
		return false;
	}

	*Property->ContainerPtrToValuePtr<FGameplayTag>(Pickup) = CollectionTag;
	return true;
}

bool SetPickupWorldItem(USigilItemPickupComponent* Pickup, USigilWorldItemComponent* WorldItem)
{
	FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(Pickup->GetClass(), TEXT("WorldItemComponent"));
	if (!Property)
	{
		return false;
	}

	Property->SetObjectPropertyValue_InContainer(Pickup, WorldItem);
	return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilItemPickupMissingWorldItemTest,
	"SigilInventory.Pickup.MissingWorldItemReturnsFalse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilItemPickupFailedAddTest,
	"SigilInventory.Pickup.FailedAddDoesNotSucceed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilItemPickupMissingWorldItemTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	USigilInventorySystemComponent* Picker = NewObject<USigilInventorySystemComponent>(Owner);
	USigilItemPickupComponent* Pickup = NewObject<USigilItemPickupComponent>(Owner);

	if (!TestNotNull(TEXT("Transient owner should exist"), Owner)
		|| !TestNotNull(TEXT("Picker should exist"), Picker)
		|| !TestNotNull(TEXT("Pickup should exist"), Pickup))
	{
		return false;
	}

	if (!TestTrue(TEXT("Transient owner should have authority"), Owner->HasAuthority())
		|| !TestTrue(TEXT("CollectionTag should be set"), SetPickupCollectionTag(Pickup, SigilCollectionTags::Main)))
	{
		return false;
	}

	Pickup->Activate(true);
	Pickup->OnPickupSuccess.AddDynamic(Pickup, &UActorComponent::Deactivate);

	TestFalse(TEXT("Pickup without a WorldItem should fail"), Pickup->Pickup(Picker));
	TestTrue(TEXT("Failed pickup should not broadcast success"), Pickup->IsActive());
	return true;
}

bool FSigilItemPickupFailedAddTest::RunTest(const FString& Parameters)
{
	if (!TestNotNull(TEXT("GEngine should exist"), GEngine))
	{
		return false;
	}

	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	if (!TestNotNull(TEXT("Test game instance should exist"), GameInstance))
	{
		return false;
	}

	GameInstance->AddToRoot();
	GameInstance->InitializeStandalone(
		MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("SigilInventoryPickupTestWorld")),
		GetTransientPackage());
	UWorld* World = GameInstance->GetWorld();
	if (World)
	{
		World->AddToRoot();
	}
	ON_SCOPE_EXIT
	{
		GameInstance->Shutdown();
		if (World)
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
			World->RemoveFromRoot();
		}
		GameInstance->RemoveFromRoot();
	};

	if (!TestNotNull(TEXT("Test world should exist"), World))
	{
		return false;
	}
	World->InitializeActorsForPlay(FURL());

	AActor* PickerActor = World->SpawnActor<AActor>();
	AActor* PickupActor = World->SpawnActor<AActor>();
	USigilInventorySystemComponent* Picker = NewObject<USigilInventorySystemComponent>(PickerActor);
	if (!TestNotNull(TEXT("Picker actor should exist"), PickerActor)
		|| !TestNotNull(TEXT("Pickup actor should exist"), PickupActor)
		|| !TestNotNull(TEXT("Picker should exist"), Picker))
	{
		return false;
	}

	USigilItemSlotCollectionDefinition* CollectionDefinition =
		NewObject<USigilItemSlotCollectionDefinition>(GetTransientPackage());
	CollectionDefinition->CollectionTag = SigilCollectionTags::Main;
	FSigilItemSlotDefinition& SlotDefinition = CollectionDefinition->SlotDefinitions.AddDefaulted_GetRef();
	SlotDefinition.Tag = SigilCollectionTags::Equipped;
	SlotDefinition.TagQuery = FGameplayTagQuery::MakeQuery_MatchTag(SigilCollectionTags::Hidden);
	TSoftObjectPtr<const USigilItemCollectionDefinition> SoftCollectionDefinition(CollectionDefinition);
	if (!TestNotNull(
		TEXT("Slot collection should be added"),
		Picker->AddCollectionByDefinition(SoftCollectionDefinition)))
	{
		return false;
	}

	USigilItemDefinition* ItemDefinition = NewObject<USigilItemDefinition>(GetTransientPackage());
	USigilItemInstance* SourceItem = NewObject<USigilItemInstance>(PickupActor);
	SourceItem->SetItemId(FGuid::NewGuid());
	SourceItem->SetDefinition(ItemDefinition);

	FSigilItemInfo Candidate(SourceItem, 1, SigilCollectionTags::Main);
	FSigilItemInfo AddableCandidate;
	TestTrue(TEXT("Fixture CanAddItem should accept the item"), Picker->CanAddItem(Candidate, AddableCandidate));
	TestEqual(TEXT("Fixture AddItem should add zero items"), Picker->AddItem(Candidate).Amount, 0);

	USigilWorldItemComponent* WorldItem = NewObject<USigilWorldItemComponent>(PickupActor);
	USigilItemPickupComponent* Pickup = NewObject<USigilItemPickupComponent>(PickupActor);
	WorldItem->SetItemInfo(SourceItem, 1);
	if (!TestTrue(TEXT("CollectionTag should be set"), SetPickupCollectionTag(Pickup, SigilCollectionTags::Main))
		|| !TestTrue(TEXT("WorldItem should be set"), SetPickupWorldItem(Pickup, WorldItem)))
	{
		return false;
	}

	Pickup->Activate(true);
	Pickup->OnPickupSuccess.AddDynamic(Pickup, &UActorComponent::Deactivate);

	TestFalse(TEXT("Pickup should fail when AddItem adds zero items"), Pickup->Pickup(Picker));
	TestTrue(TEXT("Failed AddItem should not broadcast pickup success"), Pickup->IsActive());
	return true;
}

#endif
