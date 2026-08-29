// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Tests/SigilAbilityCostTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilAbilityCostNullRelevantTagsTest,
	"SigilGas.AbilityCost.NullRelevantTagsDoesNotCrash",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilAbilityCostNullRelevantTagsTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	UAbilitySystemComponent* AbilitySystemComponent = NewObject<UAbilitySystemComponent>(Owner);
	USigilAbilityCostNullTagsTestCost* Cost = NewObject<USigilAbilityCostNullTagsTestCost>(GetTransientPackage());
	FGameplayAbilityActorInfo ActorInfo;
	ActorInfo.InitFromActor(Owner, Owner, AbilitySystemComponent);

	if (!TestNotNull(TEXT("Ability cost owner should exist"), Owner)
		|| !TestNotNull(TEXT("Ability system component should exist"), AbilitySystemComponent)
		|| !TestNotNull(TEXT("Concrete ability cost should exist"), Cost))
	{
		return false;
	}

	TestFalse(
		TEXT("An unimplemented Blueprint cost with null relevant tags should return its default result without crashing"),
		Cost->CheckCost(nullptr, FGameplayAbilitySpecHandle(), &ActorInfo, nullptr));
	return true;
}

#endif
