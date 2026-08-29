// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameFramework/Actor.h"
#include "Tests/SigilAttackResultTestTypes.h"

namespace
{
bool PrepareReadyCombatFlow(
	FAutomationTestBase& Test,
	AActor*& OutOwner,
	USigilAttackResultTestCombatSystemComponent*& OutCombatSystem,
	USigilAttackResultTestCombatFlow*& OutFlow,
	USigilAttackResultCountingProcessor*& OutProcessor)
{
	OutOwner = NewObject<AActor>(GetTransientPackage(), TEXT("SigilAttackResultTestOwner"));
	OutCombatSystem = NewObject<USigilAttackResultTestCombatSystemComponent>(OutOwner);
	OutFlow = NewObject<USigilAttackResultTestCombatFlow>(OutOwner);
	OutProcessor = NewObject<USigilAttackResultCountingProcessor>(OutFlow);

	if (!Test.TestNotNull(TEXT("Combat owner should exist"), OutOwner)
		|| !Test.TestNotNull(TEXT("Combat system should exist"), OutCombatSystem)
		|| !Test.TestNotNull(TEXT("Combat flow should exist"), OutFlow)
		|| !Test.TestNotNull(TEXT("Attack result processor should exist"), OutProcessor))
	{
		return false;
	}

	OutOwner->AddInstanceComponent(OutCombatSystem);
	OutFlow->AddTestProcessor(OutProcessor);
	OutCombatSystem->SetTestCombatFlow(OutFlow);
	OutFlow->Initialize(OutOwner);
	return Test.TestTrue(TEXT("Combat flow should be ready after initialization"), OutCombatSystem->IsCombatFlowReady());
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilAttackResultImmediateDispatchTest,
	"SigilCombat.AttackResult.ImmediateDispatchUsesUnconsumedCopy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilAttackResultPendingEntryTest,
	"SigilCombat.AttackResult.PendingEntryConsumesOnceAfterFlowReady",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilAttackResultReentrantDispatchTest,
	"SigilCombat.AttackResult.ReentrantConsumeSeesPersistentEntryConsumed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilAttackResultImmediateDispatchTest::RunTest(const FString& Parameters)
{
	AActor* Owner = nullptr;
	USigilAttackResultTestCombatSystemComponent* CombatSystem = nullptr;
	USigilAttackResultTestCombatFlow* Flow = nullptr;
	USigilAttackResultCountingProcessor* Processor = nullptr;
	if (!PrepareReadyCombatFlow(*this, Owner, CombatSystem, Flow, Processor))
	{
		return false;
	}

	FSigilAttackResult Result;
	CombatSystem->GetTestAttackResultContainer().AddEntry(Result);

	TestEqual(TEXT("Ready flow should dispatch the result once"), Processor->HandleCallCount, 1);
	TestFalse(TEXT("Ready flow should receive an unconsumed dispatch copy"), Processor->bLastHandledPayloadWasConsumed);

	CombatSystem->GetTestAttackResultContainer().ConsumePendingEntries();
	TestEqual(TEXT("The persistently consumed entry should not dispatch a second time"), Processor->HandleCallCount, 1);
	return true;
}

bool FSigilAttackResultPendingEntryTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage(), TEXT("SigilAttackResultPendingOwner"));
	USigilAttackResultTestCombatSystemComponent* CombatSystem =
		NewObject<USigilAttackResultTestCombatSystemComponent>(Owner);
	if (!TestNotNull(TEXT("Pending-entry owner should exist"), Owner)
		|| !TestNotNull(TEXT("Pending-entry combat system should exist"), CombatSystem))
	{
		return false;
	}

	Owner->AddInstanceComponent(CombatSystem);
	AddExpectedError(
		TEXT("Combat Flow on SigilAttackResultPendingOwner's combat system component is missing or not initialized; attack result kept pending."),
		EAutomationExpectedErrorFlags::Contains,
		1);
	FSigilAttackResult PendingResult;
	CombatSystem->GetTestAttackResultContainer().AddEntry(PendingResult);

	USigilAttackResultTestCombatFlow* Flow = NewObject<USigilAttackResultTestCombatFlow>(Owner);
	USigilAttackResultCountingProcessor* Processor = NewObject<USigilAttackResultCountingProcessor>(Flow);
	if (!TestNotNull(TEXT("Pending-entry combat flow should exist"), Flow)
		|| !TestNotNull(TEXT("Pending-entry processor should exist"), Processor))
	{
		return false;
	}

	Flow->AddTestProcessor(Processor);
	CombatSystem->SetTestCombatFlow(Flow);
	Flow->Initialize(Owner);
	if (!TestTrue(TEXT("Combat flow should become ready after pending entry arrival"), CombatSystem->IsCombatFlowReady()))
	{
		return false;
	}

	CombatSystem->GetTestAttackResultContainer().ConsumePendingEntries();
	TestEqual(TEXT("Pending result should dispatch once after the flow becomes ready"), Processor->HandleCallCount, 1);
	TestFalse(TEXT("Pending result should also reach the processor as unconsumed"), Processor->bLastHandledPayloadWasConsumed);

	CombatSystem->GetTestAttackResultContainer().ConsumePendingEntries();
	TestEqual(TEXT("A consumed pending result should not dispatch again"), Processor->HandleCallCount, 1);
	return true;
}

bool FSigilAttackResultReentrantDispatchTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage(), TEXT("SigilAttackResultReentrantOwner"));
	USigilAttackResultTestCombatSystemComponent* CombatSystem =
		NewObject<USigilAttackResultTestCombatSystemComponent>(Owner);
	USigilAttackResultTestCombatFlow* Flow = NewObject<USigilAttackResultTestCombatFlow>(Owner);
	USigilAttackResultReentrantProcessor* Processor = NewObject<USigilAttackResultReentrantProcessor>(Flow);
	if (!TestNotNull(TEXT("Reentrant owner should exist"), Owner)
		|| !TestNotNull(TEXT("Reentrant combat system should exist"), CombatSystem)
		|| !TestNotNull(TEXT("Reentrant combat flow should exist"), Flow)
		|| !TestNotNull(TEXT("Reentrant processor should exist"), Processor))
	{
		return false;
	}

	Owner->AddInstanceComponent(CombatSystem);
	Processor->SetCombatSystemForTest(CombatSystem);
	Flow->AddTestProcessor(Processor);
	CombatSystem->SetTestCombatFlow(Flow);
	Flow->Initialize(Owner);
	if (!TestTrue(TEXT("Reentrant combat flow should be ready after initialization"), CombatSystem->IsCombatFlowReady()))
	{
		return false;
	}

	FSigilAttackResult Result;
	CombatSystem->GetTestAttackResultContainer().AddEntry(Result);
	TestEqual(
		TEXT("A processor that consumes pending entries reentrantly should still handle the result once"),
		Processor->HandleCallCount,
		1);

	CombatSystem->GetTestAttackResultContainer().ConsumePendingEntries();
	TestEqual(TEXT("The same stored entry should remain consumed after the callback"), Processor->HandleCallCount, 1);
	return true;
}

#endif
