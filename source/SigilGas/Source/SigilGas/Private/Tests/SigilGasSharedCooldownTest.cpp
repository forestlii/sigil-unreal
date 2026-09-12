// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "SigilGasTags.h"
#include "Tests/SigilGasTestTypes.h"
#include "Tests/SigilGasTestWorld.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasSharedCooldownTest,
	"SigilGas.Ability.SharedCooldown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasSharedCooldownTest::RunTest(const FString& Parameters)
{
	FSigilGasTestWorld Fixture(TEXT("SigilGasSharedCooldownWorld"));
	ASigilGasTestAbilityActor* Actor = Fixture.SpawnAbilityActor();
	TestNotNull(TEXT("The test actor should spawn"), Actor);
	if (!Actor)
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Actor->GetAbilitySystem();
	const FGameplayTagContainer SharedTagContainer(SigilGasTestTags::SharedCooldown);

	// --- Shared cooldown GE + SetByCaller duration + ability-owned cooldown tag.
	const FGameplayAbilitySpecHandle SharedHandle = ASC->GiveAbility(FGameplayAbilitySpec(USigilGasTestSharedCooldownAbility::StaticClass(), 1));
	FGameplayAbilitySpec* SharedSpec = ASC->FindAbilitySpecFromHandle(SharedHandle);
	USigilGasTestAbility* SharedInstance = SharedSpec ? Cast<USigilGasTestAbility>(SharedSpec->GetPrimaryInstance()) : nullptr;
	TestNotNull(TEXT("The shared cooldown ability should have an instance"), SharedInstance);
	if (!SharedInstance)
	{
		return false;
	}

	// GetCooldownTags() is the union of the GE's granted tags and CooldownTags, minus the shared marker.
	TestTrue(TEXT("The shared cooldown effect grants the marker (required by the engine's IsDataValid)"),
	         GetDefault<USigilGasTestSharedCooldownEffect>()->GetGrantedTags().HasTagExact(SigilCooldownTags::SharedMarker));
	const FGameplayTagContainer* ReportedCooldownTags = static_cast<UGameplayAbility*>(SharedInstance)->GetCooldownTags();
	TestNotNull(TEXT("GetCooldownTags must return a container when CooldownTags is set"), ReportedCooldownTags);
	TestTrue(TEXT("GetCooldownTags must include the ability's own cooldown tag"), ReportedCooldownTags && ReportedCooldownTags->HasTagExact(SigilGasTestTags::SharedCooldown));
	TestFalse(TEXT("GetCooldownTags must exclude the shared marker"), ReportedCooldownTags && ReportedCooldownTags->HasTagExact(SigilCooldownTags::SharedMarker));

#if WITH_EDITOR
	{
		FDataValidationContext ValidationContext;
		const EDataValidationResult ValidationResult = GetDefault<USigilGasTestSharedCooldownAbility>()->IsDataValid(ValidationContext);
		TestTrue(TEXT("A shared-cooldown ability passes Data Validation once the effect grants the marker"), ValidationResult != EDataValidationResult::Invalid);
	}
#endif

	TestTrue(TEXT("The first activation succeeds"), ASC->TryActivateAbility(SharedHandle));
	TestEqual(TEXT("Activation count after the first activation"), SharedInstance->ActivationCount, 1);
	TestTrue(TEXT("The owner now carries the ability's cooldown tag"), ASC->HasMatchingGameplayTag(SigilGasTestTags::SharedCooldown));

	float TimeRemaining = 0.f;
	float Duration = 0.f;
	TestTrue(TEXT("A cooldown effect matching the tag is active"), ASC->GetCooldownRemainingForTags(SharedTagContainer, TimeRemaining, Duration));
	TestEqual(TEXT("The cooldown duration comes from the SetByCaller CooldownDuration"), Duration, 3.f, KINDA_SMALL_NUMBER);
	TestTrue(TEXT("The remaining time is within the duration"), TimeRemaining > 0.f && TimeRemaining <= 3.f);

	TestFalse(TEXT("The second activation is blocked by the cooldown"), ASC->TryActivateAbility(SharedHandle));
	TestEqual(TEXT("The blocked activation does not count"), SharedInstance->ActivationCount, 1);
	TestTrue(TEXT("The owner carries the shared marker while any shared cooldown is active"), ASC->HasMatchingGameplayTag(SigilCooldownTags::SharedMarker));

	// A second ability sharing the same effect with its own tag is NOT blocked by the first one's cooldown.
	const FGameplayAbilitySpecHandle SharedHandleB = ASC->GiveAbility(FGameplayAbilitySpec(USigilGasTestSharedCooldownAbilityB::StaticClass(), 1));
	FGameplayAbilitySpec* SharedSpecB = ASC->FindAbilitySpecFromHandle(SharedHandleB);
	USigilGasTestAbility* SharedInstanceB = SharedSpecB ? Cast<USigilGasTestAbility>(SharedSpecB->GetPrimaryInstance()) : nullptr;
	TestNotNull(TEXT("The second shared cooldown ability should have an instance"), SharedInstanceB);
	if (SharedInstanceB)
	{
		TestTrue(TEXT("Ability B activates while ability A is on cooldown (the marker does not cross-block)"), ASC->TryActivateAbility(SharedHandleB));
		TestEqual(TEXT("Ability B counted its activation"), SharedInstanceB->ActivationCount, 1);
		TestTrue(TEXT("Ability B's own cooldown tag is granted"), ASC->HasMatchingGameplayTag(SigilGasTestTags::SharedCooldownB));
		TestFalse(TEXT("Ability B is now on its own cooldown"), ASC->TryActivateAbility(SharedHandleB));
		float TimeRemainingB = 0.f;
		float DurationB = 0.f;
		TestTrue(TEXT("Ability B's cooldown effect is active"), ASC->GetCooldownRemainingForTags(FGameplayTagContainer(SigilGasTestTags::SharedCooldownB), TimeRemainingB, DurationB));
		TestEqual(TEXT("Ability B's cooldown uses its own SetByCaller duration"), DurationB, 4.f, KINDA_SMALL_NUMBER);
		ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(SigilGasTestTags::SharedCooldownB));
	}

	ASC->RemoveActiveEffectsWithGrantedTags(SharedTagContainer);
	TestFalse(TEXT("The cooldown tag is gone after removing the effect"), ASC->HasMatchingGameplayTag(SigilGasTestTags::SharedCooldown));
	TestTrue(TEXT("The ability activates again once the cooldown effect is removed"), ASC->TryActivateAbility(SharedHandle));
	TestEqual(TEXT("Activation count after the cooldown was cleared"), SharedInstance->ActivationCount, 2);

	// --- Engine path: no CooldownTags / CooldownDuration configured, fixed-duration GE applied through Super.
	ASC->RemoveActiveEffectsWithGrantedTags(SharedTagContainer);
	const int32 EffectsBefore = ASC->GetActiveGameplayEffects().GetNumGameplayEffects();

	const FGameplayAbilitySpecHandle FixedHandle = ASC->GiveAbility(FGameplayAbilitySpec(USigilGasTestFixedCooldownAbility::StaticClass(), 1));
	FGameplayAbilitySpec* FixedSpec = ASC->FindAbilitySpecFromHandle(FixedHandle);
	USigilGasTestAbility* FixedInstance = FixedSpec ? Cast<USigilGasTestAbility>(FixedSpec->GetPrimaryInstance()) : nullptr;
	TestNotNull(TEXT("The fixed cooldown ability should have an instance"), FixedInstance);
	if (!FixedInstance)
	{
		return false;
	}

	TestTrue(TEXT("The fixed cooldown ability activates"), ASC->TryActivateAbility(FixedHandle));
	TestEqual(TEXT("The engine path still applies exactly one cooldown effect"), ASC->GetActiveGameplayEffects().GetNumGameplayEffects(), EffectsBefore + 1);
	TestFalse(TEXT("The engine path never grants the shared cooldown tag"), ASC->HasMatchingGameplayTag(SigilGasTestTags::SharedCooldown));
	TestTrue(TEXT("The engine path grants the fixed effect's own tag"), ASC->HasMatchingGameplayTag(SigilGasTestTags::FixedCooldown));
	TestFalse(TEXT("The engine path blocks the second activation while the fixed cooldown runs"), ASC->TryActivateAbility(FixedHandle));
	TestEqual(TEXT("The blocked engine-path activation does not count"), FixedInstance->ActivationCount, 1);

	return true;
}

#endif
