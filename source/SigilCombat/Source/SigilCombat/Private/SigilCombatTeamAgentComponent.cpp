// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilCombatTeamAgentComponent.h"
#include "SigilCombatLogChannels.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Sets default values for this component's properties
USigilCombatTeamAgentComponent::USigilCombatTeamAgentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	// ...
}

void USigilCombatTeamAgentComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CombatTeamId, SharedParams);
}

FSigilCombatTeamIdChangedSignature *USigilCombatTeamAgentComponent::GetOnTeamIdChangedDelegate()
{
	return &OnTeamIdChangedEvent;
}

FGenericTeamId USigilCombatTeamAgentComponent::GetCombatTeamId_Implementation() const
{
	return CombatTeamId;
}

void USigilCombatTeamAgentComponent::SetCombatTeamId_Implementation(FGenericTeamId NewTeamId)
{
	if (GetOwner()->HasAuthority())
	{
		const FGenericTeamId OldTeamID = CombatTeamId;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CombatTeamId, this);
		CombatTeamId = NewTeamId;
		if (bAssignTeamIdToController)
		{
			if (APawn *Pawn = Cast<APawn>(GetOwner()))
			{
				if (IGenericTeamAgentInterface *AgentInterface = Cast<IGenericTeamAgentInterface>(Pawn->GetController()))
				{
					AgentInterface->SetGenericTeamId(NewTeamId);
				}
			}
		}

		ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamId);
	}
	else
	{
		UE_LOG(LogSigilCombat, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
	}
}

void USigilCombatTeamAgentComponent::OnRep_CombatTeamId(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, CombatTeamId);
}

// Called when the game starts
void USigilCombatTeamAgentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void USigilCombatTeamAgentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
