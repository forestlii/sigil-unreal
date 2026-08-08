// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GCS_CombatTeamAgentInterface.h"

#include "GCS_LogChannels.h"


// Add default functionality here for any IGCS_CombatTeamAgentInterface functions that are not pure virtual.
void IGCS_CombatTeamAgentInterface::SetCombatTeamId_Implementation(FGenericTeamId NewTeamId)
{
	if (IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>( _getUObject()))
	{
		TeamAgentInterface->SetGenericTeamId(NewTeamId);
	}
}

FGenericTeamId IGCS_CombatTeamAgentInterface::GetCombatTeamId_Implementation() const
{
	if (IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>( _getUObject()))
	{
		return TeamAgentInterface->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

void IGCS_CombatTeamAgentInterface::ConditionalBroadcastTeamChanged(TScriptInterface<IGCS_CombatTeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	if (OldTeamID != NewTeamID)
	{
		UObject* ThisObj = This.GetObject();
		UE_LOG(LogGCS, Verbose, TEXT("[%s] %s assigned team %d"), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamID.GetId());
		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamID, NewTeamID);
	}
}
