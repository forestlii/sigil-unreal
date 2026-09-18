// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilInputTestTypes.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

void USigilInputTestProcessor::HandleInputStarted_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
	Record(IC, InputTag, TEXT("Started"));
}

void USigilInputTestProcessor::HandleInputCompleted_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
	Record(IC, InputTag, TEXT("Completed"));
}

void USigilInputTestProcessor::HandleInputCanceled_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const
{
	Record(IC, InputTag, TEXT("Canceled"));
}

void USigilInputTestProcessor::Record(const USigilInputSystemComponent* IC, const FGameplayTag& InputTag, const TCHAR* EventName) const
{
	const APawn* Pawn = IC ? IC->GetControlledPawn() : nullptr;
	Log.Add(FString::Printf(TEXT("%s|%s|%s"), *InputTag.ToString(), EventName, Pawn ? *Pawn->GetName() : TEXT("None")));
}

USigilInputTestComponent::USigilInputTestComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USigilInputTestComponent::NeutralizeGameplayReceiver(APawn* OldReceiver, const FGameplayTagContainer& HeldInputTags)
{
	++NeutralizeCalls;
	bRouteEnabledDuringNeutralize = IsGameplayRoutingEnabled();
	bOwnedContextPresentDuringNeutralize = false;
	if (const UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem())
	{
		bOwnedContextPresentDuringNeutralize = InputMappingContext && Subsystem->HasMappingContext(InputMappingContext);
	}
	LastNeutralizedReceiver = OldReceiver;
	LastNeutralizedTags = HeldInputTags;

	Super::NeutralizeGameplayReceiver(OldReceiver, HeldInputTags);
}

ASigilInputTestPlayerController::ASigilInputTestPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SigilInput = CreateDefaultSubobject<USigilInputTestComponent>(TEXT("SigilInput"));
}

void ASigilInputTestPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	++SetupInputComponentCalls;
	if (SigilInput)
	{
		SigilInput->BindPlayerControllerInput(InputComponent);
	}
}
