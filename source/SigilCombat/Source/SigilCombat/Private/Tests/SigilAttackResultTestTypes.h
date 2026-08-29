// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CombatFlow/SigilAttackResultProcessor.h"
#include "CombatFlow/SigilCombatFlow.h"
#include "SigilCombatSystemComponent.h"
#include "SigilAttackResultTestTypes.generated.h"

UCLASS()
class USigilAttackResultTestCombatSystemComponent final : public USigilCombatSystemComponent
{
	GENERATED_BODY()

public:
	void SetTestCombatFlow(USigilCombatFlow* InCombatFlow)
	{
		CombatFlow = InCombatFlow;
	}

	FSigilAttackResultContainer& GetTestAttackResultContainer()
	{
		return AttackResultContainer;
	}
};

UCLASS()
class USigilAttackResultCountingProcessor final : public USigilAttackResultProcessor
{
	GENERATED_BODY()

public:
	mutable int32 HandleCallCount = 0;
	mutable bool bLastHandledPayloadWasConsumed = true;

protected:
	virtual void HandleIncomingAttackResult_Implementation(const FSigilAttackResult& AttackResult) const override
	{
		++HandleCallCount;
		bLastHandledPayloadWasConsumed = AttackResult.bConsumed;
	}
};

UCLASS()
class USigilAttackResultReentrantProcessor final : public USigilAttackResultProcessor
{
	GENERATED_BODY()

public:
	void SetCombatSystemForTest(USigilAttackResultTestCombatSystemComponent* InCombatSystem)
	{
		CombatSystem = InCombatSystem;
	}

	mutable int32 HandleCallCount = 0;

protected:
	virtual void HandleIncomingAttackResult_Implementation(const FSigilAttackResult& AttackResult) const override
	{
		++HandleCallCount;
		if (CombatSystem)
		{
			CombatSystem->GetTestAttackResultContainer().ConsumePendingEntries();
		}
	}

private:
	UPROPERTY()
	TObjectPtr<USigilAttackResultTestCombatSystemComponent> CombatSystem;
};

UCLASS()
class USigilAttackResultTestCombatFlow final : public USigilCombatFlow
{
	GENERATED_BODY()

public:
	void AddTestProcessor(USigilAttackResultProcessor* Processor)
	{
		AttackResultProcessors.Add(Processor);
	}
};
