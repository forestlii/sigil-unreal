// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimMontage.h"
#include "CombatFlow/SigilAbilityActionSetSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Equipping/SigilWeaponEquipmentInstance.h"
#include "Fragments/SigilItemFragment_Equippable.h"
#include "Fragments/SigilItemFragment_WeaponLoadout.h"
#include "Items/SigilItemDefinition.h"
#include "Items/SigilItemInstance.h"
#include "SigilArsenalFunctionLibrary.h"
#include "SigilInventoryTags.h"
#include "SigilItemSlotCollection.h"
#include "Tests/SigilArsenalTestTypes.h"
#include "UObject/Package.h"
#include "Utilities/SigilAbilitySystemFunctionLibrary.h"
#include "Weapon/SigilWeaponInterface.h"

namespace
{
struct FSigilArsenalTestWorld
{
	FWorldContext* WorldContext = nullptr;
	UWorld* World = nullptr;

	explicit FSigilArsenalTestWorld(const TCHAR* BaseName)
	{
		if (!GEngine)
		{
			return;
		}
		const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), BaseName, EUniqueObjectNameOptions::GloballyUnique);
		WorldContext = &GEngine->CreateNewWorldContext(EWorldType::Game);
		World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
		if (World)
		{
			World->AddToRoot();
			WorldContext->SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}
	}

	~FSigilArsenalTestWorld()
	{
		if (!World || !GEngine)
		{
			return;
		}
		GEngine->ShutdownWorldNetDriver(World);
		World->DestroyWorld(true);
		World->SetPhysicsScene(nullptr);
		GEngine->DestroyWorldContext(World);
		World->RemoveFromRoot();
	}
};

/** Slot collection: two weapon slots in one exclusive group, both accepting the gun item tag. */
USigilItemSlotCollectionDefinition* MakeWeaponSlotCollection()
{
	USigilItemSlotCollectionDefinition* Definition = NewObject<USigilItemSlotCollectionDefinition>(GetTransientPackage());
	Definition->CollectionTag = SigilCollectionTags::Equipped;

	const FGameplayTag Slots[] = {SigilArsenalTestTags::SlotPrimary, SigilArsenalTestTags::SlotSecondary};
	FSigilItemSlotGroup Group;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Slots); ++Index)
	{
		FSigilItemSlotDefinition& Slot = Definition->SlotDefinitions.AddDefaulted_GetRef();
		Slot.Tag = Slots[Index];
		Slot.TagQuery = FGameplayTagQuery::MakeQuery_MatchTag(SigilArsenalTestTags::ItemGun);

		// The editor builds these maps in PreSave; a runtime-made definition has to fill them itself.
		Definition->IndexToTagMap.Add(Index, Slots[Index]);
		Definition->TagToIndexMap.Add(Slots[Index], Index);
		Group.IndexToSlotMap.Add(Index, Slots[Index]);
		Group.SlotToIndexMap.Add(Slots[Index], Index);
	}
	Definition->SlotGroups.Add(SigilArsenalTestTags::SlotWeaponGroup);
	Definition->SlotGroupMap.Add(SigilArsenalTestTags::SlotWeaponGroup, Group);
	return Definition;
}

USigilAbilityActionSetSettings* MakeFireActionSet(const TCHAR* MontageName, UAnimMontage*& OutMontage)
{
	OutMontage = NewObject<UAnimMontage>(GetTransientPackage(), MontageName);
	USigilAbilityActionSetSettings* ActionSet = NewObject<USigilAbilityActionSetSettings>(GetTransientPackage());
	FSigilAbilityActionSet& Set = ActionSet->ActionSets.AddDefaulted_GetRef();
	Set.AbilityTag = SigilArsenalTestTags::AbilityFire;
	FSigilAbilityAction& Action = Set.Actions.AddDefaulted_GetRef();
	Action.Animation = OutMontage;
	return ActionSet;
}

/** Gun item: equippable (weapon equipment instance, auto-activate, spawns a weapon actor) + weapon loadout. */
USigilItemDefinition* MakeGunDefinition(const TCHAR* Name, USigilAbilitySet* AbilitySet, USigilAbilityActionSetSettings* ActionSet, ESigilWeaponAbilityGrantPolicy Policy)
{
	USigilItemDefinition* Definition = NewObject<USigilItemDefinition>(GetTransientPackage(), Name);
	Definition->ItemTags.AddTag(SigilArsenalTestTags::ItemGun);

	USigilItemFragment_Equippable* Equippable = NewObject<USigilItemFragment_Equippable>(Definition);
	Equippable->InstanceType = USigilWeaponEquipmentInstance::StaticClass();
	Equippable->bActorBased = false;
	Equippable->bAutoActivate = true;
	FSigilEquipmentActorToSpawn& Spawn = Equippable->ActorsToSpawn.AddDefaulted_GetRef();
	Spawn.ActorToSpawn = ASigilArsenalTestWeaponActor::StaticClass();
	Spawn.bShouldAttach = false;
	Definition->Fragments.Add(Equippable);

	USigilItemFragment_WeaponLoadout* Loadout = NewObject<USigilItemFragment_WeaponLoadout>(Definition);
	Loadout->AbilitySet = AbilitySet;
	Loadout->AbilityActionSet = ActionSet;
	Loadout->GrantPolicy = Policy;
	Definition->Fragments.Add(Loadout);

	return Definition;
}

USigilItemInstance* MakeItem(UObject* Outer, const USigilItemDefinition* Definition)
{
	USigilItemInstance* Item = NewObject<USigilItemInstance>(Outer);
	Item->SetItemId(FGuid::NewGuid());
	Item->SetDefinition(Definition);
	return Item;
}

USigilWeaponEquipmentInstance* WeaponInSlot(const ASigilArsenalTestPawn* Pawn, const FGameplayTag& Slot)
{
	return Cast<USigilWeaponEquipmentInstance>(Pawn->GetEquipment()->GetEquipmentInSlot(Slot));
}

USigilArsenalTestFireAbility* FireAbilityInstance(USigilAbilitySystemComponent* ASC, const UObject* Source)
{
	const FGameplayAbilitySpecHandle Handle = USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilArsenalTestFireAbility::StaticClass(), Source);
	FGameplayAbilitySpec* Spec = Handle.IsValid() ? ASC->FindAbilitySpecFromHandle(Handle) : nullptr;
	return Spec ? Cast<USigilArsenalTestFireAbility>(Spec->GetPrimaryInstance()) : nullptr;
}

ASigilArsenalTestPawn* SpawnReadyPawn(UWorld* World, USigilItemSlotCollectionDefinition* Collection)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASigilArsenalTestPawn* Pawn = World->SpawnActor<ASigilArsenalTestPawn>(ASigilArsenalTestPawn::StaticClass(), FTransform::Identity, Params);
	if (!Pawn)
	{
		return nullptr;
	}

	Pawn->GetSigilAbilitySystem()->InitAbilityActorInfo(Pawn, Pawn);
	Pawn->GetInventory()->AddCollectionDefinitionForTest(Collection);
	Pawn->GetInventory()->InitializeInventorySystem();
	Pawn->GetEquipment()->SetTargetCollectionTagForTest(SigilCollectionTags::Equipped);
	Pawn->GetEquipment()->InitializeEquipmentSystem();
	return Pawn;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalWeaponLoadoutTest,
	"SigilArsenal.WeaponLoadout.GrantsGatesAndSwitchesWeapons",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalWeaponLoadoutTest::RunTest(const FString& Parameters)
{
	FSigilArsenalTestWorld Fixture(TEXT("SigilArsenalLoadoutWorld"));
	if (!Fixture.World)
	{
		AddError(TEXT("The test world could not be created"));
		return false;
	}

	USigilItemSlotCollectionDefinition* Collection = MakeWeaponSlotCollection();
	ASigilArsenalTestPawn* Pawn = SpawnReadyPawn(Fixture.World, Collection);
	TestNotNull(TEXT("The pawn should spawn"), Pawn);
	if (!Pawn)
	{
		return false;
	}
	TestTrue(TEXT("The inventory initializes"), Pawn->GetInventory()->IsInventoryInitialized());
	TestTrue(TEXT("The equipment system initializes"), Pawn->GetEquipment()->IsEquipmentSystemInitialized());

	USigilAbilitySystemComponent* ASC = Pawn->GetSigilAbilitySystem();
	USigilAbilitySet* AbilitySet = NewObject<USigilArsenalTestAbilitySet>(GetTransientPackage());
	UAnimMontage* PistolMontage = nullptr;
	UAnimMontage* RifleMontage = nullptr;
	USigilItemDefinition* PistolDefinition = MakeGunDefinition(TEXT("SigilArsenalTestPistol"), AbilitySet, MakeFireActionSet(TEXT("SigilArsenalTestPistolFire"), PistolMontage), ESigilWeaponAbilityGrantPolicy::WhileEquipped);
	USigilItemDefinition* RifleDefinition = MakeGunDefinition(TEXT("SigilArsenalTestRifle"), AbilitySet, MakeFireActionSet(TEXT("SigilArsenalTestRifleFire"), RifleMontage), ESigilWeaponAbilityGrantPolicy::WhileEquipped);

	// Nothing equipped: no weapon, no actions, no fire ability.
	TestNull(TEXT("No active weapon before equipping"), USigilArsenalFunctionLibrary::GetActiveWeaponEquipment(Pawn, FGameplayTagQuery()));
	TArray<FSigilAbilityAction> Actions;
	TestFalse(TEXT("No actions before equipping"), USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions(Pawn, FGameplayTagContainer(SigilArsenalTestTags::AbilityFire), FGameplayTagContainer(), FGameplayTagContainer(), Actions, FGameplayTagQuery()));
	TestFalse(TEXT("No fire ability before equipping"), USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilArsenalTestFireAbility::StaticClass()).IsValid());

	// --- Pick up the pistol: it lands in the primary slot, auto-activates and grants its ability set.
	USigilItemInstance* Pistol = MakeItem(Pawn, PistolDefinition);
	TestEqual(TEXT("The pistol is added to the equipped collection"), Pawn->GetInventory()->AddItem(FSigilItemInfo(Pistol, 1, SigilCollectionTags::Equipped)).Amount, 1);

	USigilWeaponEquipmentInstance* PistolWeapon = WeaponInSlot(Pawn, SigilArsenalTestTags::SlotPrimary);
	TestNotNull(TEXT("The pistol becomes a weapon equipment instance in the primary slot"), PistolWeapon);
	if (!PistolWeapon)
	{
		return false;
	}
	TestNotNull(TEXT("The loadout fragment is resolved"), PistolWeapon->GetLoadout());
	TestTrue(TEXT("The pistol is active (auto-activate, group free)"), ISigilEquipmentInterface::Execute_IsEquipmentActive(PistolWeapon));
	TestTrue(TEXT("The pistol reports itself as an active ability source"), ISigilAbilitySourceInterface::Execute_IsAbilitySourceActive(PistolWeapon));
	TestTrue(TEXT("The pistol's ability set is granted"), PistolWeapon->AreAbilitiesGranted());
	TestEqual(TEXT("The library resolves the pistol as the active weapon"), USigilArsenalFunctionLibrary::GetActiveWeaponEquipment(Pawn, FGameplayTagQuery()), PistolWeapon);

	// Weapon actor spawned and pointed back at the equipment instance.
	AActor* PistolActor = PistolWeapon->GetWeaponActor();
	TestNotNull(TEXT("The pistol spawned a weapon actor"), PistolActor);
	if (PistolActor)
	{
		TestEqual(TEXT("The weapon actor's SourceObject is the equipment instance"), ISigilWeaponInterface::Execute_GetSourceObject(PistolActor), static_cast<UObject*>(PistolWeapon));
		TestEqual(TEXT("The library resolves the equipment from the weapon actor"), USigilArsenalFunctionLibrary::GetWeaponEquipmentOfWeaponActor(PistolActor), PistolWeapon);
		TestEqual(TEXT("The library resolves the active weapon actor"), USigilArsenalFunctionLibrary::GetActiveWeaponActor(Pawn, FGameplayTagQuery()), PistolActor);
	}

	// Ability granted with the equipment instance as SourceObject; activation allowed while active.
	USigilArsenalTestFireAbility* PistolFire = FireAbilityInstance(ASC, PistolWeapon);
	TestNotNull(TEXT("The fire ability is granted with the pistol equipment as SourceObject"), PistolFire);
	if (!PistolFire)
	{
		return false;
	}
	TestEqual(TEXT("The library resolves the weapon equipment from the ability"), USigilArsenalFunctionLibrary::GetWeaponEquipmentOfAbility(PistolFire), PistolWeapon);
	TestTrue(TEXT("The pistol's fire ability activates while the pistol is active"), ASC->TryActivateAbility(PistolFire->GetCurrentAbilitySpecHandle()));
	TestEqual(TEXT("Pistol fire counted one activation"), PistolFire->ActivationCount, 1);

	// Montage lookup goes through the pistol's action set.
	TestTrue(TEXT("The active weapon answers the fire action query"), USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions(Pawn, FGameplayTagContainer(SigilArsenalTestTags::AbilityFire), FGameplayTagContainer(), FGameplayTagContainer(), Actions, FGameplayTagQuery()));
	TestEqual(TEXT("One action for the pistol"), Actions.Num(), 1);
	TestTrue(TEXT("The pistol's fire montage is selected"), Actions.Num() == 1 && Actions[0].Animation == PistolMontage);

	// --- Pick up the rifle: secondary slot, not active (group occupied), abilities granted but gated.
	USigilItemInstance* Rifle = MakeItem(Pawn, RifleDefinition);
	TestEqual(TEXT("The rifle is added to the equipped collection"), Pawn->GetInventory()->AddItem(FSigilItemInfo(Rifle, 1, SigilCollectionTags::Equipped)).Amount, 1);
	USigilWeaponEquipmentInstance* RifleWeapon = WeaponInSlot(Pawn, SigilArsenalTestTags::SlotSecondary);
	TestNotNull(TEXT("The rifle becomes a weapon equipment instance in the secondary slot"), RifleWeapon);
	if (!RifleWeapon)
	{
		return false;
	}
	TestFalse(TEXT("The rifle is not active while the pistol holds the group"), ISigilEquipmentInterface::Execute_IsEquipmentActive(RifleWeapon));
	TestTrue(TEXT("WhileEquipped grants the rifle's abilities even while inactive"), RifleWeapon->AreAbilitiesGranted());
	USigilArsenalTestFireAbility* RifleFire = FireAbilityInstance(ASC, RifleWeapon);
	TestNotNull(TEXT("The rifle's fire ability is a separate spec with the rifle as SourceObject"), RifleFire);
	if (!RifleFire)
	{
		return false;
	}
	TestFalse(TEXT("The rifle's fire ability is blocked while the rifle is inactive"), ASC->TryActivateAbility(RifleFire->GetCurrentAbilitySpecHandle()));
	TestEqual(TEXT("Rifle fire did not activate"), RifleFire->ActivationCount, 0);

	// --- Switch weapons through the slot group (index 0 = primary, 1 = secondary): the group deactivates the old entry
	// and activates the new one. (SetEquipmentActiveState(slot, false) on an active entry is a no-op in sigil.inventory.)
	Pawn->GetEquipment()->SetGroupActiveIndex(SigilArsenalTestTags::SlotWeaponGroup, 1);
	TestFalse(TEXT("The pistol is inactive after the switch"), ISigilEquipmentInterface::Execute_IsEquipmentActive(PistolWeapon));
	TestTrue(TEXT("The rifle is active after the switch"), ISigilEquipmentInterface::Execute_IsEquipmentActive(RifleWeapon));
	TestEqual(TEXT("The library now resolves the rifle"), USigilArsenalFunctionLibrary::GetActiveWeaponEquipment(Pawn, FGameplayTagQuery()), RifleWeapon);
	TestFalse(TEXT("The pistol's fire ability is blocked once the pistol is inactive"), ASC->TryActivateAbility(PistolFire->GetCurrentAbilitySpecHandle()));
	TestEqual(TEXT("Pistol fire count unchanged"), PistolFire->ActivationCount, 1);
	TestTrue(TEXT("The rifle's fire ability activates once the rifle is active"), ASC->TryActivateAbility(RifleFire->GetCurrentAbilitySpecHandle()));
	TestEqual(TEXT("Rifle fire counted one activation"), RifleFire->ActivationCount, 1);
	TestTrue(TEXT("The action query now answers with the rifle"), USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions(Pawn, FGameplayTagContainer(SigilArsenalTestTags::AbilityFire), FGameplayTagContainer(), FGameplayTagContainer(), Actions, FGameplayTagQuery()));
	TestTrue(TEXT("The rifle's fire montage is selected"), Actions.Num() == 1 && Actions[0].Animation == RifleMontage);
	TestTrue(TEXT("The pistol still answers its own query directly"), PistolWeapon->QueryAbilityActions(FGameplayTagContainer(SigilArsenalTestTags::AbilityFire), FGameplayTagContainer(), FGameplayTagContainer(), Actions) && Actions[0].Animation == PistolMontage);

	// --- Unequip the rifle: its abilities are revoked, the pistol's remain.
	Pawn->GetEquipment()->UnequipBySlot(SigilArsenalTestTags::SlotSecondary);
	TestFalse(TEXT("The rifle's fire ability is gone after unequipping"), USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilArsenalTestFireAbility::StaticClass(), RifleWeapon).IsValid());
	TestTrue(TEXT("The pistol's fire ability survives the rifle's removal"), USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilArsenalTestFireAbility::StaticClass(), PistolWeapon).IsValid());
	TestFalse(TEXT("The rifle equipment reports its abilities as revoked"), RifleWeapon->AreAbilitiesGranted());
	TestNull(TEXT("No active weapon after the rifle is gone (pistol stayed inactive)"), USigilArsenalFunctionLibrary::GetActiveWeaponEquipment(Pawn, FGameplayTagQuery()));

	Pawn->GetEquipment()->UnequipBySlot(SigilArsenalTestTags::SlotPrimary);
	TestFalse(TEXT("No fire ability at all once both weapons are unequipped"), USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilArsenalTestFireAbility::StaticClass()).IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalWhileActivePolicyTest,
	"SigilArsenal.WeaponLoadout.WhileActivePolicyGrantsOnlyWhenActive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalWhileActivePolicyTest::RunTest(const FString& Parameters)
{
	FSigilArsenalTestWorld Fixture(TEXT("SigilArsenalWhileActiveWorld"));
	if (!Fixture.World)
	{
		AddError(TEXT("The test world could not be created"));
		return false;
	}

	ASigilArsenalTestPawn* Pawn = SpawnReadyPawn(Fixture.World, MakeWeaponSlotCollection());
	TestNotNull(TEXT("The pawn should spawn"), Pawn);
	if (!Pawn)
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Pawn->GetSigilAbilitySystem();
	USigilAbilitySet* AbilitySet = NewObject<USigilArsenalTestAbilitySet>(GetTransientPackage());
	UAnimMontage* Montage = nullptr;
	USigilItemDefinition* PistolDefinition = MakeGunDefinition(TEXT("SigilArsenalTestPistolWA"), AbilitySet, MakeFireActionSet(TEXT("SigilArsenalTestPistolFireWA"), Montage), ESigilWeaponAbilityGrantPolicy::WhileActive);
	USigilItemDefinition* RifleDefinition = MakeGunDefinition(TEXT("SigilArsenalTestRifleWA"), AbilitySet, MakeFireActionSet(TEXT("SigilArsenalTestRifleFireWA"), Montage), ESigilWeaponAbilityGrantPolicy::WhileActive);

	Pawn->GetInventory()->AddItem(FSigilItemInfo(MakeItem(Pawn, PistolDefinition), 1, SigilCollectionTags::Equipped));
	Pawn->GetInventory()->AddItem(FSigilItemInfo(MakeItem(Pawn, RifleDefinition), 1, SigilCollectionTags::Equipped));
	USigilWeaponEquipmentInstance* PistolWeapon = WeaponInSlot(Pawn, SigilArsenalTestTags::SlotPrimary);
	USigilWeaponEquipmentInstance* RifleWeapon = WeaponInSlot(Pawn, SigilArsenalTestTags::SlotSecondary);
	TestNotNull(TEXT("The pistol is equipped"), PistolWeapon);
	TestNotNull(TEXT("The rifle is equipped"), RifleWeapon);
	if (!PistolWeapon || !RifleWeapon)
	{
		return false;
	}

	// Active pistol granted, inactive rifle not granted.
	TestTrue(TEXT("The active pistol's abilities are granted"), PistolWeapon->AreAbilitiesGranted());
	TestNotNull(TEXT("The pistol's fire ability exists"), FireAbilityInstance(ASC, PistolWeapon));
	TestFalse(TEXT("The inactive rifle's abilities are not granted under WhileActive"), RifleWeapon->AreAbilitiesGranted());
	TestNull(TEXT("The rifle has no fire ability while inactive"), FireAbilityInstance(ASC, RifleWeapon));

	// Switching the group index moves the grant.
	Pawn->GetEquipment()->SetGroupActiveIndex(SigilArsenalTestTags::SlotWeaponGroup, 1);
	TestFalse(TEXT("Deactivating the pistol revokes its abilities"), PistolWeapon->AreAbilitiesGranted());
	TestNull(TEXT("The pistol's fire ability is gone"), FireAbilityInstance(ASC, PistolWeapon));
	TestTrue(TEXT("Activating the rifle grants its abilities"), RifleWeapon->AreAbilitiesGranted());
	USigilArsenalTestFireAbility* RifleFire = FireAbilityInstance(ASC, RifleWeapon);
	TestNotNull(TEXT("The rifle's fire ability exists once active"), RifleFire);
	if (RifleFire)
	{
		TestTrue(TEXT("The rifle's fire ability activates"), ASC->TryActivateAbility(RifleFire->GetCurrentAbilitySpecHandle()));
		TestEqual(TEXT("Rifle fire counted one activation"), RifleFire->ActivationCount, 1);
	}

	// Unequipping an active weapon revokes through both the deactivation and the end-play path without double release.
	Pawn->GetEquipment()->UnequipBySlot(SigilArsenalTestTags::SlotSecondary);
	TestFalse(TEXT("Unequipping the active rifle revokes its abilities"), RifleWeapon->AreAbilitiesGranted());
	TestFalse(TEXT("No fire ability remains"), USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilArsenalTestFireAbility::StaticClass()).IsValid());

	return true;
}

#endif
