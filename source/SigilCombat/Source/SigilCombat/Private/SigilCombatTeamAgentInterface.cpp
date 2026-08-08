// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilCombatTeamAgentInterface.h"

#include "SigilCombatLogChannels.h"


// Add default functionality here for any ISigilCombatTeamAgentInterface functions that are not pure virtual.
void ISigilCombatTeamAgentInterface::SetCombatTeamId_Implementation(FGenericTeamId NewTeamId)
{
	if (IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>( _getUObject()))
	{
		TeamAgentInterface->SetGenericTeamId(NewTeamId);
	}
}

FGenericTeamId ISigilCombatTeamAgentInterface::GetCombatTeamId_Implementation() const
{
	if (IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>( _getUObject()))
	{
		return TeamAgentInterface->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

void ISigilCombatTeamAgentInterface::ConditionalBroadcastTeamChanged(TScriptInterface<ISigilCombatTeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	if (OldTeamID != NewTeamID)
	{
		UObject* ThisObj = This.GetObject();
		UE_LOG(LogSigilCombat, Verbose, TEXT("[%s] %s assigned team %d"), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamID.GetId());
		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamID, NewTeamID);
	}
}
