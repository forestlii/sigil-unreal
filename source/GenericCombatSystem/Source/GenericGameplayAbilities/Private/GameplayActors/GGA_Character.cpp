// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GameplayActors/GGA_Character.h"

#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"


AGGA_Character::AGGA_Character(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGGA_Character::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AGGA_Character::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
}

void AGGA_Character::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AGGA_Character::OnRep_Controller()
{
	Super::OnRep_Controller();

	ReceivePlayerController();
}

void AGGA_Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ReceivePlayerState();
}

UAbilitySystemComponent* AGGA_Character::GetAbilitySystemComponent() const
{
	if (UAbilitySystemComponent* BpProvidedASC = CustomGetAbilitySystemComponent())
	{
		return BpProvidedASC;
	}
	return nullptr;
}

void AGGA_Character::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
}


// Called to bind functionality to input
void AGGA_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
