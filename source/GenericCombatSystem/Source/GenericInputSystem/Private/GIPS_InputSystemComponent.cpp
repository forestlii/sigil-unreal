// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIPS_InputSystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "GIPS_LogChannels.h"
#include "GIPS_InputConfig.h"
#include "GIPS_InputControlSetup.h"
#include "GIPS_InputFunctionLibrary.h"
#include "Engine/World.h"
#include "Misc/DataValidation.h"

UGIPS_InputSystemComponent::UGIPS_InputSystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UGIPS_InputSystemComponent::OnRegister()
{
	Super::OnRegister();

	const UWorld* World = GetWorld();

	if (World->IsGameWorld())
	{
		APlayerController* PCOwner = GetOwner<APlayerController>();

		APawn* PawnOwner = GetOwner<APawn>();

		OwnerType = PCOwner ? EGIPS_OwnerType::PC : EGIPS_OwnerType::Pawn;

		if (OwnerType == EGIPS_OwnerType::Pawn)
		{
			if (ensure(PawnOwner))
			{
				PawnOwner->ReceiveRestartedDelegate.AddDynamic(this, &UGIPS_InputSystemComponent::OnPawnRestarted);
				PawnOwner->ReceiveControllerChangedDelegate.AddDynamic(this, &UGIPS_InputSystemComponent::OnControllerChanged);

				// If our pawn has an input component we were added after restart
				if (PawnOwner->InputComponent)
				{
					OnPawnRestarted(PawnOwner);
				}
			}
		}

		if (OwnerType == EGIPS_OwnerType::PC)
		{
			if (ensure(PCOwner))
			{
				// TODO 支持放到PC上。
			}
		}
	}
}

void UGIPS_InputSystemComponent::OnUnregister()
{
	const UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		CleanupInputComponent();

		if (OwnerType == EGIPS_OwnerType::Pawn)
		{
			APawn* PawnOwner = GetOwner<APawn>();
			PawnOwner->ReceiveRestartedDelegate.RemoveAll(this);
			PawnOwner->ReceiveControllerChangedDelegate.RemoveAll(this);
		}

		if (OwnerType == EGIPS_OwnerType::PC)
		{
			APlayerController* PCOwner = GetOwner<APlayerController>();
		}
	}

	Super::OnUnregister();
}

APawn* UGIPS_InputSystemComponent::GetControlledPawn() const
{
	if (OwnerType == EGIPS_OwnerType::Pawn)
	{
		return GetOwner<APawn>();
	}
	if (OwnerType == EGIPS_OwnerType::PC)
	{
		APlayerController* PC = GetOwner<APlayerController>();
		return PC ? PC->GetPawn() : nullptr;
	}
	return nullptr;
}

UGIPS_InputSystemComponent* UGIPS_InputSystemComponent::GetInputSystemComponent(const AActor* Actor)
{
	return IsValid(Actor) ? Actor->FindComponentByClass<UGIPS_InputSystemComponent>() : nullptr;
}

bool UGIPS_InputSystemComponent::FindInputSystemComponent(const AActor* Actor, UGIPS_InputSystemComponent*& Component)
{
	Component = GetInputSystemComponent(Actor);
	return Component != nullptr;
}

void UGIPS_InputSystemComponent::OnSetupPlayerInputComponent_Implementation(UEnhancedInputComponent* NewInputComponent)
{
	BindInputActions();
}

void UGIPS_InputSystemComponent::OnCleanupPlayerInputComponent_Implementation(UEnhancedInputComponent* PrevInputComponent)
{
}

void UGIPS_InputSystemComponent::OnPawnRestarted(APawn* Pawn)
{
	UE_LOG(LogGIPS, Verbose, TEXT("OnPawnRestarted Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"))
	if (ensure(Pawn && Pawn == GetOwner()) && Pawn->InputComponent)
	{
		CleanupInputComponent();

		if (Pawn->InputComponent)
		{
			SetupInputComponent(Pawn->InputComponent);
		}
	}
}

void UGIPS_InputSystemComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	UE_LOG(LogGIPS, Verbose, TEXT("UGIPS_InputSystemComponent::OnControllerChanged Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"))
	// Only handle releasing, restart is a better time to handle binding
	if (ensure(Pawn && Pawn == GetOwner()) && OldController)
	{
		CleanupInputComponent(OldController);
	}
}

void UGIPS_InputSystemComponent::CleanInputActionValueBindings()
{
	for (auto& Binding : InputActionValueBindings)
	{
		InputComponent->RemoveActionValueBinding(Binding.Value);
		UE_LOG(LogGIPS, Verbose, TEXT("Clean input action value binding for InputTag:{%s}"), *Binding.Key.ToString());
	}
	InputActionValueBindings.Empty();
}

void UGIPS_InputSystemComponent::SetupInputActionValueBindings()
{
	check(InputConfig);
	for (auto& Mapping : InputConfig->InputActionMappings)
	{
		if (Mapping.Value.bValueBinding)
		{
			FEnhancedInputActionValueBinding& Binding = InputComponent->BindActionValue(Mapping.Value.InputAction);
			int32 BindingIndex = InputComponent->GetActionValueBindings().Find(Binding);
			InputActionValueBindings.Emplace(Mapping.Key, BindingIndex);
			UE_LOG(LogGIPS, Verbose, TEXT("Setup input action value binding for InputTag:{%s} ad index:{%d}"), *Mapping.Key.ToString(), BindingIndex);
		}
	}
}

void UGIPS_InputSystemComponent::SetupInputComponent(UInputComponent* NewInputComponent)
{
	InputComponent = Cast<UEnhancedInputComponent>(NewInputComponent);

	if (ensureMsgf(InputComponent, TEXT("Project must use EnhancedInputComponent to support PlayerControlsComponent")))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();

		if (Subsystem && InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, InputPriority);
		}

		CleanInputActionValueBindings();

		SetupInputActionValueBindings();

		UE_LOG(LogGIPS, Verbose, TEXT("SetupInputComponent for Pawn/PC: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NONE"))
		OnSetupPlayerInputComponent(InputComponent);
		SetupInputComponentEvent.Broadcast(InputComponent);
	}
}

void UGIPS_InputSystemComponent::CleanupInputComponent(AController* OldController)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OldController);
	if (Subsystem && InputComponent)
	{
		OnCleanupPlayerInputComponent(InputComponent);
		CleanupInputComponentEvent.Broadcast(InputComponent);

		if (InputMappingContext)
		{
			Subsystem->RemoveMappingContext(InputMappingContext);
		}
		CleanInputActionValueBindings();
	}
	InputComponent = nullptr;
}

UEnhancedInputLocalPlayerSubsystem* UGIPS_InputSystemComponent::GetEnhancedInputSubsystem(AController* OldController) const
{
	if (OwnerType == EGIPS_OwnerType::Pawn && !GetOwner<APawn>())
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

void UGIPS_InputSystemComponent::BindInputActions()
{
	check(InputConfig);

	for (auto& Pair : InputConfig->InputActionMappings)
	{
		// Generic binding.
		InputComponent->BindAction(Pair.Value.InputAction, ETriggerEvent::Triggered, this, &ThisClass::InputActionCallback, Pair.Key, ETriggerEvent::Triggered);
		InputComponent->BindAction(Pair.Value.InputAction, ETriggerEvent::Started, this, &ThisClass::InputActionCallback, Pair.Key, ETriggerEvent::Started);
		InputComponent->BindAction(Pair.Value.InputAction, ETriggerEvent::Ongoing, this, &ThisClass::InputActionCallback, Pair.Key, ETriggerEvent::Ongoing);
		InputComponent->BindAction(Pair.Value.InputAction, ETriggerEvent::Completed, this, &ThisClass::InputActionCallback, Pair.Key, ETriggerEvent::Completed);
		InputComponent->BindAction(Pair.Value.InputAction, ETriggerEvent::Canceled, this, &ThisClass::InputActionCallback, Pair.Key, ETriggerEvent::Canceled);
	}
}

UGIPS_InputControlSetup* UGIPS_InputSystemComponent::GetCurrentInputSetup() const
{
	if (InputControlSetups.IsValidIndex(InputControlSetups.Num() - 1))
	{
		return InputControlSetups[InputControlSetups.Num() - 1];
	}
	return nullptr;
}

UGIPS_InputConfig* UGIPS_InputSystemComponent::GetInputConfig() const
{
	return InputConfig;
}

void UGIPS_InputSystemComponent::PushInputSetup(UGIPS_InputControlSetup* NewSetup)
{
	if (!InputControlSetups.Contains(NewSetup))
	{
		InputControlSetups.Push(NewSetup);
	}
}

void UGIPS_InputSystemComponent::PopInputSetup()
{
	if (InputControlSetups.Num() > 1)
	{
		InputControlSetups.Pop();
	}
}

bool UGIPS_InputSystemComponent::CheckInputAllowed(FGameplayTag InputTag, ETriggerEvent TriggerEvent)
{
	FInputActionInstance ActionData;
	return CheckInputAllowed(ActionData, InputTag, TriggerEvent);
}

bool UGIPS_InputSystemComponent::CheckInputAllowed(const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent)
{
	if (UGIPS_InputControlSetup* Setup = GetCurrentInputSetup())
	{
		return Setup->CheckInput(this, ActionData, InputTag, TriggerEvent);
	}
	return true;
}

void UGIPS_InputSystemComponent::InputActionCallback(const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent)
{
	if (InputTag.IsValid())
	{
		if (!bProcessingInputExternally && CheckInputAllowed(ActionData, InputTag, TriggerEvent))
		{
			ProcessInput(ActionData, InputTag, TriggerEvent);
		}
		LastInputActionValues.Emplace(InputTag, ActionData.GetValue());
		OnReceivedInput.Broadcast(ActionData, InputTag, TriggerEvent);
	}
}

void UGIPS_InputSystemComponent::ProcessInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (UGIPS_InputControlSetup* Setup = GetCurrentInputSetup())
	{
		Setup->HandleInput(this, ActionData, InputTag, TriggerEvent);
	}
}

UInputAction* UGIPS_InputSystemComponent::GetInputActionOfInputTag(FGameplayTag InputTag) const
{
	if (InputTag.IsValid() && InputConfig->InputActionMappings.Contains(InputTag))
		return InputConfig->InputActionMappings[InputTag].InputAction;

	return nullptr;
}

FInputActionValue UGIPS_InputSystemComponent::GetInputActionValueOfInputTag(FGameplayTag InputTag) const
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

FInputActionValue UGIPS_InputSystemComponent::GetLastInputActionValueOfInputTag(FGameplayTag InputTag) const
{
	if (InputTag.IsValid() && LastInputActionValues.Contains(InputTag))
	{
		return LastInputActionValues[InputTag];
	}

	return FInputActionValue();
}

void UGIPS_InputSystemComponent::RegisterPassedInputEntry(const FGIPS_BufferedInput& InputEntry)
{
	if (PassedInputEntries.Num() >= MaxInputEntriesNum)
	{
		PassedInputEntries.RemoveAtSwap(0);
	}
	PassedInputEntries.Add(InputEntry);
}

void UGIPS_InputSystemComponent::RegisterBlockedInputEntry(const FGIPS_BufferedInput& InputEntry)
{
	if (BlockedInputEntries.Num() >= MaxInputEntriesNum)
	{
		BlockedInputEntries.RemoveAtSwap(0);
	}
	BlockedInputEntries.Add(InputEntry);
}

void UGIPS_InputSystemComponent::RegisterBufferedInputEntry(const FGIPS_BufferedInput& InputEntry)
{
	if (BufferedInputEntries.Num() >= MaxInputEntriesNum)
	{
		BufferedInputEntries.RemoveAtSwap(0);
	}
	BufferedInputEntries.Add(InputEntry);
}

#pragma region InputBuffer

bool UGIPS_InputSystemComponent::TrySaveInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
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

void UGIPS_InputSystemComponent::FireBufferedInput()
{
	ProcessInput(CurrentBufferedInput.ActionData, CurrentBufferedInput.InputTag, CurrentBufferedInput.TriggerEvent);
	OnFireBufferedInput.Broadcast(CurrentBufferedInput.ActionData, CurrentBufferedInput.InputTag, CurrentBufferedInput.TriggerEvent);
	ResetBufferedInput();
	CloseActiveInputBufferWindows();
}

void UGIPS_InputSystemComponent::OpenInputBufferWindow(FGameplayTag BufferWindowName)
{
	if (!BufferWindowName.IsValid())
	{
		UE_LOG(LogGIPS, Warning, TEXT("Passed invalid buffer name to OpenInputBufferWindow!"));
		return;
	}

	if (ActiveBufferWindows.Contains(BufferWindowName))
	{
		UE_LOG(LogGIPS, Warning, TEXT("Can't Open buffer window(%s) as it already active!"), *BufferWindowName.ToString());
		return;
	}

	if (!ActiveBufferWindows.Contains(BufferWindowName))
	{
		if (const FGIPS_InputBufferWindow* Window = InputConfig->InputBufferDefinitions.FindByKey(BufferWindowName))
		{
			ActiveBufferWindows.FindOrAdd(BufferWindowName);
			UE_LOG(LogGIPS, Verbose, TEXT("Open buffer window:%s"), *BufferWindowName.ToString());
			InputBufferWindowStateChangedEvent.Broadcast(BufferWindowName, true);
		}
	}
}

void UGIPS_InputSystemComponent::CloseInputBufferWindow(FGameplayTag BufferWindowName)
{
	if (ActiveBufferWindows.Contains(BufferWindowName))
	{
		CurrentBufferedInput = ActiveBufferWindows[BufferWindowName];
		if (CurrentBufferedInput.InputTag.IsValid())
		{
			UE_LOG(LogGIPS, Verbose, TEXT("Fire buffered input(:%s,TriggerEvent:%s) from Window(%s)"), *CurrentBufferedInput.InputTag.ToString(),
			       *UGIPS_InputFunctionLibrary::GetTriggerEventString(CurrentBufferedInput.TriggerEvent), *BufferWindowName.ToString());
			FireBufferedInput();
		}
		ActiveBufferWindows.Remove(BufferWindowName);
		UE_LOG(LogGIPS, Verbose, TEXT("Close buffer window:%s"), *BufferWindowName.ToString());
		InputBufferWindowStateChangedEvent.Broadcast(BufferWindowName, false);
	}
}

void UGIPS_InputSystemComponent::CloseActiveInputBufferWindows()
{
	ActiveBufferWindows.Empty();
}

FGIPS_BufferedInput UGIPS_InputSystemComponent::GetLastBufferedInput() const
{
	return LastBufferedInput;
}

TMap<FGameplayTag, FGIPS_BufferedInput> UGIPS_InputSystemComponent::GetActiveBufferWindows() const
{
	return ActiveBufferWindows;
}

void UGIPS_InputSystemComponent::ResetBufferedInput()
{
	LastBufferedInput = CurrentBufferedInput;
	CurrentBufferedInput = FGIPS_BufferedInput();
}

bool UGIPS_InputSystemComponent::TrySaveAsBufferedInput(const FGameplayTag BufferWindowName, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent)
{
	if (!ActiveBufferWindows.Contains(BufferWindowName))
		return false;

	FGIPS_BufferedInput& BufferedInput = ActiveBufferWindows[BufferWindowName];
	const FGIPS_InputBufferWindow* Definition = InputConfig->InputBufferDefinitions.FindByKey(BufferWindowName);

	if (Definition == nullptr)
		return false;

	const int32 AllowedInputIndex = Definition->IndexOfAllowedInput(InputTag, TriggerEvent);

	if (AllowedInputIndex == INDEX_NONE)
		return false;

	// Instance buffering.
	if (Definition->BufferType == EGIPS_InputBufferType::Instant)
	{
		BufferedInput.InputTag = InputTag;
		BufferedInput.ActionData = ActionData;
		BufferedInput.TriggerEvent = TriggerEvent;

		CurrentBufferedInput = BufferedInput;
		UE_LOG(LogGIPS, Verbose, TEXT("Instantly fire buffered input(%s,TriggerEvent:%s) from Window(%s)"), *InputTag.ToString(), *UGIPS_InputFunctionLibrary::GetTriggerEventString(TriggerEvent),
		       *BufferWindowName.ToString());
		FireBufferedInput();
		ActiveBufferWindows.Remove(BufferWindowName);

		return true;
	}

	if (BufferedInput.InputTag.IsValid() && Definition->BufferType == EGIPS_InputBufferType::HighestPriority)
	{
		const int32 ExistingInputIndex = Definition->IndexOfAllowedInput(BufferedInput.InputTag, BufferedInput.TriggerEvent);
		if (ExistingInputIndex != INDEX_NONE && AllowedInputIndex < ExistingInputIndex)
		{
			UE_LOG(LogGIPS, Verbose, TEXT("Record new buffered input(%s,TriggerEvent:%s) in Window(%s),Before was input(%s,TriggerEvent:%s)"), *InputTag.ToString(),
			       *UGIPS_InputFunctionLibrary::GetTriggerEventString(TriggerEvent), *BufferWindowName.ToString(),
			       *BufferedInput.InputTag.ToString(),
			       *UGIPS_InputFunctionLibrary::GetTriggerEventString(BufferedInput.TriggerEvent));
			BufferedInput.InputTag = InputTag;
			BufferedInput.ActionData = ActionData;
			BufferedInput.TriggerEvent = TriggerEvent;
			return true;
		}
	}

	UE_LOG(LogGIPS, Verbose, TEXT("Record buffered input(%s,TriggerEvent:%s) in Window(%s)"), *InputTag.ToString(),
	       *UGIPS_InputFunctionLibrary::GetTriggerEventString(TriggerEvent), *BufferWindowName.ToString());
	BufferedInput.InputTag = InputTag;
	BufferedInput.ActionData = ActionData;
	BufferedInput.TriggerEvent = TriggerEvent;
	return true;
}

#pragma endregion

#pragma region DataValidation
#if WITH_EDITOR
EDataValidationResult UGIPS_InputSystemComponent::IsDataValid(FDataValidationContext& Context) const
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
