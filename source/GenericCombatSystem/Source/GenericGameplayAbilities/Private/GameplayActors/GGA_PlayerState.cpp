// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GameplayActors/GGA_PlayerState.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Components/PlayerStateComponent.h"

AGGA_PlayerState::AGGA_PlayerState(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UGGA_AbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AGGA_PlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AGGA_PlayerState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AGGA_PlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AGGA_PlayerState::Reset()
{
	TArray<UPlayerStateComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UPlayerStateComponent* Component : ModularComponents)
	{
		Component->Reset();
	}
	Super::Reset();
}

void AGGA_PlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
	ReceiveClientInitialize(C);
}

UAbilitySystemComponent* AGGA_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGGA_PlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	TInlineComponentArray<UPlayerStateComponent*> PlayerStateComponents;
	GetComponents(PlayerStateComponents);
	for (UPlayerStateComponent* SourcePSComp : PlayerStateComponents)
	{
		if (UPlayerStateComponent* TargetComp = Cast<UPlayerStateComponent>(static_cast<UObject*>(FindObjectWithOuter(PlayerState, SourcePSComp->GetClass(), SourcePSComp->GetFName()))))
		{
			SourcePSComp->CopyProperties(TargetComp);
		}
	}
}
