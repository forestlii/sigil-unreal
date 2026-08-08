// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GCS_CombatTeamAgentComponent.h"
#include "GCS_LogChannels.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Sets default values for this component's properties
UGCS_CombatTeamAgentComponent::UGCS_CombatTeamAgentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	// ...
}

void UGCS_CombatTeamAgentComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CombatTeamId, SharedParams);
}

FGCS_CombatTeamIdChangedSignature *UGCS_CombatTeamAgentComponent::GetOnTeamIdChangedDelegate()
{
	return &OnTeamIdChangedEvent;
}

FGenericTeamId UGCS_CombatTeamAgentComponent::GetCombatTeamId_Implementation() const
{
	return CombatTeamId;
}

void UGCS_CombatTeamAgentComponent::SetCombatTeamId_Implementation(FGenericTeamId NewTeamId)
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
		UE_LOG(LogGCS, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
	}
}

void UGCS_CombatTeamAgentComponent::OnRep_CombatTeamId(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, CombatTeamId);
}

// Called when the game starts
void UGCS_CombatTeamAgentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void UGCS_CombatTeamAgentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
