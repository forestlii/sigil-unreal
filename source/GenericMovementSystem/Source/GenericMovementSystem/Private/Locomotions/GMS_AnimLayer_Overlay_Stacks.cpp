// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Locomotions/GMS_AnimLayer_Overlay_Stacks.h"
#include "Locomotions/GMS_MainAnimInstance.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_AnimLayer_Overlay_Stacks)

void UGMS_AnimLayer_Overlay_Stack::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	if (OverlayStackStates.IsEmpty())
	{
		OverlayStackStates.AddDefaulted(MaxLayers);
	}
}

void UGMS_AnimLayer_Overlay_Stack::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UGMS_AnimLayer_Overlay_Stack::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	RefreshRelevance();
	RefreshBlend(DeltaSeconds);
}

void UGMS_AnimLayer_Overlay_Stack::ApplySetting_Implementation(const UGMS_AnimLayerSetting* Setting)
{
	check(IsValid(Setting))
	//setting changes or invalid, reset.
	if (PrevSetting != Setting || PrevOverlayMode != GetParent()->OverlayMode)
	{
		PrevOverlayMode = FGameplayTag::EmptyTag;
		ResetSetting();
		PrevSetting = nullptr;
	}

	// Apply new data to anim instance.
	if (const UGMS_AnimLayerSetting_Overlay_Stack* DS = Cast<UGMS_AnimLayerSetting_Overlay_Stack>(Setting))
	{
		//same setting and overlay.
		if (PrevSetting == DS && PrevOverlayMode == GetParent()->OverlayMode)
		{
			return;
		}
		
		PrevSetting = DS;
		PrevOverlayMode = GetParent()->OverlayMode;
		
		if (OverlayStackStates.IsEmpty())
		{
			OverlayStackStates.AddDefaulted(MaxLayers);
		}

		if (!DS->AcceleratedOverlayModes.Contains(GetParent()->OverlayMode))
			return;

		const TArray<FGMS_AnimData_StackedOverlays>& Stacks = DS->AcceleratedOverlayModes[GetParent()->OverlayMode].StackedOverlays;

		for (int32 i = 0; i < OverlayStackStates.Num(); i++)
		{
			if (Stacks.IsValidIndex(i))
			{
				OverlayStacks.EmplaceAt(i, Stacks[i]);
			}
			else
			{
				OverlayStacks.EmplaceAt(i, FGMS_AnimData_StackedOverlays());
			}
		}
	}
}

void UGMS_AnimLayer_Overlay_Stack::ResetSetting_Implementation()
{
	OverlayStacks.Reset(MaxLayers);
	for (FGMS_OverlayStackState& OverlayStackState : OverlayStackStates)
	{
		OverlayStackState.Overlay.bValid = false;
	}
}

void UGMS_AnimLayer_Overlay_Stack::RefreshRelevance()
{
	if (IsValid(GetParent()))
	{
		for (int32 i = 0; i < OverlayStackStates.Num(); i++)
		{
			if (OverlayStacks.IsValidIndex(i))
			{
				OverlayStackStates[i].bRelevant = GetParent()->NodeRelevanceTags.HasAny(OverlayStacks[i].TargetAnimNodes);
			}
			else
			{
				OverlayStackStates[i].bRelevant = false;
			}
		}
	}
}

void UGMS_AnimLayer_Overlay_Stack::RefreshBlend(float DeltaSeconds)
{
	if (IsValid(GetParent()))
	{
		const FGameplayTagContainer& Tags = GetParent()->OwnedTags;

		//Refresh current overlay.
		for (int32 i = 0; i < OverlayStackStates.Num(); i++)
		{
			int32 foundJ = INDEX_NONE;

			if (OverlayStacks.IsValidIndex(i))
			{
				for (int32 j = 0; j < OverlayStacks[i].Overlays.Num(); j++)
				{
					const FGMS_AnimData_Overlay& Overlay = OverlayStacks[i].Overlays[j];
					if (Overlay.bValid && (Overlay.TagQuery.IsEmpty() || Overlay.TagQuery.Matches(Tags)))
					{
						foundJ = j;
						break;
					}
				}
			}

			if (foundJ != INDEX_NONE)
			{
				if (OverlayStacks[i].Overlays[foundJ] != OverlayStackStates[i].Overlay)
				{
					OverlayStackStates[i].Overlay = OverlayStacks[i].Overlays[foundJ];
					OverlayStackStates[i].BlendOutSpeed = OverlayStacks[i].Overlays[foundJ].BlendOutSpeed;
				}
			}
			else
			{
				OverlayStackStates[i].Overlay.bValid = false;
			}
		}

		//Refresh blends.
		for (int32 i = 0; i < OverlayStackStates.Num(); i++)
		{
			FGMS_OverlayStackState& CurrentState = OverlayStackStates[i];
			if (CurrentState.Overlay.bValid && CurrentState.bRelevant)
			{
				if (CurrentState.Overlay.BlendInSpeed > 0)
				{
					CurrentState.BlendWeight = FMath::FInterpTo(CurrentState.BlendWeight, CurrentState.Overlay.BlendWeight, DeltaSeconds, CurrentState.Overlay.BlendInSpeed);
				}else
				{
					CurrentState.BlendWeight = CurrentState.Overlay.BlendWeight;
				}
			}
			else
			{
				if (CurrentState.BlendOutSpeed > 0)
				{
					CurrentState.BlendWeight = FMath::FInterpTo(CurrentState.BlendWeight, 0.0f, DeltaSeconds,  CurrentState.BlendOutSpeed);
				}else
				{
					CurrentState.BlendWeight = 0.0f;
				}
			}
		}
	}
}


#if WITH_EDITORONLY_DATA

void UGMS_AnimLayerSetting_Overlay_Stack::PreSave(FObjectPreSaveContext SaveContext)
{
	AcceleratedOverlayModes.Empty();
	for (FGMS_StackedOverlaySetting& Mode : OverlayModes)
	{
		for (FGMS_AnimData_StackedOverlays& Stack : Mode.StackedOverlays)
		{
			for (FGMS_AnimData_Overlay& Overlay : Stack.Overlays)
			{
				Overlay.Validate();
			}
			Stack.EditorFriendlyName = Stack.TargetAnimNodes.IsEmpty()
				                           ? TEXT("Invalid Overlay")
				                           : FString::Format(TEXT("Play overlay for states:[{0}]"), {Stack.TargetAnimNodes.ToStringSimple(false).Replace(TEXT("GMS.SM."),TEXT(""))});
		}
		AcceleratedOverlayModes.Emplace(Mode.Tag, Mode);
	}

	Super::PreSave(SaveContext);
}

#endif
