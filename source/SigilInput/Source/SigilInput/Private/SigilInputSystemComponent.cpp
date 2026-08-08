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
			if (ensure(PCOwner))
			{
				// TODO 支持放到PC上。
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
			APawn* PawnOwner = GetOwner<APawn>();
			PawnOwner->ReceiveRestartedDelegate.RemoveAll(this);
			PawnOwner->ReceiveControllerChangedDelegate.RemoveAll(this);
		}

		if (OwnerType == ESigilOwnerType::PC)
		{
			APlayerController* PCOwner = GetOwner<APlayerController>();
		}
	}

	Super::OnUnregister();
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
	for (auto& Binding : InputActionValueBindings)
	{
		InputComponent->RemoveActionValueBinding(Binding.Value);
		UE_LOG(LogSigilInput, Verbose, TEXT("Clean input action value binding for InputTag:{%s}"), *Binding.Key.ToString());
	}
	InputActionValueBindings.Empty();
}

void USigilInputSystemComponent::SetupInputActionValueBindings()
{
	check(InputConfig);
	for (auto& Mapping : InputConfig->InputActionMappings)
	{
		if (Mapping.Value.bValueBinding)
		{
			FEnhancedInputActionValueBinding& Binding = InputComponent->BindActionValue(Mapping.Value.InputAction);
			int32 BindingIndex = InputComponent->GetActionValueBindings().Find(Binding);
			InputActionValueBindings.Emplace(Mapping.Key, BindingIndex);
			UE_LOG(LogSigilInput, Verbose, TEXT("Setup input action value binding for InputTag:{%s} ad index:{%d}"), *Mapping.Key.ToString(), BindingIndex);
		}
	}
}

void USigilInputSystemComponent::SetupInputComponent(UInputComponent* NewInputComponent)
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

		UE_LOG(LogSigilInput, Verbose, TEXT("SetupInputComponent for Pawn/PC: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NONE"))
		OnSetupPlayerInputComponent(InputComponent);
		SetupInputComponentEvent.Broadcast(InputComponent);
	}
}

void USigilInputSystemComponent::CleanupInputComponent(AController* OldController)
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
