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
#include "Tests/SigilArsenalCadenceTestTypes.h"
#include "Tests/SigilArsenalTestTypes.h"
#include "TimerManager.h"
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

/** 复用库存与装备夹具，只增加节奏技能的手动授予和确定性计时推进。 */
struct FSigilArsenalCadenceFixture
{
	FSigilArsenalTestWorld TestWorld;
	ASigilArsenalTestPawn* Pawn = nullptr;
	USigilAbilitySystemComponent* ASC = nullptr;
	FSigilArsenalAmmoWeapon Weapon;

	explicit FSigilArsenalCadenceFixture(const TCHAR* Name) : TestWorld(Name)
	{
		if (!TestWorld.World)
		{
			return;
		}
		Pawn = SpawnReadyPawn(TestWorld.World, MakeWeaponSlotCollection());
		if (Pawn)
		{
			ASC = Pawn->GetSigilAbilitySystem();
			const FString DefinitionName = FString(Name) + TEXT("Gun");
			Weapon = AddAmmoWeapon(Pawn, *DefinitionName, SigilArsenalTestTags::SlotPrimary);
		}
	}

	bool IsReady() const { return TestWorld.World && Pawn && ASC && Weapon.IsReady(); }

	template <typename TCadence>
	TCadence* GiveCadence(USigilWeaponEquipmentInstance* Source = nullptr)
	{
		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(FGameplayAbilitySpec(TCadence::StaticClass(), 1, INDEX_NONE, Source ? Source : Weapon.Equipment));
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
		return Spec ? Cast<TCadence>(Spec->GetPrimaryInstance()) : nullptr;
	}

	void TickTimers(float DeltaSeconds)
	{
		// UE 每帧只 Tick 一次；首次 Tick(0) 把 Pending 定时器转成可运行状态。
		++GFrameCounter;
		TestWorld.World->GetTimerManager().Tick(DeltaSeconds);
	}
};
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalCadenceSemiAutoTest,
	"SigilArsenal.Cadence.SemiAutoFiresOnceAndEnds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalCadenceSemiAutoTest::RunTest(const FString& Parameters)
{
	FSigilArsenalCadenceFixture Fixture(TEXT("SigilArsenalSemiAutoWorld"));
	if (!TestTrue(TEXT("节奏夹具准备完毕"), Fixture.IsReady()))
	{
		return false;
	}
	TestTrue(TEXT("本地节奏技能使用可本地控制的 ActorInfo"), Fixture.ASC->AbilityActorInfo->IsLocallyControlled());
	USigilArsenalTestCountingCadence* Ability = Fixture.GiveCadence<USigilArsenalTestCountingCadence>();
	if (!TestNotNull(TEXT("半自动节奏技能已授予"), Ability))
	{
		return false;
	}
	Ability->FireMode = ESigilFireMode::SemiAuto;
	Ability->RoundsPerMinute = 600.f;
	TestTrue(TEXT("半自动技能可激活"), Fixture.ASC->TryActivateAbility(Ability->GetCurrentAbilitySpecHandle()));
	TestEqual(TEXT("激活立即成功射击一发"), Ability->SuccessfulShotCount, 1);
	TestFalse(TEXT("半自动首发后立即结束"), Ability->IsActive());
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("半自动结束后不残留后续射击"), Ability->AttemptCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalCadenceBurstTest,
	"SigilArsenal.Cadence.BurstFiresConfiguredCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalCadenceBurstTest::RunTest(const FString& Parameters)
{
	FSigilArsenalCadenceFixture Fixture(TEXT("SigilArsenalBurstWorld"));
	if (!TestTrue(TEXT("节奏夹具准备完毕"), Fixture.IsReady()))
	{
		return false;
	}
	USigilArsenalTestCountingCadence* Ability = Fixture.GiveCadence<USigilArsenalTestCountingCadence>();
	if (!TestNotNull(TEXT("点射节奏技能已授予"), Ability))
	{
		return false;
	}
	Ability->FireMode = ESigilFireMode::Burst;
	Ability->RoundsPerMinute = 600.f;
	Ability->BurstCount = 3;
	const FGameplayAbilitySpecHandle Handle = Ability->GetCurrentAbilitySpecHandle();
	TestTrue(TEXT("三连发技能可激活"), Fixture.ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("三连发首发立即发生"), Ability->SuccessfulShotCount, 1);
	TestTrue(TEXT("首发后等待后续点射"), Ability->IsActive());
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.11f);
	TestEqual(TEXT("经过第一个间隔累计两发"), Ability->SuccessfulShotCount, 2);
	Fixture.TickTimers(0.11f);
	TestEqual(TEXT("三连发恰好成功三次"), Ability->SuccessfulShotCount, 3);
	TestFalse(TEXT("到达三次后结束"), Ability->IsActive());
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("点射结束后没有额外尝试"), Ability->AttemptCount, 3);

	Ability->BurstCount = 1;
	TestTrue(TEXT("数量一的点射可再次激活"), Fixture.ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("新一轮独立完成一发"), Ability->SuccessfulShotCount, 4);
	TestFalse(TEXT("数量一的点射立即结束"), Ability->IsActive());
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.35f);
	TestEqual(TEXT("数量一也没有重复射击"), Ability->AttemptCount, 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalCadenceFullAutoTest,
	"SigilArsenal.Cadence.FullAutoUsesConfiguredInterval",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalCadenceFullAutoTest::RunTest(const FString& Parameters)
{
	FSigilArsenalCadenceFixture Fixture(TEXT("SigilArsenalFullAutoWorld"));
	if (!TestTrue(TEXT("节奏夹具准备完毕"), Fixture.IsReady()))
	{
		return false;
	}
	USigilArsenalTestCountingCadence* Ability = Fixture.GiveCadence<USigilArsenalTestCountingCadence>();
	if (!TestNotNull(TEXT("全自动节奏技能已授予"), Ability))
	{
		return false;
	}
	Ability->FireMode = ESigilFireMode::FullAuto;
	Ability->RoundsPerMinute = 300.f;
	const FGameplayAbilitySpecHandle Handle = Ability->GetCurrentAbilitySpecHandle();
	TestTrue(TEXT("全自动技能可激活"), Fixture.ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("第一个循环回调之前已有首发"), Ability->SuccessfulShotCount, 1);
	Fixture.TickTimers(0.f);
	TestEqual(TEXT("激活 Pending 定时器不会重复首发"), Ability->SuccessfulShotCount, 1);
	// 每分钟 300 发即每 0.2 秒一发；0.45 秒内追加两发，首发单独计入。
	Fixture.TickTimers(0.45f);
	TestEqual(TEXT("零点四五秒内追加两发"), Ability->SuccessfulShotCount, 3);
	Fixture.TickTimers(0.21f);
	TestEqual(TEXT("累计零点六六秒内追加三发"), Ability->SuccessfulShotCount, 4);
	Fixture.TickTimers(0.02f);
	TestEqual(TEXT("不足下一个间隔时不会提前射击"), Ability->SuccessfulShotCount, 4);
	TestTrue(TEXT("有持续输入时全自动保持运行"), Ability->IsActive());
	Fixture.ASC->CancelAbilityHandle(Handle);
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("取消后不再追加射击"), Ability->SuccessfulShotCount, 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalCadenceAmmoRoutingTest,
	"SigilArsenal.Cadence.DefaultRouteUsesSourceAmmoAndStopsWhenEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalCadenceAmmoRoutingTest::RunTest(const FString& Parameters)
{
	FSigilArsenalCadenceFixture Fixture(TEXT("SigilArsenalCadenceAmmoWorld"));
	if (!TestTrue(TEXT("节奏夹具准备完毕"), Fixture.IsReady()))
	{
		return false;
	}
	const FSigilArsenalAmmoWeapon Rifle = AddAmmoWeapon(Fixture.Pawn, TEXT("SigilArsenalCadenceAmmoRifle"), SigilArsenalTestTags::SlotSecondary);
	if (!TestTrue(TEXT("第二把武器也授予同类单发技能"), Rifle.IsReady()))
	{
		return false;
	}
	USigilArsenalTestRoutedCadence* PistolCadence = Fixture.GiveCadence<USigilArsenalTestRoutedCadence>();
	USigilArsenalTestRoutedCadence* RifleCadence = Fixture.GiveCadence<USigilArsenalTestRoutedCadence>(Rifle.Equipment);
	if (!TestTrue(TEXT("两把武器均有独立节奏 Spec"), PistolCadence && RifleCadence))
	{
		return false;
	}
	for (USigilArsenalTestRoutedCadence* Cadence : {PistolCadence, RifleCadence})
	{
		Cadence->FireMode = ESigilFireMode::FullAuto;
		Cadence->RoundsPerMinute = 600.f;
		Cadence->SingleShotAbilityClass = USigilArsenalTestAmmoFireAbility::StaticClass();
	}

	TestTrue(TEXT("手枪默认路由可激活"), Fixture.ASC->TryActivateAbility(PistolCadence->GetCurrentAbilitySpecHandle()));
	TestEqual(TEXT("首发立即消耗手枪一发"), Fixture.Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 1);
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.11f);
	TestEqual(TEXT("后续间隔消耗手枪最后一发"), Fixture.Weapon.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 0);
	Fixture.TickTimers(0.11f);
	TestFalse(TEXT("空弹匣的单发失败立即结束节奏"), PistolCadence->IsActive());
	TestEqual(TEXT("手枪单发技能恰好成功两次"), Fixture.Weapon.Ability->ActivationCount, 2);
	TestEqual(TEXT("手枪耗尽不动步枪两发弹药"), Rifle.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 2);
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("耗尽结束后不再激活单发"), Fixture.Weapon.Ability->ActivationCount, 2);

	Fixture.Pawn->GetEquipment()->SetGroupActiveIndex(SigilArsenalTestTags::SlotWeaponGroup, 1);
	TestTrue(TEXT("步枪节奏可独立激活"), Fixture.ASC->TryActivateAbility(RifleCadence->GetCurrentAbilitySpecHandle()));
	TestEqual(TEXT("步枪默认路由找到同源单发并消耗一发"), Rifle.Item->GetIntegerAttribute(SigilArsenalTags::Ammo_Magazine), 1);
	TestEqual(TEXT("步枪单发技能成功一次"), Rifle.Ability->ActivationCount, 1);
	TestEqual(TEXT("切枪不会再次激活手枪单发"), Fixture.Weapon.Ability->ActivationCount, 2);
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.11f);
	Fixture.TickTimers(0.11f);
	TestFalse(TEXT("步枪自己的弹药耗尽后同样结束"), RifleCadence->IsActive());
	TestEqual(TEXT("步枪独立射完两发"), Rifle.Ability->ActivationCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalCadenceLifecycleTest,
	"SigilArsenal.Cadence.ReleaseDeactivationCancelAndRemovalClearTimers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalCadenceLifecycleTest::RunTest(const FString& Parameters)
{
	FSigilArsenalCadenceFixture Fixture(TEXT("SigilArsenalCadenceLifecycleWorld"));
	if (!TestTrue(TEXT("节奏夹具准备完毕"), Fixture.IsReady()))
	{
		return false;
	}
	const FSigilArsenalAmmoWeapon Rifle = AddAmmoWeapon(Fixture.Pawn, TEXT("SigilArsenalCadenceLifecycleRifle"), SigilArsenalTestTags::SlotSecondary);
	USigilArsenalTestCountingCadence* Ability = Fixture.GiveCadence<USigilArsenalTestCountingCadence>();
	if (!TestTrue(TEXT("切枪及节奏对象准备完毕"), Rifle.IsReady() && Ability))
	{
		return false;
	}
	Ability->FireMode = ESigilFireMode::FullAuto;
	Ability->RoundsPerMinute = 600.f;
	const FGameplayAbilitySpecHandle Handle = Ability->GetCurrentAbilitySpecHandle();

	TestTrue(TEXT("释放输入场景可激活"), Fixture.ASC->TryActivateAbility(Handle));
	Fixture.TickTimers(0.f);
	USigilAbilitySystemFunctionLibrary::SetAbilityInputReleased(Fixture.ASC, Handle);
	TestFalse(TEXT("释放输入立即结束，不等待下次计时"), Ability->IsActive());
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("释放输入后没有残余射击"), Ability->AttemptCount, 1);

	TestTrue(TEXT("失活场景可重新激活"), Fixture.ASC->TryActivateAbility(Handle));
	Fixture.TickTimers(0.f);
	Fixture.Pawn->GetEquipment()->SetGroupActiveIndex(SigilArsenalTestTags::SlotWeaponGroup, 1);
	TestFalse(TEXT("源武器失活立即结束节奏"), Ability->IsActive());
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("武器失活后没有残余射击"), Ability->AttemptCount, 2);
	TestFalse(TEXT("失活源武器不能重新激活节奏"), Fixture.ASC->TryActivateAbility(Handle));

	Fixture.Pawn->GetEquipment()->SetGroupActiveIndex(SigilArsenalTestTags::SlotWeaponGroup, 0);
	TestTrue(TEXT("取消场景可重新激活"), Fixture.ASC->TryActivateAbility(Handle));
	Fixture.TickTimers(0.f);
	Fixture.ASC->CancelAbilityHandle(Handle);
	TestFalse(TEXT("显式取消立即结束节奏"), Ability->IsActive());
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("取消后没有残余射击"), Ability->AttemptCount, 3);

	TestTrue(TEXT("移除场景可重新激活"), Fixture.ASC->TryActivateAbility(Handle));
	Fixture.TickTimers(0.f);
	Fixture.ASC->ClearAbility(Handle);
	TestNull(TEXT("移除后 Spec 已不在 ASC 中"), Fixture.ASC->FindAbilitySpecFromHandle(Handle));
	Fixture.Pawn->GetEquipment()->SetGroupActiveIndex(SigilArsenalTestTags::SlotWeaponGroup, 1);
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("移除后计时与武器事件都不再触发射击"), Ability->AttemptCount, 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalCadenceInvalidConfigTest,
	"SigilArsenal.Cadence.InvalidConfigAndShotFailureStopSafely",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalCadenceInvalidConfigTest::RunTest(const FString& Parameters)
{
	FSigilArsenalCadenceFixture Fixture(TEXT("SigilArsenalCadenceInvalidWorld"));
	if (!TestTrue(TEXT("节奏夹具准备完毕"), Fixture.IsReady()))
	{
		return false;
	}
	USigilArsenalTestCountingCadence* Ability = Fixture.GiveCadence<USigilArsenalTestCountingCadence>();
	if (!TestNotNull(TEXT("配置验证节奏技能已授予"), Ability))
	{
		return false;
	}
	const FGameplayAbilitySpecHandle Handle = Ability->GetCurrentAbilitySpecHandle();
	Ability->FireMode = ESigilFireMode::FullAuto;
	const float InvalidRates[] = {0.f, -600.f};
	for (const float Rate : InvalidRates)
	{
		Ability->RoundsPerMinute = Rate;
		Fixture.ASC->TryActivateAbility(Handle);
		TestFalse(TEXT("非法 RPM 不留下运行中的节奏"), Ability->IsActive());
		Fixture.TickTimers(0.f);
		Fixture.TickTimers(0.35f);
		TestEqual(TEXT("非法 RPM 不尝试首发或后续射击"), Ability->AttemptCount, 0);
	}
	Ability->RoundsPerMinute = 600.f;
	Ability->FireMode = ESigilFireMode::Burst;
	const int32 InvalidBurstCounts[] = {0, -1};
	for (const int32 Count : InvalidBurstCounts)
	{
		Ability->BurstCount = Count;
		Fixture.ASC->TryActivateAbility(Handle);
		TestFalse(TEXT("非法点射数量不留下运行中的节奏"), Ability->IsActive());
		Fixture.TickTimers(0.f);
		Fixture.TickTimers(0.35f);
		TestEqual(TEXT("非法点射数量不尝试射击"), Ability->AttemptCount, 0);
	}

	Ability->BurstCount = 3;
	Ability->FireMode = ESigilFireMode::FullAuto;
	Ability->MaxSuccessfulShots = 0;
	Fixture.ASC->TryActivateAbility(Handle);
	TestEqual(TEXT("首发失败仅尝试一次"), Ability->AttemptCount, 1);
	TestFalse(TEXT("首发失败立即结束"), Ability->IsActive());
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.35f);
	TestEqual(TEXT("首发失败后不再重试"), Ability->AttemptCount, 1);

	Ability->MaxSuccessfulShots = 1;
	TestTrue(TEXT("允许一发后可以再次激活"), Fixture.ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("此次首发成功"), Ability->SuccessfulShotCount, 1);
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.11f);
	TestFalse(TEXT("后续单发失败也立即结束"), Ability->IsActive());
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("总共只有首次失败和此轮两次尝试"), Ability->AttemptCount, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalCadenceReentryTest,
	"SigilArsenal.Cadence.CallbackRestartPreservesNewActivation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalCadenceReentryTest::RunTest(const FString& Parameters)
{
	FSigilArsenalCadenceFixture Fixture(TEXT("SigilArsenalCadenceReentryWorld"));
	if (!TestTrue(TEXT("节奏夹具准备完毕"), Fixture.IsReady()))
	{
		return false;
	}
	USigilArsenalTestReentrantCadence* Ability = Fixture.GiveCadence<USigilArsenalTestReentrantCadence>();
	if (!TestNotNull(TEXT("重入节奏技能已授予"), Ability))
	{
		return false;
	}
	Ability->FireMode = ESigilFireMode::FullAuto;
	Ability->RoundsPerMinute = 600.f;
	const FGameplayAbilitySpecHandle Handle = Ability->GetCurrentAbilitySpecHandle();
	TestTrue(TEXT("第一轮节奏可激活"), Fixture.ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("第一轮首发成功"), Ability->SuccessfulShotCount, 1);
	Fixture.TickTimers(0.f);
	Ability->bRestartOnNextShot = true;
	Fixture.TickTimers(0.11f);
	TestEqual(TEXT("仅在指定回调内重启一次"), Ability->RestartCount, 1);
	TestTrue(TEXT("回调内取消后的重新激活成功"), Ability->bRestartSucceeded);
	TestTrue(TEXT("旧回调返回 false 不会结束新一轮"), Ability->IsActive());
	TestEqual(TEXT("旧轮两发加新轮首发共三发"), Ability->SuccessfulShotCount, 3);
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.11f);
	TestEqual(TEXT("新一轮的定时器继续追加一发"), Ability->SuccessfulShotCount, 4);
	Fixture.TickTimers(0.11f);
	TestEqual(TEXT("新一轮没有受到旧定时器重复调度"), Ability->SuccessfulShotCount, 5);
	Fixture.ASC->CancelAbilityHandle(Handle);
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("最终取消能清理新一轮定时器"), Ability->SuccessfulShotCount, 5);

	// 首发也可能重入；旧激活返回后不能再额外登记一个循环定时器。
	Ability->bRestartOnNextShot = true;
	TestTrue(TEXT("首发重入场景可激活"), Fixture.ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("首发路径也只重启一次"), Ability->RestartCount, 2);
	TestTrue(TEXT("首发重启后新一轮保持运行"), Ability->IsActive());
	TestEqual(TEXT("本次旧轮首发与新轮首发各一次"), Ability->SuccessfulShotCount, 7);
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.11f);
	TestEqual(TEXT("首发重入后只保留新一轮的一个计时器"), Ability->SuccessfulShotCount, 8);
	Fixture.ASC->CancelAbilityHandle(Handle);
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("首发重入后同样可完整清理"), Ability->SuccessfulShotCount, 8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilArsenalCadenceDeferredEndTest,
	"SigilArsenal.Cadence.QueuedOldEndDoesNotStopRestartedActivation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilArsenalCadenceDeferredEndTest::RunTest(const FString& Parameters)
{
	FSigilArsenalCadenceFixture Fixture(TEXT("SigilArsenalCadenceDeferredEndWorld"));
	if (!TestTrue(TEXT("节奏夹具准备完毕"), Fixture.IsReady()))
	{
		return false;
	}
	USigilArsenalTestCountingCadence* Ability = Fixture.GiveCadence<USigilArsenalTestCountingCadence>();
	if (!TestNotNull(TEXT("延后结束回归技能已授予"), Ability))
	{
		return false;
	}
	Ability->FireMode = ESigilFireMode::FullAuto;
	Ability->RoundsPerMinute = 600.f;
	const FGameplayAbilitySpecHandle Handle = Ability->GetCurrentAbilitySpecHandle();
	TestTrue(TEXT("第一轮节奏可激活"), Fixture.ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("第一轮已有首发"), Ability->SuccessfulShotCount, 1);
	Fixture.TickTimers(0.f);

	int32 EndedCount = 0;
	bool bRestartSucceeded = false;
	const FDelegateHandle EndedHandle = Fixture.ASC->AbilityEndedCallbacks.AddLambda([&Fixture, Ability, Handle, &EndedCount, &bRestartSucceeded](UGameplayAbility* EndedAbility)
	{
		if (EndedAbility != Ability)
		{
			return;
		}
		++EndedCount;
		if (EndedCount == 1)
		{
			bRestartSucceeded = Fixture.ASC->TryActivateAbility(Handle);
		}
	});
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo OldActivationInfo = Ability->GetCurrentActivationInfo();
	Ability->BeginScopeLockForTest();
	Ability->EndAbility(Handle, ActorInfo, OldActivationInfo, false, true);
	Ability->EndAbility(Handle, ActorInfo, OldActivationInfo, false, true);
	TestEqual(TEXT("作用域锁内两个结束请求都尚未触发结束回调"), EndedCount, 0);
	Ability->EndScopeLockForTest();
	Fixture.ASC->AbilityEndedCallbacks.Remove(EndedHandle);

	TestTrue(TEXT("第一条排队 End 的回调成功重启技能"), bRestartSucceeded);
	TestEqual(TEXT("第二条旧 End 没有再结束新一轮"), EndedCount, 1);
	TestTrue(TEXT("排队结束请求处理完后新一轮仍运行"), Ability->IsActive());
	TestEqual(TEXT("新旧两轮各有一次首发"), Ability->SuccessfulShotCount, 2);
	Fixture.TickTimers(0.f);
	Fixture.TickTimers(0.11f);
	TestEqual(TEXT("第二条旧 End 没有清除新一轮定时器"), Ability->SuccessfulShotCount, 3);
	Fixture.ASC->CancelAbilityHandle(Handle);
	Fixture.TickTimers(0.75f);
	TestEqual(TEXT("结束回归最后没有残余定时射击"), Ability->SuccessfulShotCount, 3);
	return true;
}

#endif
