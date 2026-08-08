// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GameplayActors/GGA_GameState.h"

#include "GGA_AbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/GameStateComponent.h"
#include "Containers/Array.h"

// #include UE_INLINE_GENERATED_CPP_BY_NAME(ModularGameState)

AGGA_GameStateBase::AGGA_GameStateBase(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UGGA_AbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AGGA_GameStateBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(/*Owner=*/ this, /*Avatar=*/ this);
	}
}

UAbilitySystemComponent* AGGA_GameStateBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGGA_GameStateBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AGGA_GameStateBase::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AGGA_GameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}


AGGA_GameState::AGGA_GameState(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UGGA_AbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}



UAbilitySystemComponent* AGGA_GameState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGGA_GameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AGGA_GameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(/*Owner=*/ this, /*Avatar=*/ this);
	}
}

void AGGA_GameState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AGGA_GameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AGGA_GameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	TArray<UGameStateComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UGameStateComponent* Component : ModularComponents)
	{
		Component->HandleMatchHasStarted();
	}
}

