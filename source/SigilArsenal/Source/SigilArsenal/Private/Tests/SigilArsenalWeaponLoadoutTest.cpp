// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Abilities/SigilAbilityCost_ItemIntegerAttribute.h"
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
#include "SigilArsenalTags.h"
#include "SigilInventoryFactory.h"
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

/** 弹药测试共享真实库存创建、装备授予和按来源查找技能的路径。 */
struct FSigilArsenalAmmoWeapon
{
	USigilItemInstance* Item = nullptr;
	USigilWeaponEquipmentInstance* Equipment = nullptr;
	USigilArsenalTestAmmoFireAbility* Ability = nullptr;

	bool IsReady() const { return Item && Equipment && Ability && Ability->GetAmmoCostForTest(); }
};

FSigilArsenalAmmoWeapon AddAmmoWeapon(ASigilArsenalTestPawn* Pawn, const TCHAR* Name, const FGameplayTag& Slot, bool bHasMagazine = true)
{
	FSigilArsenalAmmoWeapon Result;
	USigilItemDefinition* Definition = MakeGunDefinition(Name, NewObject<USigilArsenalTestAmmoAbilitySet>(GetTransientPackage()), nullptr, ESigilWeaponAbilityGrantPolicy::WhileEquipped);
	if (bHasMagazine)
	{
		USigilArsenalTestAmmoAttributes* Attributes = NewObject<USigilArsenalTestAmmoAttributes>(Definition);
		Attributes->SetMagazineForTest(2);
		Definition->Fragments.Add(Attributes);
	}

	// 工厂会调用片段的 OnInstanceCreated；仅 SetDefinition 不会初始化弹匣。
	USigilInventoryFactory* Factory = NewObject<USigilInventoryFactory>(GetTransientPackage());
	Result.Item = Factory->CreateItem(Pawn, Definition);
	if (!Result.Item || Pawn->GetInventory()->AddItem(FSigilItemInfo(Result.Item, 1, SigilCollectionTags::Equipped)).Amount != 1)
	{
		return Result;
	}
	Result.Equipment = WeaponInSlot(Pawn, Slot);
	USigilAbilitySystemComponent* ASC = Pawn->GetSigilAbilitySystem();
	const FGameplayAbilitySpecHandle Handle = USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilArsenalTestAmmoFireAbility::StaticClass(), Result.Equipment);
	FGameplayAbilitySpec* Spec = Handle.IsValid() ? ASC->FindAbilitySpecFromHandle(Handle) : nullptr;
	Result.Ability = Spec ? Cast<USigilArsenalTestAmmoFireAbility>(Spec->GetPrimaryInstance()) : nullptr;
	return Result;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalAmmoConsumptionTest,
	"SigilArsenal.Ammo.ConsumesUntilEmptyAndRefills",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalAmmoConsumptionTest::RunTest(const FString& Parameters)
{
	FSigilArsenalTestWorld Fixture(TEXT("SigilArsenalAmmoConsumptionWorld"));
	if (!TestNotNull(TEXT("测试世界已创建"), Fixture.World))
	{
		return false;
	}
	ASigilArsenalTestPawn* Pawn = SpawnReadyPawn(Fixture.World, MakeWeaponSlotCollection());
	if (!TestNotNull(TEXT("测试 Pawn 已创建"), Pawn))
	{
		return false;
	}
	const FSigilArsenalAmmoWeapon Weapon = AddAmmoWeapon(Pawn, TEXT("SigilArsenalAmmoPistol"), SigilArsenalTestTags::SlotPrimary);
	if (!TestTrue(TEXT("武器、物品及弹药技能准备完毕"), Weapon.IsReady()))
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Pawn->GetSigilAbilitySystem();
	const FGameplayAbilitySpecHandle Handle = Weapon.Ability->GetCurrentAbilitySpecHandle();
	TestTrue(TEXT("动态属性片段创建了弹匣属性"), Weapon.Item->HasIntegerAttribute(SigilArsenalTags::Ammo_Magazine));
	TestEqual(TEXT("片段将弹匣初始化为两发"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 2);
	TestTrue(TEXT("第一发激活成功"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("第一发后剩一发"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 1);
	TestTrue(TEXT("第二发激活成功"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("第二发后弹匣耗尽"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 0);
	TestEqual(TEXT("恰有两次成功提交"), Weapon.Ability->ActivationCount, 2);

	FGameplayTagContainer FailureTags;
	int32 FailureCount = 0;
	const FDelegateHandle FailureHandle = ASC->AbilityFailedCallbacks.AddLambda([&FailureTags, &FailureCount](const UGameplayAbility*, const FGameplayTagContainer& Tags)
	{
		FailureTags.AppendTags(Tags);
		++FailureCount;
	});
	TestFalse(TEXT("第三发因空弹匣无法激活"), ASC->TryActivateAbility(Handle));
	ASC->AbilityFailedCallbacks.Remove(FailureHandle);
	TestEqual(TEXT("空弹匣触发一次激活失败回调"), FailureCount, 1);
	TestTrue(TEXT("失败回调包含弹药不足标签"), FailureTags.HasTagExact(SigilArsenalTags::Ability_Fail_Ammo));
	TestEqual(TEXT("空弹匣未变成负数"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 0);
	TestEqual(TEXT("失败没有计为成功射击"), Weapon.Ability->ActivationCount, 2);

	Weapon.Item->SetIntegerAttribute(SigilArsenalTags::Ammo_Magazine, 1);
	TestTrue(TEXT("补入一发后可再次激活"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("补入的一发被消耗"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 0);
	TestEqual(TEXT("补弹后总共成功射击三次"), Weapon.Ability->ActivationCount, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalAmmoSourceIsolationTest,
	"SigilArsenal.Ammo.UsesRequestedSpecSourcePerWeapon",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalAmmoSourceIsolationTest::RunTest(const FString& Parameters)
{
	FSigilArsenalTestWorld Fixture(TEXT("SigilArsenalAmmoSourceWorld"));
	if (!TestNotNull(TEXT("测试世界已创建"), Fixture.World))
	{
		return false;
	}
	ASigilArsenalTestPawn* Pawn = SpawnReadyPawn(Fixture.World, MakeWeaponSlotCollection());
	if (!TestNotNull(TEXT("测试 Pawn 已创建"), Pawn))
	{
		return false;
	}
	const FSigilArsenalAmmoWeapon Pistol = AddAmmoWeapon(Pawn, TEXT("SigilArsenalAmmoSourcePistol"), SigilArsenalTestTags::SlotPrimary);
	const FSigilArsenalAmmoWeapon Rifle = AddAmmoWeapon(Pawn, TEXT("SigilArsenalAmmoSourceRifle"), SigilArsenalTestTags::SlotSecondary);
	if (!TestTrue(TEXT("两把武器均已独立授予同类技能"), Pistol.IsReady() && Rifle.IsReady()))
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Pawn->GetSigilAbilitySystem();
	const FGameplayAbilitySpecHandle PistolHandle = Pistol.Ability->GetCurrentAbilitySpecHandle();
	const FGameplayAbilitySpecHandle RifleHandle = Rifle.Ability->GetCurrentAbilitySpecHandle();
	TestTrue(TEXT("手枪第一发成功"), ASC->TryActivateAbility(PistolHandle));
	TestTrue(TEXT("手枪第二发成功"), ASC->TryActivateAbility(PistolHandle));
	TestEqual(TEXT("手枪弹匣耗尽"), Pistol.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 0);
	TestEqual(TEXT("手枪射击不消耗步枪的两发弹药"), Rifle.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 2);

	// 显式传入另一 Spec，防止成本错误读取 Ability 当前 Spec 或全局激活武器。
	USigilAbilityCost_ItemIntegerAttribute* Cost = Pistol.Ability->GetAmmoCostForTest();
	const FGameplayAbilityActorInfo* ActorInfo = Pistol.Ability->GetCurrentActorInfo();
	FGameplayTagContainer FailureTags;
	TestFalse(TEXT("手枪 Spec 的空弹匣无法支付"), Cost->CheckCost(Pistol.Ability, PistolHandle, ActorInfo, &FailureTags));
	TestTrue(TEXT("同一技能指针配步枪 Spec 时读取步枪余额"), Cost->CheckCost(Pistol.Ability, RifleHandle, ActorInfo, nullptr));
	Cost->ApplyCost(Pistol.Ability, RifleHandle, ActorInfo, Pistol.Ability->GetCurrentActivationInfo());
	TestEqual(TEXT("显式步枪 Spec 只扣步枪一发"), Rifle.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 1);
	TestEqual(TEXT("显式步枪 Spec 不改变手枪余额"), Pistol.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 0);

	Pawn->GetEquipment()->SetGroupActiveIndex(SigilArsenalTestTags::SlotWeaponGroup, 1);
	TestTrue(TEXT("切换到步枪后可射出剩余一发"), ASC->TryActivateAbility(RifleHandle));
	TestEqual(TEXT("步枪独立耗尽"), Rifle.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 0);
	TestEqual(TEXT("步枪只记录自己的那次激活"), Rifle.Ability->ActivationCount, 1);
	Pawn->GetEquipment()->SetGroupActiveIndex(SigilArsenalTestTags::SlotWeaponGroup, 0);
	TestFalse(TEXT("切回空手枪后仍不能激活"), ASC->TryActivateAbility(PistolHandle));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalAmmoMissingSourceTest,
	"SigilArsenal.Ammo.RejectsMissingAttributeAndSource",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalAmmoMissingSourceTest::RunTest(const FString& Parameters)
{
	FSigilArsenalTestWorld Fixture(TEXT("SigilArsenalAmmoMissingWorld"));
	if (!TestNotNull(TEXT("测试世界已创建"), Fixture.World))
	{
		return false;
	}
	ASigilArsenalTestPawn* Pawn = SpawnReadyPawn(Fixture.World, MakeWeaponSlotCollection());
	if (!TestNotNull(TEXT("测试 Pawn 已创建"), Pawn))
	{
		return false;
	}
	const FSigilArsenalAmmoWeapon Weapon = AddAmmoWeapon(Pawn, TEXT("SigilArsenalNoMagazine"), SigilArsenalTestTags::SlotPrimary, false);
	if (!TestTrue(TEXT("缺弹匣属性的武器及技能已准备好"), Weapon.IsReady()))
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Pawn->GetSigilAbilitySystem();
	USigilAbilityCost_ItemIntegerAttribute* Cost = Weapon.Ability->GetAmmoCostForTest();
	const FGameplayAbilitySpecHandle Handle = Weapon.Ability->GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = Weapon.Ability->GetCurrentActorInfo();
	FGameplayTagContainer FailureTags;
	TestFalse(TEXT("缺少弹匣属性时成本检查失败"), Cost->CheckCost(Weapon.Ability, Handle, ActorInfo, &FailureTags));
	TestTrue(TEXT("缺少属性也返回弹药失败标签"), FailureTags.HasTagExact(SigilArsenalTags::Ability_Fail_Ammo));
	TestFalse(TEXT("缺少属性时完整技能激活失败"), ASC->TryActivateAbility(Handle));
	Cost->ApplyCost(Weapon.Ability, Handle, ActorInfo, Weapon.Ability->GetCurrentActivationInfo());
	TestFalse(TEXT("Apply 不凭空创建缺失的弹匣属性"), Weapon.Item->HasIntegerAttribute(SigilArsenalTags::Ammo_Magazine));

	// 分别覆盖空来源、非武器来源，以及没有关联物品的武器装备来源。
	UObject* Sources[] = {nullptr, Pawn, NewObject<USigilWeaponEquipmentInstance>(Pawn)};
	const TCHAR* Labels[] = {TEXT("空来源"), TEXT("非武器来源"), TEXT("武器缺源物品")};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Sources); ++Index)
	{
		const FGameplayAbilitySpecHandle MissingHandle = ASC->GiveAbility(FGameplayAbilitySpec(USigilArsenalTestAmmoFireAbility::StaticClass(), 1, INDEX_NONE, Sources[Index]));
		FailureTags.Reset();
		TestFalse(FString::Printf(TEXT("%s 无法支付弹药"), Labels[Index]), Cost->CheckCost(Weapon.Ability, MissingHandle, ActorInfo, &FailureTags));
		TestTrue(FString::Printf(TEXT("%s 返回弹药失败标签"), Labels[Index]), FailureTags.HasTagExact(SigilArsenalTags::Ability_Fail_Ammo));
		Cost->ApplyCost(Weapon.Ability, MissingHandle, ActorInfo, Weapon.Ability->GetCurrentActivationInfo());
		ASC->ClearAbility(MissingHandle);
	}
	TestEqual(TEXT("所有无效来源均未产生成功射击"), Weapon.Ability->ActivationCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalAmmoApplyGuardsTest,
	"SigilArsenal.Ammo.RequiresAuthorityAndRechecksQuantity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalAmmoApplyGuardsTest::RunTest(const FString& Parameters)
{
	FSigilArsenalTestWorld Fixture(TEXT("SigilArsenalAmmoGuardsWorld"));
	if (!TestNotNull(TEXT("测试世界已创建"), Fixture.World))
	{
		return false;
	}
	ASigilArsenalTestPawn* Pawn = SpawnReadyPawn(Fixture.World, MakeWeaponSlotCollection());
	if (!TestNotNull(TEXT("测试 Pawn 已创建"), Pawn))
	{
		return false;
	}
	const FSigilArsenalAmmoWeapon Weapon = AddAmmoWeapon(Pawn, TEXT("SigilArsenalAmmoGuards"), SigilArsenalTestTags::SlotPrimary);
	if (!TestTrue(TEXT("权限测试的武器及技能已准备好"), Weapon.IsReady()))
	{
		return false;
	}

	USigilAbilityCost_ItemIntegerAttribute* Cost = Weapon.Ability->GetAmmoCostForTest();
	const FGameplayAbilitySpecHandle Handle = Weapon.Ability->GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = Weapon.Ability->GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = Weapon.Ability->GetCurrentActivationInfo();
	// 仅模拟本地角色权限分支；不把此断言当成预测或联网复制验证。
	const ENetRole OriginalRole = Pawn->GetLocalRole();
	Pawn->SetRole(ROLE_AutonomousProxy);
	TestFalse(TEXT("测试 Pawn 暂时没有 authority"), Pawn->HasAuthority());
	TestTrue(TEXT("非 authority 可根据已有弹药做成本检查"), Cost->CheckCost(Weapon.Ability, Handle, ActorInfo, nullptr));
	Cost->ApplyCost(Weapon.Ability, Handle, ActorInfo, ActivationInfo);
	Pawn->SetRole(OriginalRole);
	TestEqual(TEXT("非 authority Apply 不扣弹药"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 2);

	Cost->Quantity = 2;
	TestTrue(TEXT("恰有两发时可以支付数量二"), Cost->CheckCost(Weapon.Ability, Handle, ActorInfo, nullptr));
	Weapon.Item->SetIntegerAttribute(SigilArsenalTags::Ammo_Magazine, 1);
	Cost->ApplyCost(Weapon.Ability, Handle, ActorInfo, ActivationInfo);
	TestEqual(TEXT("检查后余额下降时 Apply 重新检查，不扣成负数"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 1);
	Weapon.Item->SetIntegerAttribute(SigilArsenalTags::Ammo_Magazine, 2);
	Cost->ApplyCost(Weapon.Ability, Handle, ActorInfo, ActivationInfo);
	TestEqual(TEXT("authority 按配置数量二一次扣完"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 0);

	Weapon.Item->SetIntegerAttribute(SigilArsenalTags::Ammo_Magazine, 2);
	const int32 InvalidQuantities[] = {0, -1};
	for (const int32 InvalidQuantity : InvalidQuantities)
	{
		Cost->Quantity = InvalidQuantity;
		FGameplayTagContainer FailureTags;
		TestFalse(FString::Printf(TEXT("数量 %d 被拒绝"), InvalidQuantity), Cost->CheckCost(Weapon.Ability, Handle, ActorInfo, &FailureTags));
		TestTrue(TEXT("非法数量返回弹药失败标签"), FailureTags.HasTagExact(SigilArsenalTags::Ability_Fail_Ammo));
		Cost->ApplyCost(Weapon.Ability, Handle, ActorInfo, ActivationInfo);
		TestEqual(TEXT("非法数量不会扣弹或反向增加弹药"), Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 2);
	}
	Cost->Quantity = 1;
	return true;
}
#endif
