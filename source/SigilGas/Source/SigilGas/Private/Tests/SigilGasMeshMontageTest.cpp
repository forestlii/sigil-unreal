// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilityTasks/SigilAbilityTask_PlayMontageAndWaitForEvent.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Tests/SigilGasTestTypes.h"
#include "Tests/SigilGasTestWorld.h"
#include "TimerManager.h"

namespace
{
struct FSigilGasMontageFixture
{
	FSigilGasTestWorld World;
	ASigilGasTestAbilityActor* Actor = nullptr;
	USkeletalMeshComponent* MainMesh = nullptr;
	USkeletalMeshComponent* SecondaryMesh = nullptr;
	AActor* Weapon = nullptr;
	USkeletalMeshComponent* WeaponMesh = nullptr;
	AActor* Stranger = nullptr;
	USkeletalMeshComponent* StrangerMesh = nullptr;
	USigilGasTestAbility* Ability = nullptr;
	FGameplayAbilitySpecHandle Handle;

	explicit FSigilGasMontageFixture(const TCHAR* Name)
		: World(Name)
	{
		if (!World.World)
		{
			return;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Actor = World.World->SpawnActor<ASigilGasTestAbilityActor>(ASigilGasTestAbilityActor::StaticClass(), FTransform::Identity, Params);
		if (!Actor)
		{
			return;
		}

		// Register the main mesh before InitAbilityActorInfo so the actor info picks it up as SkeletalMeshComponent.
		MainMesh = NewObject<USkeletalMeshComponent>(Actor, TEXT("MainMesh"));
		MainMesh->RegisterComponent();
		SecondaryMesh = NewObject<USkeletalMeshComponent>(Actor, TEXT("SecondaryMesh"));
		SecondaryMesh->RegisterComponent();

		Params.Owner = Actor;
		Weapon = World.World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
		WeaponMesh = NewObject<USkeletalMeshComponent>(Weapon, TEXT("WeaponMesh"));
		WeaponMesh->RegisterComponent();

		Params.Owner = nullptr;
		Stranger = World.World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
		StrangerMesh = NewObject<USkeletalMeshComponent>(Stranger, TEXT("StrangerMesh"));
		StrangerMesh->RegisterComponent();

		Actor->GetAbilitySystem()->InitAbilityActorInfo(Actor, Actor);

		Handle = Actor->GetAbilitySystem()->GiveAbility(FGameplayAbilitySpec(USigilGasTestAbility::StaticClass(), 1));
		if (FGameplayAbilitySpec* Spec = Actor->GetAbilitySystem()->FindAbilitySpecFromHandle(Handle))
		{
			Ability = Cast<USigilGasTestAbility>(Spec->GetPrimaryInstance());
		}
	}

	bool IsValid() const
	{
		return World.World && Actor && MainMesh && SecondaryMesh && Weapon && WeaponMesh && Stranger && StrangerMesh && Ability;
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasMontageAbilityBookkeepingTest,
	"SigilGas.Montage.AbilityTracksMontagePerMesh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasMontageAbilityBookkeepingTest::RunTest(const FString& Parameters)
{
	FSigilGasMontageFixture Fixture(TEXT("SigilGasMontageAbilityWorld"));
	TestTrue(TEXT("The montage fixture should be valid"), Fixture.IsValid());
	if (!Fixture.IsValid())
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Fixture.Actor->GetAbilitySystem();
	TestTrue(TEXT("The actor info knows the registered main mesh"), ASC->IsAvatarMainMesh(Fixture.MainMesh));
	TestFalse(TEXT("The secondary mesh is not the main mesh"), ASC->IsAvatarMainMesh(Fixture.SecondaryMesh));
	TestFalse(TEXT("Null is not the main mesh"), ASC->IsAvatarMainMesh(nullptr));

	UAnimMontage* MontageA = NewObject<UAnimMontage>(GetTransientPackage(), TEXT("SigilGasTestMontageA"));
	UAnimMontage* MontageB = NewObject<UAnimMontage>(GetTransientPackage(), TEXT("SigilGasTestMontageB"));
	UAnimMontage* MontageC = NewObject<UAnimMontage>(GetTransientPackage(), TEXT("SigilGasTestMontageC"));
	USigilGasTestAbility* Ability = Fixture.Ability;

	TestNull(TEXT("Nothing is tracked initially"), Ability->GetCurrentMontageForMesh(Fixture.SecondaryMesh));
	TestNull(TEXT("A null mesh yields null"), Ability->GetCurrentMontageForMesh(nullptr));

	// Secondary mesh bookkeeping, including the second Set on the same mesh that GASShooter lost.
	Ability->SetCurrentMontageForMesh(Fixture.SecondaryMesh, MontageA);
	TestEqual(TEXT("First Set is tracked"), Ability->GetCurrentMontageForMesh(Fixture.SecondaryMesh), MontageA);
	Ability->SetCurrentMontageForMesh(Fixture.SecondaryMesh, MontageB);
	TestEqual(TEXT("Second Set on the same mesh replaces the montage (GASShooter by-value bug fixed)"), Ability->GetCurrentMontageForMesh(Fixture.SecondaryMesh), MontageB);

	Ability->SetCurrentMontageForMesh(Fixture.WeaponMesh, MontageC);
	TestEqual(TEXT("Another mesh is tracked independently"), Ability->GetCurrentMontageForMesh(Fixture.WeaponMesh), MontageC);
	TestEqual(TEXT("The first mesh keeps its montage"), Ability->GetCurrentMontageForMesh(Fixture.SecondaryMesh), MontageB);

	Ability->SetCurrentMontageForMesh(Fixture.SecondaryMesh, nullptr);
	TestNull(TEXT("Clearing a mesh removes its montage"), Ability->GetCurrentMontageForMesh(Fixture.SecondaryMesh));
	TestEqual(TEXT("Clearing one mesh does not touch another"), Ability->GetCurrentMontageForMesh(Fixture.WeaponMesh), MontageC);

	// Main mesh maps onto the engine's CurrentMontage in both directions.
	Ability->SetCurrentMontageForMesh(Fixture.MainMesh, MontageA);
	TestEqual(TEXT("Main mesh Set writes the engine CurrentMontage"), Ability->GetCurrentMontage(), MontageA);
	TestEqual(TEXT("Main mesh Get reads the engine CurrentMontage"), Ability->GetCurrentMontageForMesh(Fixture.MainMesh), MontageA);
	Ability->SetCurrentMontage(MontageB);
	TestEqual(TEXT("Engine SetCurrentMontage is visible through the main mesh accessor"), Ability->GetCurrentMontageForMesh(Fixture.MainMesh), MontageB);
	Ability->SetCurrentMontageForMesh(Fixture.MainMesh, nullptr);
	TestNull(TEXT("Clearing the main mesh clears the engine CurrentMontage"), Ability->GetCurrentMontage());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasMontageComponentGuardsTest,
	"SigilGas.Montage.ComponentGuardsAndRouting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasMontageComponentGuardsTest::RunTest(const FString& Parameters)
{
	FSigilGasMontageFixture Fixture(TEXT("SigilGasMontageComponentWorld"));
	TestTrue(TEXT("The montage fixture should be valid"), Fixture.IsValid());
	if (!Fixture.IsValid())
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Fixture.Actor->GetAbilitySystem();
	USigilGasTestAbility* Ability = Fixture.Ability;
	UAnimMontage* Montage = NewObject<UAnimMontage>(GetTransientPackage(), TEXT("SigilGasTestMontageGuard"));
	const FGameplayAbilityActivationInfo ActivationInfo;

	TestTrue(TEXT("The test world is standalone, so the actor info counts as locally controlled"), ASC->AbilityActorInfo->IsLocallyControlled());

	// Invalid inputs never play and never leave bookkeeping behind.
	TestEqual(TEXT("Null mesh returns -1"), ASC->PlayMontageForMesh(Ability, nullptr, ActivationInfo, Montage, 1.f), -1.f);
	TestEqual(TEXT("Null montage returns -1"), ASC->PlayMontageForMesh(Ability, Fixture.SecondaryMesh, ActivationInfo, nullptr, 1.f), -1.f);

	// A mesh that is not owned by the avatar is rejected.
	TestEqual(TEXT("A stranger's mesh returns -1"), ASC->PlayMontageForMesh(Ability, Fixture.StrangerMesh, ActivationInfo, Montage, 1.f), -1.f);
	TestNull(TEXT("A stranger's mesh is not tracked"), ASC->GetCurrentMontageForMesh(Fixture.StrangerMesh));

	// Avatar-owned meshes without a skeletal mesh asset have no anim instance: no play, no bookkeeping, no crash.
	TestEqual(TEXT("A secondary mesh without an anim instance returns -1"), ASC->PlayMontageForMesh(Ability, Fixture.SecondaryMesh, ActivationInfo, Montage, 1.f), -1.f);
	TestEqual(TEXT("A weapon mesh owned through the owner chain but without an anim instance returns -1"), ASC->PlayMontageForMesh(Ability, Fixture.WeaponMesh, ActivationInfo, Montage, 1.f), -1.f);
	TestEqual(TEXT("The main mesh without an anim instance returns -1 through the engine path"), ASC->PlayMontageForMesh(Ability, Fixture.MainMesh, ActivationInfo, Montage, 1.f), -1.f);
	TestNull(TEXT("No montage is tracked on the secondary mesh"), ASC->GetCurrentMontageForMesh(Fixture.SecondaryMesh));
	TestNull(TEXT("No animating ability is tracked on the secondary mesh"), ASC->GetAnimatingAbilityForMesh(Fixture.SecondaryMesh));
	TestFalse(TEXT("The ability animates no mesh"), ASC->IsAnimatingAbilityForAnyMesh(Ability));
	TestNull(TEXT("The ability tracks nothing on the secondary mesh"), Ability->GetCurrentMontageForMesh(Fixture.SecondaryMesh));

	// Main mesh queries route to the engine's single-mesh state.
	Ability->SetCurrentMontage(Montage);
	TestEqual(TEXT("Main mesh montage query routes to the engine CurrentMontage"), ASC->GetCurrentMontageForMesh(Fixture.MainMesh), ASC->GetCurrentMontage());
	TestEqual(TEXT("Main mesh animating ability query routes to the engine animating ability"), ASC->GetAnimatingAbilityForMesh(Fixture.MainMesh), ASC->GetAnimatingAbility());
	Ability->SetCurrentMontage(nullptr);

	// Stop / clear calls on untracked meshes are no-ops.
	ASC->CurrentMontageStopForMesh(Fixture.SecondaryMesh);
	ASC->CurrentMontageJumpToSectionForMesh(Fixture.SecondaryMesh, TEXT("Loop"));
	ASC->CurrentMontageSetNextSectionNameForMesh(Fixture.SecondaryMesh, TEXT("A"), TEXT("B"));
	ASC->CurrentMontageSetPlayRateForMesh(Fixture.SecondaryMesh, 2.f);
	ASC->ClearAnimatingAbilityForMesh(Fixture.SecondaryMesh, Ability);
	ASC->ClearAnimatingAbilityForAllMeshes(Ability);
	ASC->StopAllCurrentMontages();
	Ability->MontageStopForAllMeshes();
	TestFalse(TEXT("After the no-op calls the ability still animates nothing"), ASC->IsAnimatingAbilityForAnyMesh(Ability));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasMontageTaskMeshTest,
	"SigilGas.Montage.TaskCarriesMeshAndCancelsWithoutAnimInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasMontageTaskMeshTest::RunTest(const FString& Parameters)
{
	FSigilGasMontageFixture Fixture(TEXT("SigilGasMontageTaskWorld"));
	TestTrue(TEXT("The montage fixture should be valid"), Fixture.IsValid());
	if (!Fixture.IsValid())
	{
		return false;
	}

	USigilAbilitySystemComponent* ASC = Fixture.Actor->GetAbilitySystem();
	USigilGasTestAbility* Ability = Fixture.Ability;
	Ability->bEndImmediately = false;
	TestTrue(TEXT("The ability activates and stays active"), ASC->TryActivateAbility(Fixture.Handle));
	TestTrue(TEXT("The ability is active"), Ability->IsActive());

	UAnimMontage* Montage = NewObject<UAnimMontage>(GetTransientPackage(), TEXT("SigilGasTestMontageTask"));
	USigilGasTestAsyncListener* Listener = NewObject<USigilGasTestAsyncListener>(Fixture.Actor);

	// Ext params carry the mesh.
	FSigilPlayMontageAndWaitForEventTaskParams Params;
	Params.Mesh = Fixture.SecondaryMesh;
	Params.MontageToPlay = Montage;
	USigilAbilityTask_PlayMontageAndWaitForEvent* ExtTask = USigilAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEventExt(Ability, Params);
	TestNotNull(TEXT("The Ext task should be created"), ExtTask);
	TestEqual(TEXT("The Ext task carries the mesh"), ExtTask ? ExtTask->GetTargetMesh() : nullptr, Fixture.SecondaryMesh);

	// The legacy entry point leaves the mesh null (engine path).
	USigilAbilityTask_PlayMontageAndWaitForEvent* LegacyTask = USigilAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(Ability, NAME_None, Montage, FGameplayTagContainer());
	TestNotNull(TEXT("The legacy task should be created"), LegacyTask);
	TestNull(TEXT("The legacy task has no mesh"), LegacyTask ? LegacyTask->GetTargetMesh() : nullptr);

	// The mesh entry point: without an anim instance the task cancels instead of crashing.
	USigilAbilityTask_PlayMontageAndWaitForEvent* MeshTask = USigilAbilityTask_PlayMontageAndWaitForEvent::PlayMontageForMeshAndWaitForEvent(Ability, NAME_None, Fixture.SecondaryMesh, Montage, FGameplayTagContainer());
	TestNotNull(TEXT("The mesh task should be created"), MeshTask);
	if (!MeshTask)
	{
		return false;
	}
	TestEqual(TEXT("The mesh task carries the mesh"), MeshTask->GetTargetMesh(), Fixture.SecondaryMesh);
	MeshTask->OnCancelled.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleMontageCancelled);
	MeshTask->ReadyForActivation();
	TestEqual(TEXT("Without an anim instance the mesh task broadcasts OnCancelled once"), Listener->MontageCancelledCount, 1);
	TestFalse(TEXT("Nothing is tracked after the failed play"), ASC->IsAnimatingAbilityForAnyMesh(Ability));

	// Ending the ability tears the tasks down without touching secondary-mesh bookkeeping that was never created.
	ASC->CancelAbilityHandle(Fixture.Handle);
	TestFalse(TEXT("The ability is inactive after cancellation"), Ability->IsActive());
	TestFalse(TEXT("No secondary mesh bookkeeping survives"), ASC->IsAnimatingAbilityForAnyMesh(Ability));

	return true;
}

#endif
#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasMontageRemoteSecondaryMeshTest,
	"SigilGas.Montage.RemoteSecondaryMeshKeepsAbilityTiming",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasMontageRemoteSecondaryMeshTest::RunTest(const FString& Parameters)
{
	FSigilGasMontageFixture Fixture(TEXT("SigilGasMontageRemoteWorld"));
	TestTrue(TEXT("The montage fixture should be valid"), Fixture.IsValid());
	if (!Fixture.IsValid())
	{
		return false;
	}

	USigilGasTestAbilitySystemComponent* ASC = Fixture.Actor->GetTestAbilitySystem();
	USigilGasTestAbility* Ability = Fixture.Ability;
	const FGameplayAbilityActivationInfo ActivationInfo;

	// Montage with a real play length so the skipped path has something to wait for.
	UAnimMontage* Montage = NewObject<UAnimMontage>(GetTransientPackage(), TEXT("SigilGasTestMontageRemote"));
	Montage->SetCompositeLength(2.f);
	TestEqual(TEXT("The montage reports its play length"), Montage->GetPlayLength(), 2.f, KINDA_SMALL_NUMBER);

	// Locally controlled (default): the ASC would try to play; without an anim instance it fails with -1.
	TestTrue(TEXT("Standalone actor info counts as locally controlled"), ASC->ShouldPlaySecondaryMeshMontages());
	TestEqual(TEXT("Local: missing anim instance is a failure (-1)"), ASC->PlayMontageForMesh(Ability, Fixture.SecondaryMesh, ActivationInfo, Montage, 1.f), -1.f);

	// Not locally controlled: the secondary mesh is skipped with 0, never -1, and nothing is tracked.
	ASC->bScriptedLocallyControlled = false;
	TestFalse(TEXT("Scripted remote control disables secondary mesh montages"), ASC->ShouldPlaySecondaryMeshMontages());
	TestEqual(TEXT("Remote: the secondary mesh is skipped with 0"), ASC->PlayMontageForMesh(Ability, Fixture.SecondaryMesh, ActivationInfo, Montage, 1.f), 0.f);
	TestEqual(TEXT("Remote: a weapon mesh is skipped with 0 as well"), ASC->PlayMontageForMesh(Ability, Fixture.WeaponMesh, ActivationInfo, Montage, 1.f), 0.f);
	TestFalse(TEXT("Remote: nothing is tracked for the skipped mesh"), ASC->HasTrackedMeshMontage(Fixture.SecondaryMesh));
	TestFalse(TEXT("Remote: the ability animates nothing"), ASC->IsAnimatingAbilityForAnyMesh(Ability));
	TestEqual(TEXT("Remote: the main mesh still goes through the engine path (no anim instance -> -1)"), ASC->PlayMontageForMesh(Ability, Fixture.MainMesh, ActivationInfo, Montage, 1.f), -1.f);

	// The task on a skipped secondary mesh must not cancel the ability; it completes after the montage length.
	Ability->bEndImmediately = false;
	TestTrue(TEXT("The ability activates and stays active"), ASC->TryActivateAbility(Fixture.Handle));

	USigilGasTestAsyncListener* Listener = NewObject<USigilGasTestAsyncListener>(Fixture.Actor);
	USigilAbilityTask_PlayMontageAndWaitForEvent* Task = USigilAbilityTask_PlayMontageAndWaitForEvent::PlayMontageForMeshAndWaitForEvent(Ability, NAME_None, Fixture.SecondaryMesh, Montage, FGameplayTagContainer(), 2.f);
	TestNotNull(TEXT("The task should be created"), Task);
	if (!Task)
	{
		return false;
	}
	Task->OnCancelled.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleMontageCancelled);
	Task->OnBlendOut.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleMontageBlendOut);
	Task->OnCompleted.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleMontageCompleted);
	Task->ReadyForActivation();

	TestEqual(TEXT("Skipping the secondary mesh never broadcasts OnCancelled"), Listener->MontageCancelledCount, 0);
	TestEqual(TEXT("The task has not completed yet"), Listener->MontageCompletedCount, 0);
	TestTrue(TEXT("The ability is still active while the skipped montage 'plays'"), Ability->IsActive());
	TestTrue(TEXT("The task is still active"), Task->IsActive());

	// FTimerManager::Tick only runs once per GFrameCounter value, so bump the frame counter before every manual tick.
	auto TickTimers = [&Fixture](float DeltaSeconds)
	{
		++GFrameCounter;
		Fixture.World.World->GetTimerManager().Tick(DeltaSeconds);
	};

	// A timer set outside of a tick is Pending until the next tick activates it (its expiry is measured from that tick's
	// InternalTime), so run one zero-length activation tick first - exactly what happens between two engine frames.
	TickTimers(0.f);
	TestEqual(TEXT("The activation tick does not complete the task"), Listener->MontageCompletedCount, 0);

	// Montage length 2s at rate 2 -> 1s. Half a second in: still waiting.
	TickTimers(0.5f);
	TestEqual(TEXT("Halfway through the scaled length the task has not completed"), Listener->MontageCompletedCount, 0);

	// Past the scaled length: blend out + completed exactly once, task ended, ability untouched.
	TickTimers(0.6f);
	TestEqual(TEXT("OnBlendOut fires once after the scaled montage length"), Listener->MontageBlendOutCount, 1);
	TestEqual(TEXT("OnCompleted fires once after the scaled montage length"), Listener->MontageCompletedCount, 1);
	TestEqual(TEXT("Still no OnCancelled"), Listener->MontageCancelledCount, 0);
	TestFalse(TEXT("The task ended itself"), Task->IsActive());
	TestTrue(TEXT("The ability keeps running; only the ability decides when to end"), Ability->IsActive());

	// Cancelling the ability with a pending skipped-mesh task clears its timer without firing it.
	Listener->MontageCompletedCount = 0;
	USigilAbilityTask_PlayMontageAndWaitForEvent* PendingTask = USigilAbilityTask_PlayMontageAndWaitForEvent::PlayMontageForMeshAndWaitForEvent(Ability, NAME_None, Fixture.SecondaryMesh, Montage, FGameplayTagContainer(), 1.f);
	PendingTask->OnCompleted.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleMontageCompleted);
	PendingTask->OnCancelled.AddDynamic(Listener, &USigilGasTestAsyncListener::HandleMontageCancelled);
	PendingTask->ReadyForActivation();
	ASC->CancelAbilityHandle(Fixture.Handle);
	TestFalse(TEXT("The ability is inactive after cancellation"), Ability->IsActive());
	TickTimers(5.f);
	TestEqual(TEXT("A cleared skipped-mesh timer never completes"), Listener->MontageCompletedCount, 0);

	return true;
}

#endif
#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasMontageBookkeepingReleaseTest,
	"SigilGas.Montage.BookkeepingIsReleased",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasMontageBookkeepingReleaseTest::RunTest(const FString& Parameters)
{
	FSigilGasMontageFixture Fixture(TEXT("SigilGasMontageReleaseWorld"));
	TestTrue(TEXT("The montage fixture should be valid"), Fixture.IsValid());
	if (!Fixture.IsValid())
	{
		return false;
	}

	USigilGasTestAbilitySystemComponent* ASC = Fixture.Actor->GetTestAbilitySystem();
	USigilGasTestAbility* Ability = Fixture.Ability;
	UAnimMontage* Montage = NewObject<UAnimMontage>(GetTransientPackage(), TEXT("SigilGasTestMontageRelease"));

	TestEqual(TEXT("No ASC entries initially"), ASC->GetTrackedMeshMontageCount(), 0);
	TestEqual(TEXT("No ability entries initially"), Ability->GetTrackedMeshMontageCount(), 0);

	// ClearAnimatingAbilityForMesh drops the entry on both sides.
	ASC->AddTrackedMeshMontageForTest(Fixture.SecondaryMesh, Montage, Ability);
	TestEqual(TEXT("One ASC entry after tracking"), ASC->GetTrackedMeshMontageCount(), 1);
	TestEqual(TEXT("One ability entry after tracking"), Ability->GetTrackedMeshMontageCount(), 1);
	TestTrue(TEXT("The ability animates the tracked mesh"), ASC->IsAnimatingAbilityForAnyMesh(Ability));
	ASC->ClearAnimatingAbilityForMesh(Fixture.SecondaryMesh, Ability);
	TestEqual(TEXT("ClearAnimatingAbilityForMesh removes the ASC entry"), ASC->GetTrackedMeshMontageCount(), 0);
	TestEqual(TEXT("ClearAnimatingAbilityForMesh removes the ability entry"), Ability->GetTrackedMeshMontageCount(), 0);
	TestFalse(TEXT("Nothing is animated after the clear"), ASC->IsAnimatingAbilityForAnyMesh(Ability));

	// ClearAnimatingAbilityForAllMeshes drops every entry of the ability, leaves other abilities alone.
	ASC->AddTrackedMeshMontageForTest(Fixture.SecondaryMesh, Montage, Ability);
	ASC->AddTrackedMeshMontageForTest(Fixture.WeaponMesh, Montage, Ability);
	TestEqual(TEXT("Two ASC entries for two meshes"), ASC->GetTrackedMeshMontageCount(), 2);
	ASC->ClearAnimatingAbilityForAllMeshes(Ability);
	TestEqual(TEXT("ClearAnimatingAbilityForAllMeshes removes all ASC entries of the ability"), ASC->GetTrackedMeshMontageCount(), 0);
	TestEqual(TEXT("...and all ability entries"), Ability->GetTrackedMeshMontageCount(), 0);

	// Ending the ability (NotifyAbilityEnded) releases entries too.
	Ability->bEndImmediately = false;
	TestTrue(TEXT("The ability activates"), ASC->TryActivateAbility(Fixture.Handle));
	ASC->AddTrackedMeshMontageForTest(Fixture.SecondaryMesh, Montage, Ability);
	TestEqual(TEXT("Entry tracked while the ability runs"), ASC->GetTrackedMeshMontageCount(), 1);
	ASC->CancelAbilityHandle(Fixture.Handle);
	TestFalse(TEXT("The ability ended"), Ability->IsActive());
	TestEqual(TEXT("Ending the ability removes its ASC entries"), ASC->GetTrackedMeshMontageCount(), 0);
	TestEqual(TEXT("Ending the ability removes its ability entries"), Ability->GetTrackedMeshMontageCount(), 0);

	// A destroyed mesh leaves a stale entry that is pruned on the next lookup-or-add.
	USkeletalMeshComponent* Temporary = NewObject<USkeletalMeshComponent>(Fixture.Actor, TEXT("TemporaryMesh"));
	Temporary->RegisterComponent();
	ASC->AddTrackedMeshMontageForTest(Temporary, Montage, Ability);
	TestEqual(TEXT("The temporary mesh is tracked"), ASC->GetTrackedMeshMontageCount(), 1);
	Temporary->DestroyComponent();
	ASC->AddTrackedMeshMontageForTest(Fixture.SecondaryMesh, Montage, Ability);
	TestEqual(TEXT("Adding another mesh prunes the destroyed one (no zombie)"), ASC->GetTrackedMeshMontageCount(), 1);
	TestTrue(TEXT("The live mesh survives the prune"), ASC->HasTrackedMeshMontage(Fixture.SecondaryMesh));
	ASC->ClearAnimatingAbilityForAllMeshes(Ability);

	// Ability side: a null Set removes the entry; avatar change and removal reset the list.
	Ability->SetCurrentMontageForMesh(Fixture.SecondaryMesh, Montage);
	Ability->SetCurrentMontageForMesh(Fixture.SecondaryMesh, nullptr);
	TestEqual(TEXT("Setting null removes the ability entry instead of keeping a (mesh, null) pair"), Ability->GetTrackedMeshMontageCount(), 0);

	Ability->SetCurrentMontageForMesh(Fixture.SecondaryMesh, Montage);
	Ability->SetCurrentMontageForMesh(Fixture.WeaponMesh, Montage);
	TestEqual(TEXT("Two ability entries before the avatar change"), Ability->GetTrackedMeshMontageCount(), 2);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* NewAvatar = Fixture.World.World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
	ASC->InitAbilityActorInfo(Fixture.Actor, NewAvatar);
	TestEqual(TEXT("OnAvatarSet resets the ability's per-mesh bookkeeping"), Ability->GetTrackedMeshMontageCount(), 0);
	TestNull(TEXT("Stale meshes are not reported after the avatar change"), Ability->GetCurrentMontageForMesh(Fixture.SecondaryMesh));

	Ability->SetCurrentMontageForMesh(Fixture.SecondaryMesh, Montage);
	TestEqual(TEXT("One ability entry before removal"), Ability->GetTrackedMeshMontageCount(), 1);
	ASC->ClearAbility(Fixture.Handle);
	TestEqual(TEXT("OnRemoveAbility resets the ability's per-mesh bookkeeping"), Ability->GetTrackedMeshMontageCount(), 0);

	return true;
}

#endif
