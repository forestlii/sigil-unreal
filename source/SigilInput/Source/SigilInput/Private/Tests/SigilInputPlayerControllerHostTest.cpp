// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Misc/ScopeExit.h"
#include "NativeGameplayTags.h"
#include "SigilGameplayDebugger.h"
#include "SigilInputChecker.h"
#include "SigilInputConfig.h"
#include "Tests/SigilInputTestTypes.h"
#include "UObject/Package.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(SigilInputTestTag_Jump, "Sigil.Input.InputTag.Automation.Jump");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SigilInputTestTag_Sprint, "Sigil.Input.InputTag.Automation.Sprint");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SigilInputTestTag_StateA, "Sigil.Input.Automation.State.A");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SigilInputTestTag_StateB, "Sigil.Input.Automation.State.B");

/** Test-only view of the component's private lifecycle state. 仅测试使用的组件私有生命周期状态视图。 */
struct FSigilInputSystemComponentTestAccess
{
	static void Fire(USigilInputSystemComponent& Component, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
	{
		const FInputActionInstance ActionData;
		Component.InputActionCallback(ActionData, InputTag, TriggerEvent);
	}

	static int32 Generation(const USigilInputSystemComponent& Component) { return Component.BindingGeneration; }
	static int32 OwnedEventHandles(const USigilInputSystemComponent& Component) { return Component.OwnedActionEventBindingHandles.Num(); }
	static int32 OwnedValueHandles(const USigilInputSystemComponent& Component) { return Component.OwnedActionValueBindingHandles.Num(); }
	static bool OwnsMappingContext(const USigilInputSystemComponent& Component) { return Component.bOwnsGameplayMappingContext; }
	static int32 HeldTags(const USigilInputSystemComponent& Component) { return Component.ActiveHeldInputTags.Num(); }
	static APawn* Receiver(const USigilInputSystemComponent& Component) { return Component.RoutedGameplayReceiver.Get(); }
};

namespace
{
using FAccess = FSigilInputSystemComponentTestAccess;

int32 CountEventBindings(const UEnhancedInputComponent* InputComponent, const UInputAction* Action)
{
	int32 Count = 0;
	if (InputComponent)
	{
		for (const TUniquePtr<FEnhancedInputActionEventBinding>& Binding : InputComponent->GetActionEventBindings())
		{
			Count += Binding->GetAction() == Action ? 1 : 0;
		}
	}
	return Count;
}

int32 CountValueBindings(const UEnhancedInputComponent* InputComponent, const UInputAction* Action)
{
	int32 Count = 0;
	if (InputComponent)
	{
		for (const FEnhancedInputActionValueBinding& Binding : InputComponent->GetActionValueBindings())
		{
			Count += Binding.GetAction() == Action ? 1 : 0;
		}
	}
	return Count;
}

/** Standalone game instance with one real LocalPlayer and runtime-built input assets. 独立GameInstance，含一个真实LocalPlayer与运行时构建的输入资产。 */
struct FSigilInputTestHarness
{
	explicit FSigilInputTestHarness(FAutomationTestBase& InTest) : Test(InTest)
	{
		if (!GEngine)
		{
			return;
		}

		// The headless test game instance has no CommonGameViewportClient; CommonUI's router would log an error per LocalPlayer.
		CommonUIViewportCheck = IConsoleManager::Get().FindConsoleVariable(TEXT("CommonUI.Debug.CheckGameViewportClientValid"));
		if (CommonUIViewportCheck)
		{
			bPreviousCommonUIViewportCheck = CommonUIViewportCheck->GetBool();
			CommonUIViewportCheck->Set(false, ECVF_SetByCode);
		}

		GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->AddToRoot();
		GameInstance->InitializeStandalone(MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("SigilInputPCHostTestWorld")), GetTransientPackage());
		World = GameInstance->GetWorld();
		if (!World)
		{
			return;
		}
		World->AddToRoot();
		World->InitializeActorsForPlay(FURL());

		// CreateLocalPlayer ensures a viewport exists, so add the player directly; PlayerAdded still initializes its subsystems.
		LocalPlayer = NewObject<ULocalPlayer>(GEngine, GEngine->LocalPlayerClass);
		GameInstance->AddLocalPlayer(LocalPlayer, FPlatformUserId::CreateFromInternalId(0));

		Jump = NewObject<UInputAction>(GetTransientPackage());
		Sprint = NewObject<UInputAction>(GetTransientPackage());
		MappingContext = NewObject<UInputMappingContext>(GetTransientPackage());
		MappingContext->MapKey(Jump, EKeys::SpaceBar);
		MappingContext->MapKey(Sprint, EKeys::LeftShift);

		Config = NewObject<USigilInputConfig>(GetTransientPackage());
		FSigilInputActionSetting JumpSetting;
		JumpSetting.InputAction = Jump;
		JumpSetting.bValueBinding = true;
		Config->InputActionMappings.Add(SigilInputTestTag_Jump, JumpSetting);
		FSigilInputActionSetting SprintSetting;
		SprintSetting.InputAction = Sprint;
		SprintSetting.bValueBinding = false;
		Config->InputActionMappings.Add(SigilInputTestTag_Sprint, SprintSetting);

		Processor = NewObject<USigilInputTestProcessor>(GetTransientPackage());
		Processor->InputTags.AddTag(SigilInputTestTag_Jump);
		Processor->InputTags.AddTag(SigilInputTestTag_Sprint);
		Processor->TriggerEvents = {ETriggerEvent::Started, ETriggerEvent::Completed, ETriggerEvent::Canceled};

		Setup = NewObject<USigilInputTestControlSetup>(GetTransientPackage());
		Setup->AddProcessor(Processor);

		for (UObject* Object : TArray<UObject*>{Jump, Sprint, MappingContext, Config, Processor, Setup})
		{
			Object->AddToRoot();
		}
	}

	~FSigilInputTestHarness()
	{
		for (UObject* Object : TArray<UObject*>{Jump, Sprint, MappingContext, Config, Processor, Setup})
		{
			if (Object)
			{
				Object->RemoveFromRoot();
			}
		}

		if (GameInstance)
		{
			GameInstance->Shutdown();
		}
		if (World)
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
			World->RemoveFromRoot();
		}
		if (GameInstance)
		{
			GameInstance->RemoveFromRoot();
		}
		if (CommonUIViewportCheck)
		{
			CommonUIViewportCheck->Set(bPreviousCommonUIViewportCheck, ECVF_SetByCode);
		}
	}

	bool IsValid() const
	{
		return Test.TestNotNull(TEXT("Test world should exist"), World)
			&& Test.TestNotNull(TEXT("Test local player should exist"), LocalPlayer)
			&& Test.TestNotNull(TEXT("Enhanced Input local player subsystem should exist"), GetSubsystem());
	}

	UEnhancedInputLocalPlayerSubsystem* GetSubsystem() const
	{
		return LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	}

	bool HasContext() const
	{
		const UEnhancedInputLocalPlayerSubsystem* Subsystem = GetSubsystem();
		return Subsystem && Subsystem->HasMappingContext(MappingContext);
	}

	template <typename ActorType>
	ActorType* Spawn() const
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<ActorType>(ActorType::StaticClass(), FTransform::Identity, SpawnParameters);
	}

	/** Spawns the test PC, configures its component, then attaches the LocalPlayer (which runs SetupInputComponent). */
	ASigilInputTestPlayerController* SpawnLocalController(bool bConfigure = true) const
	{
		ASigilInputTestPlayerController* PC = Spawn<ASigilInputTestPlayerController>();
		if (PC && bConfigure)
		{
			PC->SigilInput->Configure(Config, Setup, MappingContext);
		}
		if (PC)
		{
			PC->SetPlayer(LocalPlayer);
		}
		return PC;
	}

	FAutomationTestBase& Test;
	UGameInstance* GameInstance = nullptr;
	UWorld* World = nullptr;
	ULocalPlayer* LocalPlayer = nullptr;
	UInputAction* Jump = nullptr;
	UInputAction* Sprint = nullptr;
	UInputMappingContext* MappingContext = nullptr;
	USigilInputConfig* Config = nullptr;
	USigilInputTestProcessor* Processor = nullptr;
	USigilInputTestControlSetup* Setup = nullptr;
	IConsoleVariable* CommonUIViewportCheck = nullptr;
	bool bPreviousCommonUIViewportCheck = true;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInputPCHostBindLifecycleTest,
	"SigilInput.PCHost.BindLifecycleIsIdempotent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInputPCHostBindLifecycleTest::RunTest(const FString& Parameters)
{
	FSigilInputTestHarness Harness(*this);
	if (!Harness.IsValid())
	{
		return false;
	}

	ASigilInputTestPlayerController* PC = Harness.SpawnLocalController();
	if (!TestNotNull(TEXT("Local test controller should spawn"), PC))
	{
		return false;
	}
	USigilInputTestComponent* Input = PC->SigilInput;
	UEnhancedInputComponent* PCInput = Cast<UEnhancedInputComponent>(PC->InputComponent);

	// Bound from the PC's SetupInputComponent while no pawn exists.
	TestTrue(TEXT("PC SetupInputComponent ran"), PC->SetupInputComponentCalls > 0);
	TestNull(TEXT("No pawn is required to bind"), Input->GetControlledPawn());
	TestTrue(TEXT("Owning local PC binds"), Input->IsPlayerControllerInputBound());
	TestEqual(TEXT("Bound to the PC input component"), Input->GetBoundInputComponent(), PCInput);
	TestEqual(TEXT("Generation after first bind"), FAccess::Generation(*Input), 1);
	TestEqual(TEXT("Jump event bindings"), CountEventBindings(PCInput, Harness.Jump), 5);
	TestEqual(TEXT("Sprint event bindings"), CountEventBindings(PCInput, Harness.Sprint), 5);
	TestEqual(TEXT("Jump value binding"), CountValueBindings(PCInput, Harness.Jump), 1);
	TestEqual(TEXT("Sprint has no value binding"), CountValueBindings(PCInput, Harness.Sprint), 0);
	TestFalse(TEXT("Route starts disabled"), Input->IsGameplayRoutingEnabled());
	TestFalse(TEXT("Gameplay context absent while route is off"), Harness.HasContext());

	// Repeat bind and repeat route requests do not stack anything.
	TestTrue(TEXT("Repeat bind of the same component succeeds"), Input->BindPlayerControllerInput(PCInput));
	Input->SetGameplayRoutingEnabled(true);
	Input->SetGameplayRoutingEnabled(true);
	TestEqual(TEXT("Generation unchanged by repeat bind"), FAccess::Generation(*Input), 1);
	TestEqual(TEXT("Jump event bindings after repeats"), CountEventBindings(PCInput, Harness.Jump), 5);
	TestEqual(TEXT("Jump value binding after repeats"), CountValueBindings(PCInput, Harness.Jump), 1);
	TestTrue(TEXT("Route enabled"), Input->IsGameplayRoutingEnabled());
	TestTrue(TEXT("Gameplay context owned while route is on"), Harness.HasContext() && FAccess::OwnsMappingContext(*Input));

	// Rebuilt input component: old bindings released, new set bound once, route fails closed.
	UEnhancedInputComponent* RebuiltInput = NewObject<UEnhancedInputComponent>(PC);
	TestTrue(TEXT("Bind to rebuilt input component"), Input->BindPlayerControllerInput(RebuiltInput));
	TestEqual(TEXT("Generation after rebuild"), FAccess::Generation(*Input), 2);
	TestEqual(TEXT("Old component Jump event bindings released"), CountEventBindings(PCInput, Harness.Jump), 0);
	TestEqual(TEXT("Old component Jump value binding released"), CountValueBindings(PCInput, Harness.Jump), 0);
	TestEqual(TEXT("New component Jump event bindings"), CountEventBindings(RebuiltInput, Harness.Jump), 5);
	TestEqual(TEXT("New component Jump value binding"), CountValueBindings(RebuiltInput, Harness.Jump), 1);
	TestFalse(TEXT("Route disabled after rebind"), Input->IsGameplayRoutingEnabled());
	TestFalse(TEXT("Gameplay context removed after rebind"), Harness.HasContext());

	// Unbind is ordered and idempotent.
	Input->SetGameplayRoutingEnabled(true);
	Input->UnbindPlayerControllerInput();
	Input->UnbindPlayerControllerInput();
	TestFalse(TEXT("Unbound"), Input->IsPlayerControllerInputBound());
	TestFalse(TEXT("Route off after unbind"), Input->IsGameplayRoutingEnabled());
	TestFalse(TEXT("Gameplay context removed after unbind"), Harness.HasContext());
	TestEqual(TEXT("Rebuilt component event bindings released"), CountEventBindings(RebuiltInput, Harness.Jump) + CountEventBindings(RebuiltInput, Harness.Sprint), 0);
	TestEqual(TEXT("Rebuilt component value bindings released"), CountValueBindings(RebuiltInput, Harness.Jump), 0);
	TestEqual(TEXT("Owned event handles cleared"), FAccess::OwnedEventHandles(*Input), 0);
	TestEqual(TEXT("Owned value handles cleared"), FAccess::OwnedValueHandles(*Input), 0);

	// Route cannot be opened without a binding.
	AddExpectedMessage(TEXT("SetGameplayRoutingEnabled\\(true\\) ignored"), ELogVerbosity::Warning);
	Input->SetGameplayRoutingEnabled(true);
	TestFalse(TEXT("Route stays off without binding"), Input->IsGameplayRoutingEnabled());
	TestFalse(TEXT("No context without binding"), Harness.HasContext());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInputPCHostPreflightTest,
	"SigilInput.PCHost.BindPreflightFailsWithoutSideEffects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInputPCHostPreflightTest::RunTest(const FString& Parameters)
{
	FSigilInputTestHarness Harness(*this);
	if (!Harness.IsValid())
	{
		return false;
	}

	// A PlayerController without a LocalPlayer (stand-in for remote/non-owning) never binds.
	ASigilInputTestPlayerController* RemotePC = Harness.Spawn<ASigilInputTestPlayerController>();
	RemotePC->SigilInput->Configure(Harness.Config, Harness.Setup, Harness.MappingContext);
	UEnhancedInputComponent* RemoteInput = NewObject<UEnhancedInputComponent>(RemotePC);
	TestFalse(TEXT("PC without LocalPlayer does not bind"), RemotePC->SigilInput->BindPlayerControllerInput(RemoteInput));
	TestEqual(TEXT("PC without LocalPlayer adds no bindings"), CountEventBindings(RemoteInput, Harness.Jump) + CountValueBindings(RemoteInput, Harness.Jump), 0);

	// Unconfigured component on the owning local PC.
	AddExpectedError(TEXT("requires InputConfig and a current InputControlSetup"), EAutomationExpectedErrorFlags::Contains, 0);
	ASigilInputTestPlayerController* PC = Harness.SpawnLocalController(false);
	USigilInputTestComponent* Input = PC->SigilInput;
	UEnhancedInputComponent* PCInput = Cast<UEnhancedInputComponent>(PC->InputComponent);
	TestFalse(TEXT("Missing InputConfig and setup fails"), Input->IsPlayerControllerInputBound());

	Input->Configure(Harness.Config, nullptr, Harness.MappingContext);
	TestFalse(TEXT("Missing InputControlSetup fails"), Input->BindPlayerControllerInput(PCInput));

	Input->Configure(nullptr, Harness.Setup, Harness.MappingContext);
	TestFalse(TEXT("Missing InputConfig fails"), Input->BindPlayerControllerInput(PCInput));

	Input->Configure(Harness.Config, Harness.Setup, Harness.MappingContext);
	AddExpectedError(TEXT("requires an EnhancedInputComponent"), EAutomationExpectedErrorFlags::Contains, 1);
	UInputComponent* LegacyInput = NewObject<UInputComponent>(PC);
	TestFalse(TEXT("Non-enhanced input component fails"), Input->BindPlayerControllerInput(LegacyInput));

	// Mid-bind failure (an unmapped action) rolls back to zero.
	USigilInputConfig* BrokenConfig = DuplicateObject(Harness.Config, GetTransientPackage());
	BrokenConfig->InputActionMappings.Add(SigilInputTestTag_StateA, FSigilInputActionSetting());
	Input->Configure(BrokenConfig, Harness.Setup, Harness.MappingContext);
	AddExpectedError(TEXT("has no InputAction"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Unmapped action fails the bind"), Input->BindPlayerControllerInput(PCInput));

	TestFalse(TEXT("Still unbound after failures"), Input->IsPlayerControllerInputBound());
	TestEqual(TEXT("No generation after failures"), FAccess::Generation(*Input), 0);
	TestEqual(TEXT("No event bindings after failures"), CountEventBindings(PCInput, Harness.Jump) + CountEventBindings(PCInput, Harness.Sprint), 0);
	TestEqual(TEXT("No value bindings after failures"), CountValueBindings(PCInput, Harness.Jump), 0);
	TestFalse(TEXT("No context after failures"), Harness.HasContext());
	TestFalse(TEXT("Route off after failures"), Input->IsGameplayRoutingEnabled());

	// Once prerequisites are met a single reconcile binds exactly one set.
	Input->Configure(Harness.Config, Harness.Setup, Harness.MappingContext);
	TestTrue(TEXT("Bind succeeds once prerequisites exist"), Input->BindPlayerControllerInput(PCInput));
	TestTrue(TEXT("Repeat reconcile succeeds"), Input->BindPlayerControllerInput(PCInput));
	TestEqual(TEXT("Exactly one generation"), FAccess::Generation(*Input), 1);
	TestEqual(TEXT("Exactly one set of Jump event bindings"), CountEventBindings(PCInput, Harness.Jump), 5);

	// A pawn-hosted component rejects the PC-host API.
	ASigilInputTestPawn* Pawn = Harness.Spawn<ASigilInputTestPawn>();
	USigilInputTestComponent* PawnInput = NewObject<USigilInputTestComponent>(Pawn);
	PawnInput->RegisterComponent();
	AddExpectedMessage(TEXT("requires a PlayerController owner"), ELogVerbosity::Warning);
	TestFalse(TEXT("Pawn host rejects BindPlayerControllerInput"), PawnInput->BindPlayerControllerInput(PCInput));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInputPCHostBorrowedValueBindingTest,
	"SigilInput.PCHost.BorrowedValueBindingIsPreserved",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInputPCHostBorrowedValueBindingTest::RunTest(const FString& Parameters)
{
	FSigilInputTestHarness Harness(*this);
	if (!Harness.IsValid())
	{
		return false;
	}

	// Sprint gets a value binding here so both actions have one: Jump pre-bound externally (borrowed), Sprint owned.
	Harness.Config->InputActionMappings[SigilInputTestTag_Sprint].bValueBinding = true;
	AddExpectedError(TEXT("requires InputConfig and a current InputControlSetup"), EAutomationExpectedErrorFlags::Contains, 1);
	ASigilInputTestPlayerController* PC = Harness.SpawnLocalController(false);
	USigilInputTestComponent* Input = PC->SigilInput;
	Input->Configure(Harness.Config, Harness.Setup, Harness.MappingContext);
	UEnhancedInputComponent* PCInput = Cast<UEnhancedInputComponent>(PC->InputComponent);

	const uint32 ExternalHandle = PCInput->BindActionValue(Harness.Jump).GetHandle();

	for (int32 Round = 0; Round < 2; ++Round)
	{
		TestTrue(TEXT("Bind"), Input->BindPlayerControllerInput(PCInput));
		TestEqual(TEXT("Jump value binding is shared, not duplicated"), CountValueBindings(PCInput, Harness.Jump), 1);
		TestEqual(TEXT("Sprint value binding owned"), CountValueBindings(PCInput, Harness.Sprint), 1);
		TestEqual(TEXT("Only Sprint handle is owned"), FAccess::OwnedValueHandles(*Input), 1);
		Input->UnbindPlayerControllerInput();
		TestEqual(TEXT("External Jump value binding survives unbind"), CountValueBindings(PCInput, Harness.Jump), 1);
		TestEqual(TEXT("Owned Sprint value binding removed"), CountValueBindings(PCInput, Harness.Sprint), 0);
	}

	const bool bExternalHandleKept = PCInput->GetActionValueBindings().ContainsByPredicate([ExternalHandle](const FEnhancedInputActionValueBinding& Binding)
	{
		return Binding.GetHandle() == ExternalHandle;
	});
	TestTrue(TEXT("External value binding keeps its original handle"), bExternalHandleKept);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInputPCHostRoutingTeardownTest,
	"SigilInput.PCHost.RouteTeardownNeutralizesOldReceiverOnce",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInputPCHostRoutingTeardownTest::RunTest(const FString& Parameters)
{
	FSigilInputTestHarness Harness(*this);
	if (!Harness.IsValid())
	{
		return false;
	}

	ASigilInputTestPlayerController* PC = Harness.SpawnLocalController();
	USigilInputTestComponent* Input = PC->SigilInput;
	TArray<FString>& Log = Harness.Processor->Log;
	ASigilInputTestPawn* PawnA = Harness.Spawn<ASigilInputTestPawn>();
	ASigilInputTestPawn* PawnB = Harness.Spawn<ASigilInputTestPawn>();
	const FString SprintName = SigilInputTestTag_Sprint.GetTag().ToString();

	// Route off: input fails closed and nothing is buffered or recorded.
	PC->Possess(PawnA);
	FAccess::Fire(*Input, SigilInputTestTag_Sprint, ETriggerEvent::Started);
	TestEqual(TEXT("Route off drops input"), Log.Num(), 0);
	TestEqual(TEXT("Route off records no held input"), FAccess::HeldTags(*Input), 0);

	// Held Sprint on A is neutralized exactly once, before route off and context removal.
	Input->SetGameplayRoutingEnabled(true);
	FAccess::Fire(*Input, SigilInputTestTag_Sprint, ETriggerEvent::Started);
	TestEqual(TEXT("Routed start reaches A"), Log, TArray<FString>{FString::Printf(TEXT("%s|Started|%s"), *SprintName, *PawnA->GetName())});
	TestEqual(TEXT("Receiver recorded"), FAccess::Receiver(*Input), static_cast<APawn*>(PawnA));
	Input->SetGameplayRoutingEnabled(false);
	Input->SetGameplayRoutingEnabled(false);
	TestEqual(TEXT("Neutralized once"), Input->NeutralizeCalls, 1);
	TestTrue(TEXT("Neutralize ran while route was still on"), Input->bRouteEnabledDuringNeutralize);
	TestTrue(TEXT("Neutralize ran before context removal"), Input->bOwnedContextPresentDuringNeutralize);
	TestEqual(TEXT("A received a single cancel"), Log.Last(), FString::Printf(TEXT("%s|Canceled|%s"), *SprintName, *PawnA->GetName()));
	TestEqual(TEXT("Two processed events total"), Log.Num(), 2);
	TestFalse(TEXT("Context removed"), Harness.HasContext());
	TestEqual(TEXT("Held set cleared"), FAccess::HeldTags(*Input), 0);
	TestNull(TEXT("Receiver cleared"), FAccess::Receiver(*Input));

	// Released input is not neutralized again.
	Input->SetGameplayRoutingEnabled(true);
	FAccess::Fire(*Input, SigilInputTestTag_Sprint, ETriggerEvent::Started);
	FAccess::Fire(*Input, SigilInputTestTag_Sprint, ETriggerEvent::Completed);
	Input->SetGameplayRoutingEnabled(false);
	TestEqual(TEXT("No neutralize for released input"), Input->NeutralizeCalls, 1);

	// Pawn switched away while held: the old release never reaches the new pawn.
	Log.Reset();
	Input->SetGameplayRoutingEnabled(true);
	FAccess::Fire(*Input, SigilInputTestTag_Sprint, ETriggerEvent::Started);
	PC->Possess(PawnB);
	Input->SetGameplayRoutingEnabled(false);
	TestEqual(TEXT("Neutralize invoked for stale receiver"), Input->NeutralizeCalls, 2);
	TestEqual(TEXT("Stale receiver is A"), Input->LastNeutralizedReceiver.Get(), static_cast<APawn*>(PawnA));
	TestEqual(TEXT("B receives nothing from A's hold"), Log.Num(), 1);

	// Unbind with a held input on the current pawn neutralizes once, then unbinds.
	Log.Reset();
	Input->SetGameplayRoutingEnabled(true);
	FAccess::Fire(*Input, SigilInputTestTag_Sprint, ETriggerEvent::Started);
	Input->UnbindPlayerControllerInput();
	Input->UnbindPlayerControllerInput();
	TestEqual(TEXT("Unbind neutralized once"), Input->NeutralizeCalls, 3);
	TestEqual(TEXT("B got start and one cancel"), Log, TArray<FString>{
		FString::Printf(TEXT("%s|Started|%s"), *SprintName, *PawnB->GetName()),
		FString::Printf(TEXT("%s|Canceled|%s"), *SprintName, *PawnB->GetName())});
	TestFalse(TEXT("Unbound"), Input->IsPlayerControllerInputBound());
	TestFalse(TEXT("Context removed after unbind"), Harness.HasContext());

	// Without a pawn, routed input resolves a null subject and never borrows the old pawn.
	TestTrue(TEXT("Rebind"), Input->BindPlayerControllerInput(PC->InputComponent));
	PC->UnPossess();
	Log.Reset();
	Input->SetGameplayRoutingEnabled(true);
	FAccess::Fire(*Input, SigilInputTestTag_Jump, ETriggerEvent::Started);
	TestEqual(TEXT("No-pawn input sees no subject"), Log, TArray<FString>{FString::Printf(TEXT("%s|Started|None"), *SigilInputTestTag_Jump.GetTag().ToString())});
	Input->SetGameplayRoutingEnabled(false);
	TestEqual(TEXT("Null receiver is not dispatched"), Log.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInputCheckerSubjectTest,
	"SigilInput.PCHost.CheckerUsesControlledPawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInputCheckerSubjectTest::RunTest(const FString& Parameters)
{
	FSigilInputTestHarness Harness(*this);
	if (!Harness.IsValid())
	{
		return false;
	}

	USigilInputTestTagChecker* Checker = NewObject<USigilInputTestTagChecker>(GetTransientPackage());
	ASigilInputTestPawn* PawnA = Harness.Spawn<ASigilInputTestPawn>();
	ASigilInputTestPawn* PawnB = Harness.Spawn<ASigilInputTestPawn>();
	PawnA->OwnedTags.AddTag(SigilInputTestTag_StateA);
	PawnB->OwnedTags.AddTag(SigilInputTestTag_StateB);

	// Pawn host.
	USigilInputTestComponent* PawnInput = NewObject<USigilInputTestComponent>(PawnA);
	PawnInput->RegisterComponent();
	TestTrue(TEXT("Pawn host sees its pawn tags"), Checker->QueryTags(PawnInput) == PawnA->OwnedTags);

	// PC host: null, A, then B. The unconfigured pawn host reports its missing config when A is possessed.
	AddExpectedError(TEXT("SetupInputComponent requires InputConfig"), EAutomationExpectedErrorFlags::Contains, 1);
	ASigilInputTestPlayerController* PC = Harness.SpawnLocalController();
	TestTrue(TEXT("PC host without pawn sees no tags"), Checker->QueryTags(PC->SigilInput).IsEmpty());
	PC->Possess(PawnA);
	TestTrue(TEXT("PC host sees A tags"), Checker->QueryTags(PC->SigilInput) == PawnA->OwnedTags);
	PC->Possess(PawnB);
	TestTrue(TEXT("PC host sees only B tags after switch"), Checker->QueryTags(PC->SigilInput) == PawnB->OwnedTags);
	TestTrue(TEXT("Null component is safe"), Checker->QueryTags(nullptr).IsEmpty());

#if WITH_GAMEPLAY_DEBUGGER
	TestEqual(TEXT("Debugger resolves PC host from pawn target"), FSigilGameplayDebuggerCategory_Input::ResolveInputSystem(PawnB), static_cast<const USigilInputSystemComponent*>(PC->SigilInput));
	TestEqual(TEXT("Debugger resolves PC target"), FSigilGameplayDebuggerCategory_Input::ResolveInputSystem(PC), static_cast<const USigilInputSystemComponent*>(PC->SigilInput));
	TestEqual(TEXT("Debugger prefers the pawn's own component"), FSigilGameplayDebuggerCategory_Input::ResolveInputSystem(PawnA), static_cast<const USigilInputSystemComponent*>(PawnInput));
	TestNull(TEXT("Debugger null target is safe"), FSigilGameplayDebuggerCategory_Input::ResolveInputSystem(nullptr));
	TestEqual(TEXT("Debugger shows controlled pawn"), FSigilGameplayDebuggerCategory_Input::GetDisplayName(PC->SigilInput), PawnB->GetName());
	PC->UnPossess();
	TestTrue(TEXT("Debugger marks NoPawn"), FSigilGameplayDebuggerCategory_Input::GetDisplayName(PC->SigilInput).Contains(TEXT("(NoPawn)")));
#endif
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilInputPawnHostCompatibilityTest,
	"SigilInput.PawnHost.PossessBindsAndUnpossessReleases",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilInputPawnHostCompatibilityTest::RunTest(const FString& Parameters)
{
	FSigilInputTestHarness Harness(*this);
	if (!Harness.IsValid())
	{
		return false;
	}

	APlayerController* PC = Harness.Spawn<APlayerController>();
	PC->SetPlayer(Harness.LocalPlayer);

	ASigilInputTestPawn* Pawn = Harness.Spawn<ASigilInputTestPawn>();
	USigilInputTestComponent* Input = NewObject<USigilInputTestComponent>(Pawn);
	Input->Configure(Harness.Config, Harness.Setup, Harness.MappingContext);
	Input->RegisterComponent();

	PC->Possess(Pawn);
	UEnhancedInputComponent* PawnInput = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (!TestNotNull(TEXT("Possessed pawn has an enhanced input component"), PawnInput))
	{
		return false;
	}
	TestEqual(TEXT("Pawn host bound to pawn input"), Input->GetBoundInputComponent(), PawnInput);
	TestTrue(TEXT("Pawn host routes after setup"), Input->IsGameplayRoutingEnabled());
	TestTrue(TEXT("Pawn host adds its context"), Harness.HasContext());
	TestEqual(TEXT("Pawn host Jump event bindings"), CountEventBindings(PawnInput, Harness.Jump), 5);
	TestFalse(TEXT("Pawn host is not a PC binding"), Input->IsPlayerControllerInputBound());

	PC->UnPossess();
	TestNull(TEXT("Pawn host released input component"), Input->GetBoundInputComponent());
	TestFalse(TEXT("Pawn host route off after unpossess"), Input->IsGameplayRoutingEnabled());
	TestFalse(TEXT("Pawn host context removed after unpossess"), Harness.HasContext());
	TestEqual(TEXT("Pawn host event bindings removed"), CountEventBindings(PawnInput, Harness.Jump), 0);
	return true;
}

#endif
