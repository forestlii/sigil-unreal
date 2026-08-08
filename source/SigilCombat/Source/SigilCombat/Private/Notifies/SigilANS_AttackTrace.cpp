// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Notifies/SigilANS_AttackTrace.h"

#include "CombatFlow/SigilAttackRequest.h"
#include "UObject/ObjectSaveContext.h"


USigilANS_AttackTrace::USigilANS_AttackTrace(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
	AttackRequest = ObjectInitializer.CreateDefaultSubobject<USigilAttackRequest_Melee>(this, TEXT("AttackRequest"));
}

void USigilANS_AttackTrace::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITORONLY_DATA
void USigilANS_AttackTrace::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}
#endif
