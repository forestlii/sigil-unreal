// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Locomotions/GMS_AnimLayer.h"
#include "GameFramework/Pawn.h"
#include "GMS_MovementSystemComponent.h"
#include "Locomotions/GMS_MainAnimInstance.h"
#include "Misc/DataValidation.h"
#include "Settings/GMS_SettingObjectLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimLayer)


bool UGMS_AnimLayerSetting::GetOverrideAnimLayerClass_Implementation(TSubclassOf<UGMS_AnimLayer>& OutLayerClass) const
{
	return false;
}

bool UGMS_AnimLayerSetting::K2_IsDataValid_Implementation(FText& ErrorText) const
{
	return true;
}

#if WITH_EDITOR
EDataValidationResult UGMS_AnimLayerSetting::IsDataValid(class FDataValidationContext& Context) const
{
	FText ErrorText;
	if (!IsTemplate() && !K2_IsDataValid(ErrorText))
	{
		Context.AddError(ErrorText);
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif


void UGMS_AnimLayer::OnLinked_Implementation()
{
	// make sure to get reference to parent when linked.
	if (!Parent.IsValid())
	{
		Parent = Cast<UGMS_MainAnimInstance>(GetSkelMeshComponent()->GetAnimInstance());
		checkf(Parent!=nullptr, TEXT("Parent is not GMS_MainAnimInstance!"));
	}
	if (!PawnOwner)
	{
		PawnOwner = Cast<APawn>(GetOwningActor());
		checkf(PawnOwner!=nullptr, TEXT("PawnOwner is not valid!"));
	}
	if (!MSC)
	{
		MSC = PawnOwner->FindComponentByClass<UGMS_MovementSystemComponent>();
		checkf(MSC!=nullptr, TEXT("Movement Sysytem Component is not valid!"));
	}

	if (!AnimStateNameToTagMapping.IsEmpty())
	{
		Parent->RegisterStateNameToTagMapping(this, AnimStateNameToTagMapping);
	}
}

void UGMS_AnimLayer::OnUnlinked_Implementation()
{
	if (Parent.IsValid())
	{
		if (!AnimStateNameToTagMapping.IsEmpty())
		{
			Parent->UnregisterStateNameToTagMapping(this);
		}
		Parent = nullptr;
	}
}


UGMS_AnimLayer::UGMS_AnimLayer()
{
	bUseMainInstanceMontageEvaluationData = true;
}

UGMS_MainAnimInstance* UGMS_AnimLayer::GetParent() const
{
	return Parent.Get();
}

void UGMS_AnimLayer::ApplySetting_Implementation(const UGMS_AnimLayerSetting* Setting)
{
}

void UGMS_AnimLayer::ResetSetting_Implementation()
{
}

void UGMS_AnimLayer::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Parent = Cast<UGMS_MainAnimInstance>(GetSkelMeshComponent()->GetAnimInstance());

	PawnOwner = Cast<APawn>(GetOwningActor());

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		// Use default objects for editor preview.

		if (!Parent.IsValid())
		{
			Parent = GetMutableDefault<UGMS_MainAnimInstance>();
		}

		if (!IsValid(PawnOwner))
		{
			PawnOwner = GetMutableDefault<APawn>();
		}
	}
#endif
}

void UGMS_AnimLayer::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	ensure(PawnOwner);

	MSC = PawnOwner->FindComponentByClass<UGMS_MovementSystemComponent>();
}
