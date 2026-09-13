// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/SigilGasTestTypes.h"
#include "Tests/SigilGasTestWorld.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasPassiveAvatarRetryTest,
	"SigilGas.Ability.PassiveActivatesWhenAvatarArrives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasPassiveAvatarRetryTest::RunTest(const FString& Parameters)
{
	FSigilGasTestWorld Fixture(TEXT("SigilGasPassiveAvatarWorld"));
	if (!Fixture.World)
	{
		AddError(TEXT("The test world could not be created"));
		return false;
	}

	// Owner carries the ASC; the avatar arrives later (PlayerState-style setup).
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASigilGasTestAbilityActor* Owner = Fixture.World->SpawnActor<ASigilGasTestAbilityActor>(ASigilGasTestAbilityActor::StaticClass(), FTransform::Identity, SpawnParameters);
	AActor* Avatar = Fixture.World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParameters);
	TestNotNull(TEXT("The owner should spawn"), Owner);
	TestNotNull(TEXT("The avatar should spawn"), Avatar);
	if (!Owner || !Avatar)
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Owner->GetAbilitySystem();
	ASC->InitAbilityActorInfo(Owner, Owner);

	// Grant the passive while the owner is still its own avatar: it activates right away on grant.
	const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(FGameplayAbilitySpec(USigilGasTestPassiveAbility::StaticClass(), 1));
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
	USigilGasTestAbility* Instance = Spec ? Cast<USigilGasTestAbility>(Spec->GetPrimaryInstance()) : nullptr;
	TestNotNull(TEXT("The passive should have an instance"), Instance);
	if (!Instance)
	{
		return false;
	}
	TestEqual(TEXT("Granting with a valid avatar activates the passive once"), Instance->ActivationCount, 1);
	TestTrue(TEXT("The passive stays active"), Spec->IsActive());

	// Cancel it, then swap in the real avatar: OnAvatarSet must bring it back, exactly once.
	ASC->CancelAbilityHandle(Handle);
	TestFalse(TEXT("The passive is inactive after cancellation"), Spec->IsActive());

	ASC->InitAbilityActorInfo(Owner, Avatar);
	Spec = ASC->FindAbilitySpecFromHandle(Handle);
	TestNotNull(TEXT("The spec survives the avatar change"), Spec);
	if (!Spec)
	{
		return false;
	}
	TestTrue(TEXT("The passive is active again after the avatar arrived"), Spec->IsActive());
	TestEqual(TEXT("The avatar change activates the passive exactly once (no double activation from the GA and ASC retries)"), Instance->ActivationCount, 2);

	// Re-initializing with the same avatar is not an avatar change and must not activate again.
	ASC->InitAbilityActorInfo(Owner, Avatar);
	TestEqual(TEXT("Re-initializing with the same avatar does not re-activate"), Instance->ActivationCount, 2);

	return true;
}

#endif
