// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagAssetInterface.h"
#include "SigilInputChecker.h"
#include "SigilInputControlSetup.h"
#include "SigilInputProcessor.h"
#include "SigilInputSystemComponent.h"
#include "SigilInputTestTypes.generated.h"

/** Records every handled input as "Tag|Event|Pawn". 以"标签|事件|Pawn"记录每个处理的输入。 */
UCLASS(NotBlueprintable, NotBlueprintType, HideDropdown)
class USigilInputTestProcessor : public USigilInputProcessor
{
	GENERATED_BODY()

public:
	mutable TArray<FString> Log;

protected:
	virtual void HandleInputStarted_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const override;
	virtual void HandleInputCompleted_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const override;
	virtual void HandleInputCanceled_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const override;

private:
	void Record(const USigilInputSystemComponent* IC, const FGameplayTag& InputTag, const TCHAR* EventName) const;
};

UCLASS(NotBlueprintable, NotBlueprintType, HideDropdown)
class USigilInputTestControlSetup : public USigilInputControlSetup
{
	GENERATED_BODY()

public:
	void AddProcessor(USigilInputProcessor* Processor) { InputProcessors.Add(Processor); }
	void AddChecker(USigilInputChecker* Checker) { InputCheckers.Add(Checker); }
};

/** Exposes the protected subject tag query. 暴露受保护的主体标签查询。 */
UCLASS(NotBlueprintable, NotBlueprintType, HideDropdown)
class USigilInputTestTagChecker : public USigilInputChecker_TagRelationship
{
	GENERATED_BODY()

public:
	FGameplayTagContainer QueryTags(USigilInputSystemComponent* IC) const { return GetActorTags(IC); }
};

/** Exposes configuration and records the state seen by NeutralizeGameplayReceiver. 暴露配置并记录中和回调看到的状态。 */
UCLASS(NotBlueprintable, NotBlueprintType, HideDropdown)
class USigilInputTestComponent : public USigilInputSystemComponent
{
	GENERATED_BODY()

public:
	USigilInputTestComponent(const FObjectInitializer& ObjectInitializer);

	void Configure(USigilInputConfig* NewConfig, USigilInputControlSetup* NewSetup, UInputMappingContext* NewMappingContext)
	{
		InputConfig = NewConfig;
		InputControlSetups.Reset();
		if (NewSetup)
		{
			InputControlSetups.Add(NewSetup);
		}
		InputMappingContext = NewMappingContext;
	}

	UEnhancedInputComponent* GetBoundInputComponent() const { return InputComponent; }

	int32 NeutralizeCalls = 0;
	bool bRouteEnabledDuringNeutralize = false;
	bool bOwnedContextPresentDuringNeutralize = false;
	TWeakObjectPtr<APawn> LastNeutralizedReceiver;
	FGameplayTagContainer LastNeutralizedTags;

protected:
	virtual void NeutralizeGameplayReceiver(APawn* OldReceiver, const FGameplayTagContainer& HeldInputTags) override;
};

/** Mirrors the intended project PlayerController: binds from SetupInputComponent. 模拟目标项目PC：在SetupInputComponent中绑定。 */
UCLASS(NotBlueprintable, NotBlueprintType, HideDropdown)
class ASigilInputTestPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASigilInputTestPlayerController(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	TObjectPtr<USigilInputTestComponent> SigilInput;

	int32 SetupInputComponentCalls = 0;

protected:
	virtual void SetupInputComponent() override;
};

UCLASS(NotBlueprintable, NotBlueprintType, HideDropdown)
class ASigilInputTestPawn : public APawn, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = OwnedTags; }

	FGameplayTagContainer OwnedTags;
};
