// Copyright (c) 2026 Likeon. All Rights Reserved.
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/SigilEquipmentLifecycleTestTypes.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "NativeGameplayTags.h"
#include "SigilInventoryTags.h"
#include "SigilItemDefinition.h"
#include "SigilItemInstance.h"
#include "SigilItemFragment_Equippable.h"
#include "SigilItemSlotCollection.h"

namespace
{
UE_DEFINE_GAMEPLAY_TAG_STATIC(LifecycleGroup, "Sigil.Test.EquipmentLifecycle.Slots");
UE_DEFINE_GAMEPLAY_TAG_STATIC(LifecycleA, "Sigil.Test.EquipmentLifecycle.Slots.A");
UE_DEFINE_GAMEPLAY_TAG_STATIC(LifecycleB, "Sigil.Test.EquipmentLifecycle.Slots.B");
UE_DEFINE_GAMEPLAY_TAG_STATIC(LifecycleC, "Sigil.Test.EquipmentLifecycle.Slots.C");

struct FLifecycleFixture
{
	UGameInstance* Instance = nullptr;
	UWorld* World = nullptr;
	USigilLifecycleTestInventory* Inventory = nullptr;
	USigilLifecycleTestEquipment* Equipment = nullptr;
	TArray<USigilLifecycleTestInstance*> Items;
	const TArray<FGameplayTag> Slots { LifecycleA, LifecycleB, LifecycleC };

	explicit FLifecycleFixture(int32 Count, bool bResetOnAdded = false)
	{
		Instance = NewObject<UGameInstance>(GEngine);
		Instance->AddToRoot();
		Instance->InitializeStandalone(MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("EquipmentLifecycle")), GetTransientPackage());
		World = Instance->GetWorld();
		World->AddToRoot();
		World->InitializeActorsForPlay(FURL());
		World->GetWorldSettings()->NotifyBeginPlay();
		auto* Pawn = World->SpawnActor<ASigilLifecycleTestPawn>();
		Inventory = NewObject<USigilLifecycleTestInventory>(Pawn);
		Equipment = NewObject<USigilLifecycleTestEquipment>(Pawn);
		Pawn->AddInstanceComponent(Inventory);
		Pawn->AddInstanceComponent(Equipment);
		Inventory->RegisterComponent();
		Equipment->RegisterComponent();
		auto* Layout = NewObject<USigilItemSlotCollectionDefinition>();
		Layout->CollectionTag = SigilCollectionTags::Equipped;
		Layout->SlotGroups.Add(LifecycleGroup);
		auto& Group = Layout->SlotGroupMap.Add(LifecycleGroup);
		for (int32 Idx = 0; Idx < Slots.Num(); ++Idx)
		{
			auto& Slot = Layout->SlotDefinitions.AddDefaulted_GetRef();
			Slot.Tag = Slots[Idx];
			Slot.TagQuery = FGameplayTagQuery::MakeQuery_MatchTag(Slots[Idx]);
			Layout->IndexToTagMap.Add(Idx, Slots[Idx]);
			Layout->TagToIndexMap.Add(Slots[Idx], Idx);
			Group.IndexToSlotMap.Add(Idx, Slots[Idx]);
			Group.SlotToIndexMap.Add(Slots[Idx], Idx);
		}
		Inventory->Configure(Layout);
		Equipment->Configure(SigilCollectionTags::Equipped);
		Equipment->bResetOnAdded = bResetOnAdded;
		Equipment->OnEquipmentStateChangedEvent.AddDynamic(Equipment, &USigilLifecycleTestEquipment::ObserveEquipment);
		Inventory->InitializeInventorySystem();
		Equipment->InitializeEquipmentSystemWithInventory(Inventory);
		for (int32 Idx = 0; Idx < Count; ++Idx)
		{
			auto* Definition = NewObject<USigilItemDefinition>();
			Definition->bUnique = true;
			Definition->ItemTags.AddTag(Slots[Idx]);
			auto* Fragment = NewObject<USigilItemFragment_Equippable>(Definition);
			Fragment->InstanceType = USigilLifecycleTestInstance::StaticClass();
			Fragment->bAutoActivate = true;
			Definition->Fragments.Add(Fragment);
			Inventory->AddItemByDefinition(SigilCollectionTags::Equipped, Definition, 1);
			auto* Item = CastChecked<USigilLifecycleTestInstance>(Equipment->LastAdded);
			Item->System = Equipment;
			Item->Group = LifecycleGroup;
			Item->Slot = Slots[Idx];
			Items.Add(Item);
		}
	}
	~FLifecycleFixture()
	{
		Instance->Shutdown();
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		World->RemoveFromRoot();
		Instance->RemoveFromRoot();
	}
};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FSigilEquipmentLifecycleBasicTest,
	"SigilInventory.EquipmentLifecycle.Basic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

void FSigilEquipmentLifecycleBasicTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	Names.Add(TEXT("ReequipCurrentSlot")); Commands.Add(TEXT("reequip"));
	for (int32 Count = 0; Count <= 3; ++Count)
	{
		Names.Add(FString::Printf(TEXT("Reset%d"), Count)); Commands.Add(FString::FromInt(Count));
		Names.Add(FString::Printf(TEXT("RemoveAll%d"), Count)); Commands.Add(FString::Printf(TEXT("remove%d"), Count));
	}
}
bool FSigilEquipmentLifecycleBasicTest::RunTest(const FString& Parameters)
{
	const bool bReequip = Parameters == TEXT("reequip");
	const bool bRemoveOnly = Parameters.StartsWith(TEXT("remove"));
	FLifecycleFixture F(bReequip ? 1 : FCString::Atoi(*(bRemoveOnly ? Parameters.Mid(6) : Parameters)));
	if (bReequip)
	{
		auto* Source = F.Items[0]->GetSourceItem_Implementation();
		F.Equipment->UnequipBySlot(LifecycleA);
		F.Equipment->EquipItemToSlot(Source, LifecycleA);
		F.Equipment->SetGroupActiveIndex(LifecycleGroup, 0);
		TestNotNull(TEXT("重装同槽后应激活"), F.Equipment->GetActiveEquipment(
			UObject::StaticClass(), FGameplayTagQuery::MakeQuery_MatchTag(LifecycleA)));
		TestNull(TEXT("旧实例 Pawn 解绑"), F.Items[0]->GetOwningPawn_Implementation());
		return true;
	}
	if (bRemoveOnly)
	{
		F.Equipment->RemoveAllEquipments();
		F.Equipment->RemoveAllEquipments();
		TestTrue(TEXT("仅卸装不重置系统"), F.Equipment->IsEquipmentSystemInitialized());
	}
	else
	{
		F.Equipment->ResetEquipmentSystem();
		// 重复重置应幂等，不依赖初始化标志掩盖残留。
		F.Equipment->ResetEquipmentSystem();
	}
	for (int32 Idx = 0; Idx < F.Items.Num(); ++Idx)
	{
		TestNull(TEXT("实际槽清空"), F.Equipment->GetEquipmentInSlot(F.Slots[Idx]));
		TestNull(TEXT("来源解绑"), F.Items[Idx]->GetSourceItem_Implementation());
		TestNull(TEXT("Pawn 解绑"), F.Items[Idx]->GetOwningPawn_Implementation());
		TestEqual(TEXT("结束恰好一次"), F.Items[Idx]->EndCount, 1);
	}
	if (bRemoveOnly) { F.Equipment->ResetEquipmentSystem(); }
	F.Equipment->InitializeEquipmentSystemWithInventory(F.Inventory);
	TestEqual(TEXT("重新初始化不重复实例"),
		F.Equipment->GetEquipments(UObject::StaticClass(), FGameplayTagQuery::MakeQuery_MatchTag(LifecycleGroup)).Num(), F.Items.Num());
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FSigilEquipmentLifecycleReentrantTest,
	"SigilInventory.EquipmentLifecycle.Reentrant", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FSigilEquipmentLifecycleReentrantTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	Names = { TEXT("ResetDuringDeactivate"), TEXT("RemoveDuringDeactivate"), TEXT("NewSelectionDuringDeactivate"), TEXT("ResetDuringEnd"), TEXT("ResetDuringAdded"), TEXT("ResetDuringRemoveAll") };
	Commands = { TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6") };
	Names.Add(TEXT("FillEmptyTargetDuringDeactivate")); Commands.Add(TEXT("7"));
}
bool FSigilEquipmentLifecycleReentrantTest::RunTest(const FString& Parameters)
{
	const int32 Action = FCString::Atoi(*Parameters);
	FLifecycleFixture F(Action == 5 ? 1 : 3, Action == 5);
	if (Action == 5)
	{
		TestFalse(TEXT("添加通知内重置"), F.Equipment->IsEquipmentSystemInitialized());
		TestNull(TEXT("已移除实例不得重绑来源"), F.Items[0]->GetSourceItem_Implementation());
		TestFalse(TEXT("已移除实例不得再次激活"), F.Items[0]->IsEquipmentActive_Implementation());
		TestEqual(TEXT("旧添加不得派发激活"), F.Items[0]->ActivateCount, 0);
		TestEqual(TEXT("添加回调内只结束一次"), F.Items[0]->EndCount, 1);
		return true;
	}
	F.Items[0]->Callback = Action == 6 ? 4 : Action;
	if (Action == 7)
	{
		F.Items[0]->ItemToEquip = F.Items[1]->GetSourceItem_Implementation();
		F.Items[0]->TargetSlot = LifecycleB;
		F.Equipment->UnequipBySlot(LifecycleB);
		F.Equipment->SetGroupActiveIndex(LifecycleGroup, 1);
		auto* NewTarget = Cast<USigilLifecycleTestInstance>(F.Equipment->GetEquipmentInSlot(LifecycleB));
		if (!TestNotNull(TEXT("停用回调向空目标装入新实例"), NewTarget)) { return false; }
		TestEqual(TEXT("旧切换不得激活后来装入的目标"), NewTarget->ActivateCount, 0);
		F.Equipment->SetGroupActiveIndex(LifecycleGroup, 1);
		TestEqual(TEXT("随后显式选择新目标必须激活"), NewTarget->ActivateCount, 1);
		return true;
	}
	if (Action == 4) { F.Equipment->ResetEquipmentSystem(); }
	else if (Action == 6) { F.Equipment->RemoveAllEquipments(); }
	else { F.Equipment->SetGroupActiveIndex(LifecycleGroup, 1); }
	if (Action == 1 || Action == 4 || Action == 6)
	{
		TestFalse(TEXT("重入重置后系统停止"), F.Equipment->IsEquipmentSystemInitialized());
		for (int32 Idx = 0; Idx < 3; ++Idx)
		{
			TestNull(TEXT("重入重置实际槽清空"), F.Equipment->GetEquipmentInSlot(F.Slots[Idx]));
			TestEqual(TEXT("重入清理只结束一次"), F.Items[Idx]->EndCount, 1);
		}
		TestEqual(TEXT("旧切换不得激活 B"), F.Items[1]->ActivateCount, 0);
	}
	else if (Action == 2)
	{
		TestNull(TEXT("回调删除旧 A"), F.Equipment->GetEquipmentInSlot(LifecycleA));
		TestEqual(TEXT("A 只结束一次"), F.Items[0]->EndCount, 1);
	}
	else
	{
		TestEqual(TEXT("新切换激活 C"), F.Items[2]->ActivateCount, 1);
		TestEqual(TEXT("旧切换不能再激活 B"), F.Items[1]->ActivateCount, 0);
	}
	return true;
}
#endif
