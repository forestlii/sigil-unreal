// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "SigilGasTags.h"
#include "Tests/SigilGasTestTypes.h"
#include "Tests/SigilGasTestWorld.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasAbilitySourceGateTest,
	"SigilGas.Ability.RequireSourceObjectActive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasAbilitySourceGateTest::RunTest(const FString& Parameters)
{
	FSigilGasTestWorld Fixture(TEXT("SigilGasSourceGateWorld"));
	ASigilGasTestAbilityActor* Actor = Fixture.SpawnAbilityActor();
	TestNotNull(TEXT("The test actor should spawn"), Actor);
	if (!Actor)
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Actor->GetAbilitySystem();
	USigilGasTestSourceObject* Source = NewObject<USigilGasTestSourceObject>(Actor);
	UObject* PlainSource = NewObject<USigilGasTestPlainObject>(Actor);

	// Static helper semantics.
	TestFalse(TEXT("Null source is never active"), USigilGameplayAbility::IsAbilitySourceActive(nullptr));
	TestFalse(TEXT("A source that does not implement the interface is never active"), USigilGameplayAbility::IsAbilitySourceActive(PlainSource));
	TestFalse(TEXT("An inactive interface source reports inactive"), USigilGameplayAbility::IsAbilitySourceActive(Source));
	Source->bActive = true;
	TestTrue(TEXT("An active interface source reports active"), USigilGameplayAbility::IsAbilitySourceActive(Source));
	Source->bActive = false;

	// Grant an ability that requires an active source, using the toggleable source object.
	FGameplayAbilitySpec Spec(USigilGasTestAbility::StaticClass(), 1, INDEX_NONE, Source);
	const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
	FGameplayAbilitySpec* GrantedSpec = ASC->FindAbilitySpecFromHandle(Handle);
	TestNotNull(TEXT("The ability spec should be granted"), GrantedSpec);
	if (!GrantedSpec)
	{
		return false;
	}

	USigilGasTestAbility* Instance = Cast<USigilGasTestAbility>(GrantedSpec->GetPrimaryInstance());
	TestNotNull(TEXT("An InstancedPerActor ability must have a primary instance"), Instance);
	if (!Instance)
	{
		return false;
	}

	// Default (flag off): activates regardless of the source state.
	TestTrue(TEXT("With the flag off the ability activates while the source is inactive"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("Activation count after the unguarded activation"), Instance->ActivationCount, 1);

	// Flag on + inactive source: blocked, and the failure tag is reported.
	Instance->SetRequireSourceObjectActive(true);
	FGameplayTagContainer FailTags;
	// CanActivateAbility is public on UGameplayAbility but protected on the Sigil override; call through the base type.
	TestFalse(TEXT("CanActivateAbility must fail while the source is inactive"),
	          static_cast<UGameplayAbility*>(Instance)->CanActivateAbility(Handle, ASC->AbilityActorInfo.Get(), nullptr, nullptr, &FailTags));
	TestTrue(TEXT("The SourceObjectInactive fail tag must be reported"), FailTags.HasTagExact(SigilAbilityActivateFailTags::SourceObjectInactive));
	TestFalse(TEXT("TryActivateAbility must fail while the source is inactive"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("Activation count must not change after the blocked activation"), Instance->ActivationCount, 1);

	// Flag on + active source: allowed.
	Source->bActive = true;
	TestTrue(TEXT("TryActivateAbility succeeds once the source is active"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("Activation count after the gated activation"), Instance->ActivationCount, 2);

	// Flag on + source that does not implement the interface: blocked.
	FGameplayAbilitySpec PlainSpec(USigilGasTestAbility::StaticClass(), 1, INDEX_NONE, PlainSource);
	const FGameplayAbilitySpecHandle PlainHandle = ASC->GiveAbility(PlainSpec);
	FGameplayAbilitySpec* PlainGranted = ASC->FindAbilitySpecFromHandle(PlainHandle);
	USigilGasTestAbility* PlainInstance = PlainGranted ? Cast<USigilGasTestAbility>(PlainGranted->GetPrimaryInstance()) : nullptr;
	TestNotNull(TEXT("The second ability should have a primary instance"), PlainInstance);
	if (PlainInstance)
	{
		PlainInstance->SetRequireSourceObjectActive(true);
		TestFalse(TEXT("A source without the interface must block activation"), ASC->TryActivateAbility(PlainHandle));
		TestEqual(TEXT("The blocked ability must not count an activation"), PlainInstance->ActivationCount, 0);
	}

	return true;
}

#endif
