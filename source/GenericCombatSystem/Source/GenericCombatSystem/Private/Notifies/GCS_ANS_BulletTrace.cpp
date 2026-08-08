// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Notifies/GCS_ANS_BulletTrace.h"

#include "CombatFlow/GCS_AttackRequest.h"
#include "UObject/ObjectSaveContext.h"


UGCS_ANS_BulletTrace::UGCS_ANS_BulletTrace(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
	AttackRequest = ObjectInitializer.CreateDefaultSubobject<UGCS_AttackRequest_Bullet>(this, TEXT("AttackRequest"));
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult UGCS_ANS_BulletTrace::IsDataValid(FDataValidationContext& Context) const
{
	if (AttackRequest && AttackRequest->bRequireTargeting && AttackRequest->TargetingPreset == nullptr)
	{
		Context.AddError(FText::FromString(TEXT("TargetingPreset is required!")));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}

void UGCS_ANS_BulletTrace::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}
#endif
