// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilInputSystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "SigilInputLogChannels.h"
#include "SigilInputConfig.h"
#include "SigilInputControlSetup.h"
#include "SigilInputFunctionLibrary.h"
#include "Engine/World.h"
#include "Misc/DataValidation.h"

USigilInputSystemComponent::USigilInputSystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USigilInputSystemComponent::OnRegister()
{
	Super::OnRegister();

	const UWorld* World = GetWorld();

	if (World->IsGameWorld())
	{
		APlayerController* PCOwner = GetOwner<APlayerController>();

		APawn* PawnOwner = GetOwner<APawn>();

		OwnerType = PCOwner ? ESigilOwnerType::PC : ESigilOwnerType::Pawn;

		if (OwnerType == ESigilOwnerType::Pawn)
		{
			if (ensure(PawnOwner))
			{
				PawnOwner->ReceiveRestartedDelegate.AddDynamic(this, &USigilInputSystemComponent::OnPawnRestarted);
				PawnOwner->ReceiveControllerChangedDelegate.AddDynamic(this, &USigilInputSystemComponent::OnControllerChanged);

				// If our pawn has an input component we were added after restart
				if (PawnOwner->InputComponent)
				{
					OnPawnRestarted(PawnOwner);
				}
			}
		}

		if (OwnerType == ESigilOwnerType::PC)
		{
			// If our PlayerController already set up its input component we were added afterwards.
			// Otherwise the owning PC binds from its SetupInputComponent.
			if (ensure(PCOwner) && PCOwner->InputComponent)
			{
				BindPlayerControllerInput(PCOwner->InputComponent);
			}
		}
	}
}

void USigilInputSystemComponent::OnUnregister()
{
	const UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		CleanupInputComponent();

		if (OwnerType == ESigilOwnerType::Pawn)
		{
			if (APawn* PawnOwner = GetOwner<APawn>())
			{
				PawnOwner->ReceiveRestartedDelegate.RemoveAll(this);
				PawnOwner->ReceiveControllerChangedDelegate.RemoveAll(this);
			}
		}
	}

	Super::OnUnregister();
}

void USigilInputSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupInputComponent();

	Super::EndPlay(EndPlayReason);
}

bool USigilInputSystemComponent::BindPlayerControllerInput(UInputComponent* NewInputComponent)
{
	// Read-only preflight: any failure leaves every binding, context and transient state untouched.
	APlayerController* PC = GetOwner<APlayerController>();
	if (OwnerType != ESigilOwnerType::PC || !PC)
	{
		UE_LOG(LogSigilInput, Warning, TEXT("BindPlayerControllerInput requires a PlayerController owner. Owner: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NONE"));
		return false;
	}

	if (!PC->IsLocalController() || !PC->GetLocalPlayer())
	{
		UE_LOG(LogSigilInput, Verbose, TEXT("BindPlayerControllerInput skipped for non-local PlayerController: %s"), *PC->GetName());
		return false;
	}

	UEnhancedInputComponent* NewEnhancedInputComponent = Cast<UEnhancedInputComponent>(NewInputComponent);
	if (!NewEnhancedInputComponent)
	{
		UE_LOG(LogSigilInput, Error, TEXT("BindPlayerControllerInput requires an EnhancedInputComponent. PlayerController: %s"), *PC->GetName());
		return false;
	}

	if (!GetEnhancedInputSubsystem())
	{
		UE_LOG(LogSigilInput, Error, TEXT("BindPlayerControllerInput requires the Enhanced Input local player subsystem. PlayerController: %s"), *PC->GetName());
		return false;
	}

	if (!InputConfig || !GetCurrentInputSetup())
	{
		UE_LOG(LogSigilInput, Error, TEXT("BindPlayerControllerInput requires InputConfig and a current InputControlSetup. PlayerController: %s"), *PC->GetName());
		return false;
	}

	if (InputComponent == NewEnhancedInputComponent)
	{
		return true;
	}

	if (InputComponent)
	{
		TeardownGameplayRouteCore();
		ReleaseInputBindingsCore();
	}

	return BindInputComponentCore(NewEnhancedInputComponent, true);
}

void USigilInputSystemComponent::UnbindPlayerControllerInput()
{
	if (OwnerType != ESigilOwnerType::PC)
	{
		return;
	}

	TeardownGameplayRouteCore();
	ReleaseInputBindingsCore();
}

void USigilInputSystemComponent::SetGameplayRoutingEnabled(bool bEnabled)
{
	if (!bEnabled)
	{
		if (bGameplayRoutingEnabled)
		{
			TeardownGameplayRouteCore();
		}
		return;
	}

	if (bGameplayRoutingEnabled)
	{
		return;
	}

	if (!InputComponent)
	{
		UE_LOG(LogSigilInput, Warning, TEXT("SetGameplayRoutingEnabled(true) ignored: no bound input component. Owner: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NONE"));
		return;
	}

	AddOwnedGameplayMappingContext();
	bGameplayRoutingEnabled = true;
}

bool USigilInputSystemComponent::IsPlayerControllerInputBound() const
{
	return OwnerType == ESigilOwnerType::PC && InputComponent != nullptr;
}

bool USigilInputSystemComponent::IsGameplayRoutingEnabled() const
{
	return bGameplayRoutingEnabled;
}

APawn* USigilInputSystemComponent::GetControlledPawn() const
{
	if (OwnerType == ESigilOwnerType::Pawn)
	{
		return GetOwner<APawn>();
	}
	if (OwnerType == ESigilOwnerType::PC)
	{
		APlayerController* PC = GetOwner<APlayerController>();
		return PC ? PC->GetPawn() : nullptr;
	}
	return nullptr;
}

USigilInputSystemComponent* USigilInputSystemComponent::GetInputSystemComponent(const AActor* Actor)
{
	return IsValid(Actor) ? Actor->FindComponentByClass<USigilInputSystemComponent>() : nullptr;
}

bool USigilInputSystemComponent::FindInputSystemComponent(const AActor* Actor, USigilInputSystemComponent*& Component)
{
	Component = GetInputSystemComponent(Actor);
	return Component != nullptr;
}

void USigilInputSystemComponent::OnSetupPlayerInputComponent_Implementation(UEnhancedInputComponent* NewInputComponent)
{
	BindInputActions();
}

void USigilInputSystemComponent::OnCleanupPlayerInputComponent_Implementation(UEnhancedInputComponent* PrevInputComponent)
{
}

void USigilInputSystemComponent::OnPawnRestarted(APawn* Pawn)
{
	UE_LOG(LogSigilInput, Verbose, TEXT("OnPawnRestarted Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"))
	if (ensure(Pawn && Pawn == GetOwner()) && Pawn->InputComponent)
	{
		CleanupInputComponent();

		if (Pawn->InputComponent)
		{
			SetupInputComponent(Pawn->InputComponent);
		}
	}
}

void USigilInputSystemComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	UE_LOG(LogSigilInput, Verbose, TEXT("USigilInputSystemComponent::OnControllerChanged Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"))
	// Only handle releasing, restart is a better time to handle binding
	if (ensure(Pawn && Pawn == GetOwner()) && OldController)
	{
		CleanupInputComponent(OldController);
	}
}

void USigilInputSystemComponent::CleanInputActionValueBindings()
{
	// Only bindings created by this component are removed; borrowed bindings keep their original handle.
	if (InputComponent)
	{
		for (const uint32 Handle : OwnedActionValueBindingHandles)
		{
			InputComponent->RemoveBindingByHandle(Handle);
		}
	}
	OwnedActionValueBindingHandles.Empty();
	InputActionValueBindings.Empty();
}

void USigilInputSystemComponent::SetupInputActionValueBindings()
{
	if (!InputComponent || !InputConfig)
	{
		return;
	}

	for (auto& Mapping : InputConfig->InputActionMappings)
	{
		if (!Mapping.Value.bValueBinding || !Mapping.Value.InputAction || InputActionValueBindings.Contains(Mapping.Key))
		{
			continue;
		}

		const UInputAction* Action = Mapping.Value.InputAction;
		const bool bBorrowed = InputComponent->GetActionValueBindings().ContainsByPredicate([Action](const FEnhancedInputActionValueBinding& Binding)
		{
			return Binding.GetAction() == Action;
		});

		const uint32 Handle = InputComponent->BindActionValue(Action).GetHandle();
		if (!bBorrowed)
		{
			OwnedActionValueBindingHandles.Add(Handle);
		}
		InputActionValueBindings.Emplace(Mapping.Key, static_cast<int32>(Handle));
		UE_LOG(LogSigilInput, Verbose, TEXT("Setup input action value binding for InputTag:{%s} handle:{%u} borrowed:{%d}"), *Mapping.Key.ToString(), Handle, bBorrowed);
	}
}

void USigilInputSystemComponent::SetupInputComponent(UInputComponent* NewInputComponent)
{
	if (OwnerType == ESigilOwnerType::PC)
	{
		BindPlayerControllerInput(NewInputComponent);
		return;
	}

	UEnhancedInputComponent* NewEnhancedInputComponent = Cast<UEnhancedInputComponent>(NewInputComponent);
	if (!ensureMsgf(NewEnhancedInputComponent, TEXT("Project must use EnhancedInputComponent to support PlayerControlsComponent")))
	{
		return;
	}

	if (!InputConfig)
	{
		UE_LOG(LogSigilInput, Error, TEXT("SetupInputComponent requires InputConfig. Owner: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NONE"));
		return;
	}

	if (InputComponent != NewEnhancedInputComponent)
	{
		if (InputComponent)
		{
			TeardownGameplayRouteCore();
			ReleaseInputBindingsCore();
		}

		if (!BindInputComponentCore(NewEnhancedInputComponent, false))
		{
			return;
		}
	}

	// Pawn hosts keep their original behavior: gameplay routing opens as soon as input is set up.
	SetGameplayRoutingEnabled(true);
}

void USigilInputSystemComponent::CleanupInputComponent(AController* OldController)
{
	if (OwnerType == ESigilOwnerType::PC)
	{
		UnbindPlayerControllerInput();
		return;
	}

	// The owned mapping context is removed from the subsystem it was added to, so OldController is no longer needed.
	TeardownGameplayRouteCore();
	ReleaseInputBindingsCore();
}

void USigilInputSystemComponent::NeutralizeGameplayReceiver(APawn* OldReceiver, const FGameplayTagContainer& HeldInputTags)
{
	if (!IsValid(OldReceiver) || OldReceiver != GetControlledPawn())
	{
		// Never forward an old release to a different pawn; the server-side pawn/ASC lifecycle owns authoritative cancel.
		UE_LOG(LogSigilInput, Verbose, TEXT("Skip neutralizing %d held inputs: receiver %s is no longer the controlled pawn."), HeldInputTags.Num(),
		       OldReceiver ? *OldReceiver->GetName() : TEXT("NONE"));
		return;
	}

	const FInputActionInstance EmptyActionData;
	for (const FGameplayTag& InputTag : HeldInputTags)
	{
		ProcessInput(EmptyActionData, InputTag, ETriggerEvent::Canceled);
	}
}

bool USigilInputSystemComponent::BindInputComponentCore(UEnhancedInputComponent* NewInputComponent, bool bRequireValidActions)
{
	check(NewInputComponent && InputConfig && !InputComponent);

	if (bRequireValidActions)
	{
		for (const auto& Mapping : InputConfig->InputActionMappings)
		{
			if (!Mapping.Value.InputAction)
			{
				UE_LOG(LogSigilInput, Error, TEXT("Input bind failed: InputTag:{%s} has no InputAction in %s."), *Mapping.Key.ToString(), *InputConfig->GetName());
				return false;
			}
		}
	}

	InputComponent = NewInputComponent;

	SetupInputActionValueBindings();

	for (const auto& Pair : InputConfig->InputActionMappings)
	{
		const UInputAction* Action = Pair.Value.InputAction;
		if (!Action)
		{
			UE_LOG(LogSigilInput, Warning, TEXT("Skip binding InputTag:{%s}: no InputAction in %s."), *Pair.Key.ToString(), *InputConfig->GetName());
			continue;
		}

		for (const ETriggerEvent TriggerEvent : {ETriggerEvent::Triggered, ETriggerEvent::Started, ETriggerEvent::Ongoing, ETriggerEvent::Completed, ETriggerEvent::Canceled})
		{
			OwnedActionEventBindingHandles.Add(InputComponent->BindAction(Action, TriggerEvent, this, &ThisClass::InputActionCallback, Pair.Key, TriggerEvent).GetHandle());
		}
	}

	++BindingGeneration;

	UE_LOG(LogSigilInput, Verbose, TEXT("SetupInputComponent for Pawn/PC: %s, generation: %d"), GetOwner() ? *GetOwner()->GetName() : TEXT("NONE"), BindingGeneration)
	OnSetupPlayerInputComponent(InputComponent);
	SetupInputComponentEvent.Broadcast(InputComponent);
	return true;
}

void USigilInputSystemComponent::ReleaseInputBindingsCore()
{
	if (!InputComponent)
	{
		OwnedActionEventBindingHandles.Empty();
		OwnedActionValueBindingHandles.Empty();
		InputActionValueBindings.Empty();
		return;
	}

	OnCleanupPlayerInputComponent(InputComponent);
	CleanupInputComponentEvent.Broadcast(InputComponent);

	for (const uint32 Handle : OwnedActionEventBindingHandles)
	{
		InputComponent->RemoveBindingByHandle(Handle);
	}
	OwnedActionEventBindingHandles.Empty();

	CleanInputActionValueBindings();

	InputComponent = nullptr;
}

void USigilInputSystemComponent::TeardownGameplayRouteCore()
{
	if (bTearingDownGameplayRoute)
	{
		return;
	}
	TGuardValue<bool> TeardownGuard(bTearingDownGameplayRoute, true);

	// 1. Neutralize the recorded receiver exactly once, while the route still points at it.
	APawn* OldReceiver = RoutedGameplayReceiver.Get();
	const FGameplayTagContainer HeldInputTags = ActiveHeldInputTags;
	ActiveHeldInputTags.Reset();
	RoutedGameplayReceiver.Reset();
	if (bGameplayRoutingEnabled && !HeldInputTags.IsEmpty())
	{
		NeutralizeGameplayReceiver(OldReceiver, HeldInputTags);
	}

	// 2. Close the route.
	bGameplayRoutingEnabled = false;

	// 3. Remove the owned gameplay mapping context.
	RemoveOwnedGameplayMappingContext();

	// 4. Clear transient state.
	ResetTransientInputStateAfterOrderedTeardown();
}

void USigilInputSystemComponent::ResetTransientInputStateAfterOrderedTeardown()
{
	ensure(bTearingDownGameplayRoute && !bGameplayRoutingEnabled && !bOwnsGameplayMappingContext);

	ActiveBufferWindows.Empty();
	CurrentBufferedInput = FSigilBufferedInput();
	LastBufferedInput = FSigilBufferedInput();
	PassedInputEntries.Empty();
	BlockedInputEntries.Empty();
	BufferedInputEntries.Empty();
	LastInputActionValues.Empty();
	ActiveHeldInputTags.Reset();
	RoutedGameplayReceiver.Reset();
}

void USigilInputSystemComponent::AddOwnedGameplayMappingContext()
{
	if (bOwnsGameplayMappingContext || !InputMappingContext)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (!Subsystem || Subsystem->HasMappingContext(InputMappingContext))
	{
		// A context added by someone else is borrowed and never removed by this component.
		return;
	}

	FModifyContextOptions Options;
	Options.bIgnoreAllPressedKeysUntilRelease = true;
	Subsystem->AddMappingContext(InputMappingContext, InputPriority, Options);
	OwnedMappingContextSubsystem = Subsystem;
	bOwnsGameplayMappingContext = true;
}

void USigilInputSystemComponent::RemoveOwnedGameplayMappingContext()
{
	if (bOwnsGameplayMappingContext && InputMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = OwnedMappingContextSubsystem.Get())
		{
			Subsystem->RemoveMappingContext(InputMappingContext);
		}
	}
	OwnedMappingContextSubsystem.Reset();
	bOwnsGameplayMappingContext = false;
}

void USigilInputSystemComponent::RecordRoutedInputEvent(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (TriggerEvent == ETriggerEvent::Started)
	{
		ActiveHeldInputTags.AddTag(InputTag);
		if (!RoutedGameplayReceiver.IsValid())
		{
			RoutedGameplayReceiver = GetControlledPawn();
		}
	}
	else if (TriggerEvent == ETriggerEvent::Completed || TriggerEvent == ETriggerEvent::Canceled)
	{
		ActiveHeldInputTags.RemoveTag(InputTag);
		if (ActiveHeldInputTags.IsEmpty())
		{
			RoutedGameplayReceiver.Reset();
		}
	}
}

UEnhancedInputLocalPlayerSubsystem* USigilInputSystemComponent::GetEnhancedInputSubsystem(AController* OldController) const
{
	if (OwnerType == ESigilOwnerType::Pawn && !GetOwner<APawn>())
	{
		return nullptr;
	}
	const APawn* PawnOwner = GetOwner<APawn>();

	const APlayerController* PC = PawnOwner ? PawnOwner->GetController<APlayerController>() : GetOwner<APlayerController>();
	if (!PC)
	{
		PC = Cast<APlayerController>(OldController);
		if (!PC)
		{
			return nullptr;
		}
	}

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return nullptr;
	}

	return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

void USigilInputSystemComponent::BindInputActions()
{
	// Generic action event bindings are created and owned by the bind core before OnSetupPlayerInputComponent runs,
	// so calling this again never duplicates them.
	UE_LOG(LogSigilInput, Verbose, TEXT("BindInputActions: %d action event bindings owned by generation %d."), OwnedActionEventBindingHandles.Num(), BindingGeneration);
}

USigilInputControlSetup* USigilInputSystemComponent::GetCurrentInputSetup() const
{
	if (InputControlSetups.IsValidIndex(InputControlSetups.Num() - 1))
	{
		return InputControlSetups[InputControlSetups.Num() - 1];
	}
	return nullptr;
}

USigilInputConfig* USigilInputSystemComponent::GetInputConfig() const
{
	return InputConfig;
}

void USigilInputSystemComponent::PushInputSetup(USigilInputControlSetup* NewSetup)
{
	if (!InputControlSetups.Contains(NewSetup))
	{
		InputControlSetups.Push(NewSetup);
	}
}

void USigilInputSystemComponent::PopInputSetup()
{
	if (InputControlSetups.Num() > 1)
	{
		InputControlSetups.Pop();
	}
}

bool USigilInputSystemComponent::CheckInputAllowed(FGameplayTag InputTag, ETriggerEvent TriggerEvent)
{
	FInputActionInstance ActionData;
	return CheckInputAllowed(ActionData, InputTag, TriggerEvent);
}

bool USigilInputSystemComponent::CheckInputAllowed(const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent)
{
	if (USigilInputControlSetup* Setup = GetCurrentInputSetup())
	{
		return Setup->CheckInput(this, ActionData, InputTag, TriggerEvent);
	}
	return true;
}

void USigilInputSystemComponent::InputActionCallback(const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent)
{
	// Fail closed while gameplay routing is off: nothing is processed, buffered or replayed later.
	if (InputTag.IsValid() && bGameplayRoutingEnabled)
	{
		if (!bProcessingInputExternally && CheckInputAllowed(ActionData, InputTag, TriggerEvent))
		{
			ProcessInput(ActionData, InputTag, TriggerEvent);
			RecordRoutedInputEvent(InputTag, TriggerEvent);
		}
		LastInputActionValues.Emplace(InputTag, ActionData.GetValue());
		OnReceivedInput.Broadcast(ActionData, InputTag, TriggerEvent);
	}
}

void USigilInputSystemComponent::ProcessInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (USigilInputControlSetup* Setup = GetCurrentInputSetup())
	{
		Setup->HandleInput(this, ActionData, InputTag, TriggerEvent);
	}
}

UInputAction* USigilInputSystemComponent::GetInputActionOfInputTag(FGameplayTag InputTag) const
{
	if (InputTag.IsValid() && InputConfig->InputActionMappings.Contains(InputTag))
		return InputConfig->InputActionMappings[InputTag].InputAction;

	return nullptr;
}

FInputActionValue USigilInputSystemComponent::GetInputActionValueOfInputTag(FGameplayTag InputTag) const
{
	if (InputComponent)
	{
		if (UInputAction* IA = GetInputActionOfInputTag(InputTag))
		{
			return InputComponent->GetBoundActionValue(IA);
		}
	}
	return FInputActionValue();
}

FInputActionValue USigilInputSystemComponent::GetLastInputActionValueOfInputTag(FGameplayTag InputTag) const
{
	if (InputTag.IsValid() && LastInputActionValues.Contains(InputTag))
	{
		return LastInputActionValues[InputTag];
	}

	return FInputActionValue();
}

void USigilInputSystemComponent::RegisterPassedInputEntry(const FSigilBufferedInput& InputEntry)
{
	if (PassedInputEntries.Num() >= MaxInputEntriesNum)
	{
		PassedInputEntries.RemoveAtSwap(0);
	}
	PassedInputEntries.Add(InputEntry);
}

void USigilInputSystemComponent::RegisterBlockedInputEntry(const FSigilBufferedInput& InputEntry)
{
	if (BlockedInputEntries.Num() >= MaxInputEntriesNum)
	{
		BlockedInputEntries.RemoveAtSwap(0);
	}
	BlockedInputEntries.Add(InputEntry);
}

void USigilInputSystemComponent::RegisterBufferedInputEntry(const FSigilBufferedInput& InputEntry)
{
	if (BufferedInputEntries.Num() >= MaxInputEntriesNum)
	{
		BufferedInputEntries.RemoveAtSwap(0);
	}
	BufferedInputEntries.Add(InputEntry);
}

#pragma region InputBuffer

bool USigilInputSystemComponent::TrySaveInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (ActiveBufferWindows.IsEmpty())
	{
		// No any buffer window.
		return false;
	}

	TArray<FGameplayTag> ActiveBufferWindowNames;
	ActiveBufferWindows.GetKeys(ActiveBufferWindowNames);

	// To see if any active buffer window can accept this input.
	int32 Counter{0};
	for (FGameplayTag& ActiveBufferWindowName : ActiveBufferWindowNames)
	{
		if (TrySaveAsBufferedInput(ActiveBufferWindowName, ActionData, InputTag, TriggerEvent))
		{
			Counter++;
		}
	}

	return Counter > 0;
}

void USigilInputSystemComponent::FireBufferedInput()
{
	ProcessInput(CurrentBufferedInput.ActionData, CurrentBufferedInput.InputTag, CurrentBufferedInput.TriggerEvent);
	RecordRoutedInputEvent(CurrentBufferedInput.InputTag, CurrentBufferedInput.TriggerEvent);
	OnFireBufferedInput.Broadcast(CurrentBufferedInput.ActionData, CurrentBufferedInput.InputTag, CurrentBufferedInput.TriggerEvent);
	ResetBufferedInput();
	CloseActiveInputBufferWindows();
}

void USigilInputSystemComponent::OpenInputBufferWindow(FGameplayTag BufferWindowName)
{
	if (!BufferWindowName.IsValid())
	{
		UE_LOG(LogSigilInput, Warning, TEXT("Passed invalid buffer name to OpenInputBufferWindow!"));
		return;
	}

	if (ActiveBufferWindows.Contains(BufferWindowName))
	{
		UE_LOG(LogSigilInput, Warning, TEXT("Can't Open buffer window(%s) as it already active!"), *BufferWindowName.ToString());
		return;
	}

	if (!ActiveBufferWindows.Contains(BufferWindowName))
	{
		if (const FSigilInputBufferWindow* Window = InputConfig->InputBufferDefinitions.FindByKey(BufferWindowName))
		{
			ActiveBufferWindows.FindOrAdd(BufferWindowName);
			UE_LOG(LogSigilInput, Verbose, TEXT("Open buffer window:%s"), *BufferWindowName.ToString());
			InputBufferWindowStateChangedEvent.Broadcast(BufferWindowName, true);
		}
	}
}

void USigilInputSystemComponent::CloseInputBufferWindow(FGameplayTag BufferWindowName)
{
	if (ActiveBufferWindows.Contains(BufferWindowName))
	{
		CurrentBufferedInput = ActiveBufferWindows[BufferWindowName];
		if (CurrentBufferedInput.InputTag.IsValid())
		{
			UE_LOG(LogSigilInput, Verbose, TEXT("Fire buffered input(:%s,TriggerEvent:%s) from Window(%s)"), *CurrentBufferedInput.InputTag.ToString(),
			       *USigilInputFunctionLibrary::GetTriggerEventString(CurrentBufferedInput.TriggerEvent), *BufferWindowName.ToString());
			FireBufferedInput();
		}
		ActiveBufferWindows.Remove(BufferWindowName);
		UE_LOG(LogSigilInput, Verbose, TEXT("Close buffer window:%s"), *BufferWindowName.ToString());
		InputBufferWindowStateChangedEvent.Broadcast(BufferWindowName, false);
	}
}

void USigilInputSystemComponent::CloseActiveInputBufferWindows()
{
	ActiveBufferWindows.Empty();
}

FSigilBufferedInput USigilInputSystemComponent::GetLastBufferedInput() const
{
	return LastBufferedInput;
}

TMap<FGameplayTag, FSigilBufferedInput> USigilInputSystemComponent::GetActiveBufferWindows() const
{
	return ActiveBufferWindows;
}

void USigilInputSystemComponent::ResetBufferedInput()
{
	LastBufferedInput = CurrentBufferedInput;
	CurrentBufferedInput = FSigilBufferedInput();
}

bool USigilInputSystemComponent::TrySaveAsBufferedInput(const FGameplayTag BufferWindowName, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (!ActiveBufferWindows.Contains(BufferWindowName))
		return false;

	FSigilBufferedInput& BufferedInput = ActiveBufferWindows[BufferWindowName];
	const FSigilInputBufferWindow* Definition = InputConfig->InputBufferDefinitions.FindByKey(BufferWindowName);

	if (Definition == nullptr)
		return false;

	const int32 AllowedInputIndex = Definition->IndexOfAllowedInput(InputTag, TriggerEvent);

	if (AllowedInputIndex == INDEX_NONE)
		return false;

	// Instance buffering.
	if (Definition->BufferType == ESigilInputBufferType::Instant)
	{
		BufferedInput.InputTag = InputTag;
		BufferedInput.ActionData = ActionData;
		BufferedInput.TriggerEvent = TriggerEvent;

		CurrentBufferedInput = BufferedInput;
		UE_LOG(LogSigilInput, Verbose, TEXT("Instantly fire buffered input(%s,TriggerEvent:%s) from Window(%s)"), *InputTag.ToString(), *USigilInputFunctionLibrary::GetTriggerEventString(TriggerEvent),
		       *BufferWindowName.ToString());
		FireBufferedInput();
		ActiveBufferWindows.Remove(BufferWindowName);

		return true;
	}

	if (BufferedInput.InputTag.IsValid() && Definition->BufferType == ESigilInputBufferType::HighestPriority)
	{
		const int32 ExistingInputIndex = Definition->IndexOfAllowedInput(BufferedInput.InputTag, BufferedInput.TriggerEvent);
		if (ExistingInputIndex != INDEX_NONE && AllowedInputIndex < ExistingInputIndex)
		{
			UE_LOG(LogSigilInput, Verbose, TEXT("Record new buffered input(%s,TriggerEvent:%s) in Window(%s),Before was input(%s,TriggerEvent:%s)"), *InputTag.ToString(),
			       *USigilInputFunctionLibrary::GetTriggerEventString(TriggerEvent), *BufferWindowName.ToString(),
			       *BufferedInput.InputTag.ToString(),
			       *USigilInputFunctionLibrary::GetTriggerEventString(BufferedInput.TriggerEvent));
			BufferedInput.InputTag = InputTag;
			BufferedInput.ActionData = ActionData;
			BufferedInput.TriggerEvent = TriggerEvent;
			return true;
		}
	}

	UE_LOG(LogSigilInput, Verbose, TEXT("Record buffered input(%s,TriggerEvent:%s) in Window(%s)"), *InputTag.ToString(),
	       *USigilInputFunctionLibrary::GetTriggerEventString(TriggerEvent), *BufferWindowName.ToString());
	BufferedInput.InputTag = InputTag;
	BufferedInput.ActionData = ActionData;
	BufferedInput.TriggerEvent = TriggerEvent;
	return true;
}

#pragma endregion

#pragma region DataValidation
#if WITH_EDITOR
EDataValidationResult USigilInputSystemComponent::IsDataValid(FDataValidationContext& Context) const
{
	if (!InputConfig)
	{
		Context.AddError(FText::FromString(TEXT("InputConfig is required.")));
		return EDataValidationResult::Invalid;
	}

	if (InputControlSetups.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("At least one InputConrolSetup is required.")));
		return EDataValidationResult::Invalid;
	}

	return Super::IsDataValid(Context);
}
#endif
#pragma endregion
