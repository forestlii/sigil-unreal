// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameFramework/Actor.h"
#include "Tests/SigilInteractionTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInteractionRefreshesRetainedActorTest,
	"SigilInteraction.System.RefreshesRetainedActor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInteractionRefreshesRetainedActorTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage());
	AActor* Candidate = NewObject<AActor>(GetTransientPackage());
	USigilInteractionRefreshProbeComponent* Component =
		NewObject<USigilInteractionRefreshProbeComponent>(Owner);

	TestNotNull(TEXT("The interaction owner should exist"), Owner);
	TestNotNull(TEXT("The retained candidate should exist"), Candidate);
	TestNotNull(TEXT("The interaction component should exist"), Component);
	if (!Owner || !Candidate || !Component)
	{
		return false;
	}

	TestTrue(TEXT("The transient interaction owner should have authority"), Owner->HasAuthority());
	if (!Owner->HasAuthority())
	{
		return false;
	}

	Component->SetInteractableActor(Candidate);
	Component->RefreshCallCount = 0;
	Component->SetInteractableActors({ Candidate });

	TestEqual(
		TEXT("Updating candidates must refresh options when the selected actor is retained"),
		Component->RefreshCallCount,
		1);
	return true;
}

#endif
