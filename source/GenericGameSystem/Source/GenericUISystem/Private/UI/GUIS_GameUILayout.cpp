// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UI/GUIS_GameUILayout.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GUIS_LogChannels.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_GameUILayout)

class UObject;

UGUIS_GameUILayout::UGUIS_GameUILayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGUIS_GameUILayout::SetIsDormant(bool InDormant)
{
	if (bIsDormant != InDormant)
	{
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const int32 PlayerId = LP ? LP->GetControllerId() : -1;
		const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* NewDormancyStr = InDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
		UE_LOG(LogGUIS, Display, TEXT("%s PrimaryGameLayout Dormancy changed for [%d] from [%s] to [%s]"), PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

		bIsDormant = InDormant;
		OnIsDormantChanged();
	}
}

void UGUIS_GameUILayout::OnIsDormantChanged()
{
	//@TODO NDarnell Determine what to do with dormancy, in the past we treated dormancy as a way to shutoff rendering
	//and the view for the other local players when we force multiple players to use the player view of a single player.

	//if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer<ULocalPlayer>())
	//{
	//	// When the root layout is dormant, we don't want to render anything from the owner's view either
	//	LocalPlayer->SetIsPlayerViewEnabled(!bIsDormant);
	//}

	//SetVisibility(bIsDormant ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);

	//OnLayoutDormancyChanged().Broadcast(bIsDormant);
}

void UGUIS_GameUILayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		LayerWidget->OnTransitioningChanged.AddUObject(this, &UGUIS_GameUILayout::OnWidgetStackTransitioning);
		// TODO: Consider allowing a transition duration, we currently set it to 0, because if it's not 0, the
		//       transition effect will cause focus to not transition properly to the new widgets when using
		//       gamepad always.
		LayerWidget->SetTransitionDuration(0.0);

		Layers.Add(LayerTag, LayerWidget);
	}
}

void UGUIS_GameUILayout::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	if (bIsTransitioning)
	{
		const FName SuspendToken = UGUIS_GameUIFunctionLibrary::SuspendInputForPlayer(GetOwningLocalPlayer(), TEXT("GlobalStackTransion"));
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			const FName SuspendToken = SuspendInputTokens.Pop();
			UGUIS_GameUIFunctionLibrary::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
		}
	}
}

void UGUIS_GameUILayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	// We're not sure what layer the widget is on so go searching.
	for (const auto& LayerKVP : Layers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* UGUIS_GameUILayout::GetLayerWidget(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}
