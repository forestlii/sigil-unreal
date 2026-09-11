// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AsyncTasks/SigilAsyncTask_CooldownChanged.h"
#include "AsyncTasks/SigilAsyncTask_EffectStackChanged.h"
#include "GameplayEffect.h"
#include "NativeGameplayTags.h"
#include "Tests/SigilGasTestTypes.h"
#include "Tests/SigilGasTestWorld.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(SigilGasTestCooldownTag, "Sigil.Test.Cooldown.AsyncTask");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SigilGasTestStackTag, "Sigil.Test.Effect.AsyncStack");

namespace
{
UGameplayEffect* MakeDurationEffect(const TCHAR* Name, float DurationSeconds)
{
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), Name);
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	Effect->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(DurationSeconds));
	return Effect;
}

FActiveGameplayEffectHandle ApplyWithGrantedTag(UAbilitySystemComponent* ASC, UGameplayEffect* Effect, const FGameplayTag& GrantedTag)
{
	FGameplayEffectSpec Spec(Effect, ASC->MakeEffectContext(), 1.f);
	Spec.DynamicGrantedTags.AddTag(GrantedTag);
	return ASC->ApplyGameplayEffectSpecToSelf(Spec);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasAsyncTaskCooldownChangedTest,
	"SigilGas.AsyncTask.CooldownChanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasAsyncTaskCooldownChangedTest::RunTest(const FString& Parameters)
{
	FSigilGasTestWorld Fixture(TEXT("SigilGasCooldownAsyncWorld"));
	ASigilGasTestAbilityActor* Actor = Fixture.SpawnAbilityActor();
	TestNotNull(TEXT("The test actor should spawn"), Actor);
	if (!Actor)
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Actor->GetAbilitySystem();
	USigilGasTestAsyncListener* Listener = NewObject<USigilGasTestAsyncListener>(Actor);

	// Invalid inputs return null instead of a half-initialized task.
	TestNull(TEXT("A null ASC yields no task"), USigilAsyncTask_CooldownChanged::ListenForCooldownChange(nullptr, FGameplayTagContainer(SigilGasTestCooldownTag), false));
	TestNull(TEXT("Empty cooldown tags yield no task"), USigilAsyncTask_CooldownChanged::ListenForCooldownChange(ASC, FGameplayTagContainer(), false));

	USigilAsyncTask_CooldownChanged* Task = USigilAsyncTask_CooldownChanged::ListenForCooldownChange(ASC, FGameplayTagContainer(SigilGasTestCooldownTag), false);
	TestNotNull(TEXT("The cooldown task should be created"), Task);
	if (!Task)
	{
		return false;
	}
	Task->AddToRoot();
	Task->OnCooldownBegin.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleCooldownBegin);
	Task->OnCooldownEnd.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleCooldownEnd);
	Task->Activate();

	// A duration effect granting the cooldown tag begins the cooldown with the effect's duration.
	UGameplayEffect* CooldownEffect = MakeDurationEffect(TEXT("SigilGasTestCooldownEffect"), 5.f);
	const FActiveGameplayEffectHandle Handle = ApplyWithGrantedTag(ASC, CooldownEffect, SigilGasTestCooldownTag);
	TestTrue(TEXT("The cooldown effect should apply"), Handle.IsValid());
	TestEqual(TEXT("OnCooldownBegin fires once"), Listener->CooldownBeginCount, 1);
	TestEqual(TEXT("OnCooldownEnd has not fired yet"), Listener->CooldownEndCount, 0);
	TestTrue(TEXT("The broadcast carries the cooldown tag"), Listener->LastCooldownTag == SigilGasTestCooldownTag);
	TestEqual(TEXT("The broadcast reports the effect duration"), Listener->LastDuration, 5.f, KINDA_SMALL_NUMBER);
	TestTrue(TEXT("The broadcast reports a positive remaining time"), Listener->LastTimeRemaining > 0.f && Listener->LastTimeRemaining <= 5.f);

	// An unrelated effect does not trigger the listener.
	UGameplayEffect* OtherEffect = MakeDurationEffect(TEXT("SigilGasTestOtherEffect"), 5.f);
	ApplyWithGrantedTag(ASC, OtherEffect, SigilGasTestStackTag);
	TestEqual(TEXT("Unrelated effects do not fire OnCooldownBegin"), Listener->CooldownBeginCount, 1);

	// Removing the effect drops the tag count to zero and ends the cooldown.
	ASC->RemoveActiveGameplayEffect(Handle);
	TestEqual(TEXT("OnCooldownEnd fires once after removal"), Listener->CooldownEndCount, 1);

	// After EndAction nothing is broadcast anymore.
	Task->EndAction();
	ApplyWithGrantedTag(ASC, CooldownEffect, SigilGasTestCooldownTag);
	TestEqual(TEXT("OnCooldownBegin stays silent after EndAction"), Listener->CooldownBeginCount, 1);

	Task->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasAsyncTaskEffectStackChangedTest,
	"SigilGas.AsyncTask.EffectStackChanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasAsyncTaskEffectStackChangedTest::RunTest(const FString& Parameters)
{
	FSigilGasTestWorld Fixture(TEXT("SigilGasStackAsyncWorld"));
	ASigilGasTestAbilityActor* Actor = Fixture.SpawnAbilityActor();
	TestNotNull(TEXT("The test actor should spawn"), Actor);
	if (!Actor)
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Actor->GetAbilitySystem();
	USigilGasTestAsyncListener* Listener = NewObject<USigilGasTestAsyncListener>(Actor);

	TestNull(TEXT("A null ASC yields no task"), USigilAsyncTask_EffectStackChanged::ListenForGameplayEffectStackChange(nullptr, SigilGasTestStackTag));
	TestNull(TEXT("An invalid tag yields no task"), USigilAsyncTask_EffectStackChanged::ListenForGameplayEffectStackChange(ASC, FGameplayTag()));

	USigilAsyncTask_EffectStackChanged* Task = USigilAsyncTask_EffectStackChanged::ListenForGameplayEffectStackChange(ASC, SigilGasTestStackTag);
	TestNotNull(TEXT("The stack task should be created"), Task);
	if (!Task)
	{
		return false;
	}
	Task->AddToRoot();
	Task->OnGameplayEffectStackChange.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleStackChanged);
	Task->Activate();

	UGameplayEffect* StackingEffect = MakeDurationEffect(TEXT("SigilGasTestStackingEffect"), 30.f);
	// SetStackingType is WITH_EDITOR-only in 5.8; write the (deprecated-for-privatization) field so the test also builds without editor.
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	StackingEffect->StackingType = EGameplayEffectStackingType::AggregateByTarget;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	StackingEffect->StackLimitCount = 5;

	// First application: (1, 0).
	const FActiveGameplayEffectHandle Handle = ApplyWithGrantedTag(ASC, StackingEffect, SigilGasTestStackTag);
	TestTrue(TEXT("The stacking effect should apply"), Handle.IsValid());
	TestEqual(TEXT("First application broadcasts once"), Listener->StackChangeCount, 1);
	TestEqual(TEXT("First application reports a new count of 1"), Listener->LastNewStackCount, 1);
	TestEqual(TEXT("First application reports an old count of 0"), Listener->LastOldStackCount, 0);
	TestTrue(TEXT("The broadcast carries the effect tag"), Listener->LastStackTag == SigilGasTestStackTag);

	// Second application stacks: (2, 1).
	ApplyWithGrantedTag(ASC, StackingEffect, SigilGasTestStackTag);
	TestEqual(TEXT("Stacking broadcasts again"), Listener->StackChangeCount, 2);
	TestEqual(TEXT("Stacking reports a new count of 2"), Listener->LastNewStackCount, 2);
	TestEqual(TEXT("Stacking reports an old count of 1"), Listener->LastOldStackCount, 1);
	TestEqual(TEXT("The ASC agrees on the stack count"), ASC->GetCurrentStackCount(Handle), 2);

	// Removal: (0, previous).
	ASC->RemoveActiveGameplayEffect(Handle);
	TestEqual(TEXT("Removal broadcasts"), Listener->StackChangeCount, 3);
	TestEqual(TEXT("Removal reports a new count of 0"), Listener->LastNewStackCount, 0);
	TestEqual(TEXT("Removal reports the previous stack count"), Listener->LastOldStackCount, 2);

	// EndAction after the effect is gone must not crash (stack delegate pointer is null) and silences the task.
	Task->EndAction();
	ApplyWithGrantedTag(ASC, StackingEffect, SigilGasTestStackTag);
	TestEqual(TEXT("No broadcast after EndAction"), Listener->StackChangeCount, 3);

	Task->RemoveFromRoot();
	return true;
}

#endif
