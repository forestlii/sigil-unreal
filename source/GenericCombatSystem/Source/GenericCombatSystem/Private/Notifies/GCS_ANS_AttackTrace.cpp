// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Notifies/GCS_ANS_AttackTrace.h"

#include "CombatFlow/GCS_AttackRequest.h"
#include "UObject/ObjectSaveContext.h"


UGCS_ANS_AttackTrace::UGCS_ANS_AttackTrace(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
	AttackRequest = ObjectInitializer.CreateDefaultSubobject<UGCS_AttackRequest_Melee>(this, TEXT("AttackRequest"));
}

void UGCS_ANS_AttackTrace::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITORONLY_DATA
void UGCS_ANS_AttackTrace::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}
#endif
