// Copyright (c) 2026 Likeon. All Rights Reserved.
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/SigilInventoryRemoveByDefinitionTestTypes.h"
#include "Tests/SigilEquipmentLifecycleTestTypes.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Items/SigilItemDefinition.h"
#include "Items/SigilItemInstance.h"
#include "Misc/ScopeExit.h"
#include "SigilInventoryTags.h"
#include "SigilItemCollection.h"
#include "SigilItemMultiStackCollection.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FSigilInventoryRemoveByDefinitionTest,
	"SigilInventory.RemoveByDefinition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

void FSigilInventoryRemoveByDefinitionTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : { TEXT("SingleStack"), TEXT("ExactAmount"), TEXT("Insufficient"),
		TEXT("Zero"), TEXT("Negative"), TEXT("NullDefinition"), TEXT("MissingDefinition"),
		TEXT("MultipleStacks"), TEXT("UniqueStacks"), TEXT("NoProgress"),
		TEXT("PartialThenBlocked"), TEXT("RestrictedPartialAmounts") })
	{
		Names.Add(Name);
		Commands.Add(Name);
	}
}

bool FSigilInventoryRemoveByDefinitionTest::RunTest(const FString& Parameters)
{
	if (!TestNotNull(TEXT("引擎存在"), GEngine)) { return false; }
	auto* GI = NewObject<UGameInstance>(GEngine);
	GI->AddToRoot();
	GI->InitializeStandalone(MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("InventoryRemoval")), GetTransientPackage());
	UWorld* World = GI->GetWorld();
	ON_SCOPE_EXIT
	{
		GI->Shutdown();
		if (World)
		{
			World->EndPlay(EEndPlayReason::Quit);
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
			World->RemoveFromRoot();
		}
		GI->RemoveFromRoot();
	};
	if (!TestNotNull(TEXT("真实世界"), World)) { return false; }
	World->AddToRoot();
	World->InitializeActorsForPlay(FURL());
	World->GetWorldSettings()->NotifyBeginPlay();
	auto* Pawn = World->SpawnActor<ASigilLifecycleTestPawn>();
	if (!TestNotNull(TEXT("真实Pawn"), Pawn)) { return false; }
	auto* Inventory = NewObject<USigilInventorySystemComponent>(Pawn);
	Pawn->AddInstanceComponent(Inventory);
	Inventory->RegisterComponent();
	const bool bMulti = Parameters == TEXT("MultipleStacks");
	const bool bBlockedAfterSuccess = Parameters == TEXT("PartialThenBlocked");
	const bool bUnique = bBlockedAfterSuccess || Parameters == TEXT("UniqueStacks");
	USigilItemCollectionDefinition* Layout = nullptr;
	if (bMulti)
	{
		auto* Multi = NewObject<USigilItemMultiStackCollectionDefinition>();
		Multi->DefaultStackSizeLimit = 10;
		Layout = Multi;
	}
	else { Layout = NewObject<USigilItemCollectionDefinition>(); }
	Layout->CollectionTag = SigilCollectionTags::Main;
	auto* Restriction = NewObject<USigilRemovalTestRestriction>(Layout);
	Layout->Restrictions.Add(Restriction);
	auto* Collection = Inventory->AddCollectionByDefinition(Layout);
	if (!TestNotNull(TEXT("真实集合"), Collection)) { return false; }
	auto* Definition = NewObject<USigilItemDefinition>();
	Definition->bUnique = bUnique;
	auto* Other = NewObject<USigilItemDefinition>();
	Other->bUnique = false;
	const int32 InitialAmount = bUnique ? 3 : 30;
	// 多叠每次加入一整叠，随后直接核对库存总量；不依赖批量添加的返回聚合语义。
	for (int32 Index = 0; Index < (bMulti ? 3 : 1); ++Index)
	{
		const int32 AddAmount = bMulti ? 10 : InitialAmount;
		if (!TestEqual(TEXT("请求物品实际加入"), Inventory->AddItemByDefinition(SigilCollectionTags::Main, Definition, AddAmount).Amount, AddAmount)) { return false; }
	}
	if (!TestEqual(TEXT("删除前实际总量"), Inventory->GetItemAmountByDefinition(Definition, false), InitialAmount)) { return false; }
	if (!TestEqual(TEXT("真实叠数"), Collection->GetItemStacksNum(), (bMulti || bUnique) ? 3 : 1)) { return false; }
	if (!TestEqual(TEXT("其他定义加入7"), Inventory->AddItemByDefinition(SigilCollectionTags::Main, Other, 7).Amount, 7)) { return false; }
	FSigilItemInfo FirstItem;
	if (!TestTrue(TEXT("可查询已加入物品"), Inventory->GetItemInfoByDefinition(Definition, FirstItem))) { return false; }

	int32 Request = 5;
	int32 ExpectedRemoved = 5;
	int32 ExpectedLeft = 25;
	int32 ExpectedCalls = 1;
	TSoftObjectPtr<USigilItemDefinition> RequestedDefinition = Definition;
	if (Parameters == TEXT("ExactAmount")) { Request = 30; ExpectedRemoved = 30; ExpectedLeft = 0; }
	if (Parameters == TEXT("Insufficient")) { Request = 35; ExpectedRemoved = 30; ExpectedLeft = 0; }
	if (Parameters == TEXT("Zero") || Parameters == TEXT("Negative"))
	{
		Request = Parameters == TEXT("Zero") ? 0 : -5;
		ExpectedRemoved = 0; ExpectedLeft = 30; ExpectedCalls = 0;
	}
	if (Parameters == TEXT("NullDefinition") || Parameters == TEXT("MissingDefinition"))
	{
		RequestedDefinition = Parameters == TEXT("NullDefinition") ? nullptr : NewObject<USigilItemDefinition>();
		ExpectedRemoved = 0; ExpectedLeft = 30; ExpectedCalls = 0;
	}
	if (bMulti) { Request = 15; ExpectedRemoved = 15; ExpectedLeft = 15; ExpectedCalls = 2; }
	if (Parameters == TEXT("UniqueStacks")) { Request = 2; ExpectedRemoved = 2; ExpectedLeft = 1; ExpectedCalls = 2; }
	if (Parameters == TEXT("NoProgress"))
	{
		Restriction->SuccessfulCalls = 0;
		ExpectedRemoved = 0; ExpectedLeft = 30;
	}
	if (bBlockedAfterSuccess)
	{
		Restriction->SuccessfulCalls = 1;
		ExpectedRemoved = 1; ExpectedLeft = 2; ExpectedCalls = 2;
	}
	if (Parameters == TEXT("RestrictedPartialAmounts"))
	{
		Restriction->PerCallLimit = 2;
		ExpectedCalls = 3;
	}

	const FSigilItemInfo Removed = Inventory->RemoveItemByDefinition(RequestedDefinition, Request);
	TestEqual(TEXT("实际返回数量"), Removed.Amount, ExpectedRemoved);
	TestEqual(TEXT("实际库存剩量"), Inventory->GetItemAmountByDefinition(Definition, false), ExpectedLeft);
	TestEqual(TEXT("其他物品不受影响"), Inventory->GetItemAmountByDefinition(Other, false), 7);
	TestEqual(TEXT("达成请求或首次零进度后不再删除"), Restriction->RemoveCalls, ExpectedCalls);
	if (ExpectedRemoved > 0)
	{
		TestNotNull(TEXT("成功返回有效物品来源"), Removed.Item.Get());
		TestTrue(TEXT("成功返回原集合"), Removed.ItemCollection == Collection);
	}
	if (bBlockedAfterSuccess)
	{
		TestTrue(TEXT("后续拒绝不覆盖最后成功物品"), Removed.Item == FirstItem.Item);
	}
	return true;
}
#endif
