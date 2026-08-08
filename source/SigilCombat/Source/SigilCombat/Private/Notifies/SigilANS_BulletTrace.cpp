// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Notifies/SigilANS_BulletTrace.h"

#include "CombatFlow/SigilAttackRequest.h"
#include "UObject/ObjectSaveContext.h"


USigilANS_BulletTrace::USigilANS_BulletTrace(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
	AttackRequest = ObjectInitializer.CreateDefaultSubobject<USigilAttackRequest_Bullet>(this, TEXT("AttackRequest"));
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult USigilANS_BulletTrace::IsDataValid(FDataValidationContext& Context) const
{
	if (AttackRequest && AttackRequest->bRequireTargeting && AttackRequest->TargetingPreset == nullptr)
	{
		Context.AddError(FText::FromString(TEXT("TargetingPreset is required!")));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}

void USigilANS_BulletTrace::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}
#endif
