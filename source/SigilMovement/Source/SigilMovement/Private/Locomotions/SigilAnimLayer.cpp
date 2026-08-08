// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Locomotions/SigilAnimLayer.h"
#include "GameFramework/Pawn.h"
#include "SigilMovementSystemComponent.h"
#include "Locomotions/SigilMainAnimInstance.h"
#include "Misc/DataValidation.h"
#include "Settings/SigilSettingObjectLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAnimLayer)


bool USigilAnimLayerSetting::GetOverrideAnimLayerClass_Implementation(TSubclassOf<USigilAnimLayer>& OutLayerClass) const
{
	return false;
}

bool USigilAnimLayerSetting::K2_IsDataValid_Implementation(FText& ErrorText) const
{
	return true;
}

#if WITH_EDITOR
EDataValidationResult USigilAnimLayerSetting::IsDataValid(class FDataValidationContext& Context) const
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


void USigilAnimLayer::OnLinked_Implementation()
{
	// make sure to get reference to parent when linked.
	if (!Parent.IsValid())
	{
		Parent = Cast<USigilMainAnimInstance>(GetSkelMeshComponent()->GetAnimInstance());
		checkf(Parent!=nullptr, TEXT("Parent is not SigilMainAnimInstance!"));
	}
	if (!PawnOwner)
	{
		PawnOwner = Cast<APawn>(GetOwningActor());
		checkf(PawnOwner!=nullptr, TEXT("PawnOwner is not valid!"));
	}
	if (!MSC)
	{
		MSC = PawnOwner->FindComponentByClass<USigilMovementSystemComponent>();
		checkf(MSC!=nullptr, TEXT("Movement Sysytem Component is not valid!"));
	}

	if (!AnimStateNameToTagMapping.IsEmpty())
	{
		Parent->RegisterStateNameToTagMapping(this, AnimStateNameToTagMapping);
	}
}

void USigilAnimLayer::OnUnlinked_Implementation()
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


USigilAnimLayer::USigilAnimLayer()
{
	bUseMainInstanceMontageEvaluationData = true;
}

USigilMainAnimInstance* USigilAnimLayer::GetParent() const
{
	return Parent.Get();
}

void USigilAnimLayer::ApplySetting_Implementation(const USigilAnimLayerSetting* Setting)
{
}

void USigilAnimLayer::ResetSetting_Implementation()
{
}

void USigilAnimLayer::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Parent = Cast<USigilMainAnimInstance>(GetSkelMeshComponent()->GetAnimInstance());

	PawnOwner = Cast<APawn>(GetOwningActor());

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		// Use default objects for editor preview.

		if (!Parent.IsValid())
		{
			Parent = GetMutableDefault<USigilMainAnimInstance>();
		}

		if (!IsValid(PawnOwner))
		{
			PawnOwner = GetMutableDefault<APawn>();
		}
	}
#endif
}

void USigilAnimLayer::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	ensure(PawnOwner);

	MSC = PawnOwner->FindComponentByClass<USigilMovementSystemComponent>();
}
