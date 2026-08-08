// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Targeting/Selections/SigilTargetingSelectionTask_TraceExt_BindShape.h"

#include "SigilCombatLogChannels.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ShapeComponent.h"
#include "Misc/DataValidation.h"
#include "Targeting/SigilTargetingSourceInterface.h"

float USigilTargetingSelectionTask_TraceExt_BindShape::GetSweptTraceRadius_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	float BaseValue = -1;

	if (USphereComponent* Shape = Cast<USphereComponent>(GetTraceShape(TargetingHandle)))
	{
		BaseValue = Shape->GetScaledSphereRadius();
	}
	if (const UCapsuleComponent* Shape = Cast<UCapsuleComponent>(GetTraceShape(TargetingHandle)))
	{
		BaseValue = Shape->GetScaledCapsuleRadius();
	}

	float Value = Super::GetSweptTraceRadius_Implementation(TargetingHandle);

	if (BaseValue < 0)
	{
		return Value;
	}

	if (SweptTraceRadiusModType == ESigilTraceDataModifyType::Add)
	{
		return BaseValue + Value;
	}

	if (SweptTraceRadiusModType == ESigilTraceDataModifyType::Multiply)
	{
		return BaseValue * Value;
	}
	return BaseValue;
}

float USigilTargetingSelectionTask_TraceExt_BindShape::GetSweptTraceCapsuleHalfHeight_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	float BaseValue = -1;

	if (const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(GetTraceShape(TargetingHandle)))
	{
		BaseValue = Capsule->GetScaledCapsuleHalfHeight();
	}

	float Value = Super::GetSweptTraceCapsuleHalfHeight_Implementation(TargetingHandle);

	if (BaseValue < 0)
	{
		return Value;
	}

	if (SweptTraceCapsuleHalfHeightModType == ESigilTraceDataModifyType::Add)
	{
		return BaseValue + Value;
	}

	if (SweptTraceCapsuleHalfHeightModType == ESigilTraceDataModifyType::Multiply)
	{
		return BaseValue * Value;
	}
	return BaseValue;
}

FVector USigilTargetingSelectionTask_TraceExt_BindShape::GetSweptTraceBoxHalfExtents_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	FVector BaseValue = FVector::ZeroVector;

	if (const UBoxComponent* Shape = Cast<UBoxComponent>(GetTraceShape(TargetingHandle)))
	{
		BaseValue = Shape->GetScaledBoxExtent();
	}

	FVector Value = Super::GetSweptTraceBoxHalfExtents_Implementation(TargetingHandle);

	if (BaseValue == FVector::ZeroVector)
	{
		return Value;
	}

	if (SweptTraceBoxHalfExtentModType == ESigilTraceDataModifyType::Add)
	{
		return BaseValue + Value;
	}

	if (SweptTraceBoxHalfExtentModType == ESigilTraceDataModifyType::Multiply)
	{
		return BaseValue * Value;
	}
	return BaseValue;
}

UShapeComponent* USigilTargetingSelectionTask_TraceExt_BindShape::GetTraceShape(const FTargetingRequestHandle& TargetingHandle) const
{
	UShapeComponent* ShapeComponent = nullptr;
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceObject)
		{
			if (SourceContext->SourceObject->GetClass()->ImplementsInterface(USigilTargetingSourceInterface::StaticClass()))
			{
				if (ISigilTargetingSourceInterface::Execute_GetTraceShape(SourceContext->SourceObject, ShapeComponent))
				{
					return ShapeComponent;
				}
				else
				{
					UE_LOG(LogSigilCombat, VeryVerbose, TEXT("Source Object(%s) doesn't provide valid ShapeComponent! TargetingPreset:%s"),
					       *SourceContext->SourceObject->GetName(), *GetOuter()->GetName());
				}
			}
			else
			{
				UE_LOG(LogSigilCombat, VeryVerbose, TEXT("Source Object(%s) doesn't implements SigilTargetingSourceInterface.! TargetingPreset:%s"),
				       *SourceContext->SourceObject->GetName(), *GetOuter()->GetName());
			}
		}
		else
		{
			UE_LOG(LogSigilCombat, Error, TEXT("No valid Context Source Object found! TargetingPreset:%s"), *GetOuter()->GetName());
		}
	}
	return nullptr;
}

FRotator USigilTargetingSelectionTask_TraceExt_BindShape::GetSweptTraceRotation_Implementation(const FTargetingRequestHandle& TargetingHandle) const
{
	if (const FTargetingSourceContext* SourceContext = FTargetingSourceContext::Find(TargetingHandle))
	{
		if (SourceContext->SourceObject && SourceContext->SourceObject->GetClass()->ImplementsInterface(USigilTargetingSourceInterface::StaticClass()))
		{
			FRotator TraceRotation;
			if (ISigilTargetingSourceInterface::Execute_GetSweptTraceRotation(SourceContext->SourceObject, TraceRotation))
			{
				return TraceRotation;
			}
		}
	}
	return Super::GetSweptTraceRotation_Implementation(TargetingHandle);
}

#if WITH_EDITORONLY_DATA
EDataValidationResult USigilTargetingSelectionTask_TraceExt_BindShape::IsDataValid(class FDataValidationContext& Context) const
{
	if (TraceType == ETargetingTraceType::Line)
	{
		FString Txt = FString::Format(TEXT("TraceType == Line is not allowed in this type of task:{0} "), {GetClass()->GetName()});
		Context.AddError(FText::FromString(Txt));
	}
	if (bUseContextLocationAsSourceLocation)
	{
		FString Txt = FString::Format(TEXT("bUseContextLocationAsSourceLocation is not allowed in this type of task:{0} "), {GetClass()->GetName()});
		Context.AddError(FText::FromString(Txt));
	}
	return Super::IsDataValid(Context);
}
#endif
