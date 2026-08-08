// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "GameplayActors/SigilCharacter.h"

#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"


ASigilCharacter::ASigilCharacter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASigilCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ASigilCharacter::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
}

void ASigilCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void ASigilCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	ReceivePlayerController();
}

void ASigilCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ReceivePlayerState();
}

UAbilitySystemComponent* ASigilCharacter::GetAbilitySystemComponent() const
{
	if (UAbilitySystemComponent* BpProvidedASC = CustomGetAbilitySystemComponent())
	{
		return BpProvidedASC;
	}
	return nullptr;
}

void ASigilCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
}


// Called to bind functionality to input
void ASigilCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
