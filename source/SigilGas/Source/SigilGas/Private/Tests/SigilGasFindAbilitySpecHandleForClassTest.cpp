// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/SigilGasTestTypes.h"
#include "Tests/SigilGasTestWorld.h"
#include "Utilities/SigilAbilitySystemFunctionLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasFindAbilitySpecHandleForClassTest,
	"SigilGas.Library.FindAbilitySpecHandleForClass",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasFindAbilitySpecHandleForClassTest::RunTest(const FString& Parameters)
{
	FSigilGasTestWorld Fixture(TEXT("SigilGasFindHandleWorld"));
	ASigilGasTestAbilityActor* Actor = Fixture.SpawnAbilityActor();
	TestNotNull(TEXT("The test actor should spawn"), Actor);
	if (!Actor)
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Actor->GetAbilitySystem();
	USigilGasTestSourceObject* SourceA = NewObject<USigilGasTestSourceObject>(Actor);
	USigilGasTestSourceObject* SourceB = NewObject<USigilGasTestSourceObject>(Actor);
	USigilGasTestSourceObject* SourceUnused = NewObject<USigilGasTestSourceObject>(Actor);

	// Nothing granted yet: invalid handle, and null inputs are tolerated.
	TestFalse(TEXT("No granted ability yields an invalid handle"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilGasTestAbility::StaticClass()).IsValid());
	TestFalse(TEXT("A null ability system yields an invalid handle"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(nullptr, USigilGasTestAbility::StaticClass()).IsValid());
	TestFalse(TEXT("A null class yields an invalid handle"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, nullptr).IsValid());

	const FGameplayAbilitySpecHandle HandleA = ASC->GiveAbility(FGameplayAbilitySpec(USigilGasTestAbility::StaticClass(), 1, INDEX_NONE, SourceA));
	const FGameplayAbilitySpecHandle HandleB = ASC->GiveAbility(FGameplayAbilitySpec(USigilGasTestAbility::StaticClass(), 1, INDEX_NONE, SourceB));
	TestTrue(TEXT("Both specs should be granted"), HandleA.IsValid() && HandleB.IsValid() && HandleA != HandleB);

	// Class only: the first granted spec of that class.
	TestTrue(TEXT("Class-only lookup returns the first granted spec"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilGasTestAbility::StaticClass()) == HandleA);

	// Class + source object filter.
	TestTrue(TEXT("Filtering by SourceA returns HandleA"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilGasTestAbility::StaticClass(), SourceA) == HandleA);
	TestTrue(TEXT("Filtering by SourceB returns HandleB"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilGasTestAbility::StaticClass(), SourceB) == HandleB);
	TestFalse(TEXT("Filtering by a source that granted nothing yields an invalid handle"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilGasTestAbility::StaticClass(), SourceUnused).IsValid());

	// Exact class match: the parent class does not match the granted subclass.
	TestFalse(TEXT("Lookup by the parent class must not match a subclass spec"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilGameplayAbility::StaticClass()).IsValid());

	// Removal is reflected.
	ASC->ClearAbility(HandleA);
	TestTrue(TEXT("After removing HandleA the class-only lookup returns HandleB"),
	          USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(ASC, USigilGasTestAbility::StaticClass()) == HandleB);

	return true;
}

#endif
