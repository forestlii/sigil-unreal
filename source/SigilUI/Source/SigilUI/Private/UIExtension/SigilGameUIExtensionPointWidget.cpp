// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UIExtension/SigilGameUIExtensionPointWidget.h"
#include "Widgets/SOverlay.h"
#include "TimerManager.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor/WidgetCompilerLog.h"
#include "Misc/UObjectToken.h"
#include "GameFramework/PlayerState.h"
#include "UI/Common/SigilUserWidgetInterface.h"
#include "SigilUILogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilGameUIExtensionPointWidget)

#define LOCTEXT_NAMESPACE "UIExtension"

/////////////////////////////////////////////////////
// USigilGameUIExtensionPointWidget

USigilGameUIExtensionPointWidget::USigilGameUIExtensionPointWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USigilGameUIExtensionPointWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	ResetExtensionPoint();

	Super::ReleaseSlateResources(bReleaseChildren);
}

TSharedRef<SWidget> USigilGameUIExtensionPointWidget::RebuildWidget()
{
	if (!IsDesignTime() && ExtensionPointTag.IsValid())
	{
		ResetExtensionPoint();
		RegisterExtensionPoint();

		// PlayerState may replicate a few frames after the controller: allow ~10 seconds of retries.
		PlayerStateRetriesLeft = 50;
		RegisterForPlayerStateIfReady();
	}

	if (IsDesignTime())
	{
		auto GetExtensionPointText = [this]()
		{
			return FText::Format(LOCTEXT("DesignTime_ExtensionPointLabel", "Extension Point\n{0}"), FText::FromName(ExtensionPointTag.GetTagName()));
		};

		TSharedRef<SOverlay> MessageBox = SNew(SOverlay);

		MessageBox->AddSlot()
		          .Padding(5.0f)
		          .HAlign(HAlign_Center)
		          .VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text_Lambda(GetExtensionPointText)
		];

		return MessageBox;
	}
	return Super::RebuildWidget();
}

void USigilGameUIExtensionPointWidget::RegisterForPlayerStateIfReady()
{
	StopWaitingForPlayerState();

	APlayerController* PC = GetOwningPlayer();
	APlayerState* PS = PC ? PC->GetPlayerState<APlayerState>() : nullptr;
	if (PC && PS)
	{
		RegisterExtensionPointForPlayerState(GetOwningLocalPlayer(), PS);
		return;
	}

	// Either the controller or its PlayerState has not arrived yet: retry on a bounded timer.
	if (PlayerStateRetriesLeft <= 0)
	{
		UE_LOG(LogSigilUI, Warning, TEXT("[%s] gave up waiting for a PlayerState; the PlayerState-context extension point was not registered."), *GetName());
		return;
	}
	if (UWorld* World = GetWorld())
	{
		--PlayerStateRetriesLeft;
		World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::OnCheckPlayerState, 0.2f, false);
	}
}

void USigilGameUIExtensionPointWidget::OnCheckPlayerState()
{
	TimerHandle.Invalidate();
	RegisterForPlayerStateIfReady();
}

void USigilGameUIExtensionPointWidget::StopWaitingForPlayerState()
{
	if (TimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
		TimerHandle.Invalidate();
	}
}

void USigilGameUIExtensionPointWidget::ResetExtensionPoint()
{
	StopWaitingForPlayerState();
	ResetInternal();

	ExtensionMapping.Reset();
	for (FSigilGameUIExtPointHandle& Handle : ExtensionPointHandles)
	{
		Handle.Unregister();
	}
	ExtensionPointHandles.Reset();
}

void USigilGameUIExtensionPointWidget::RegisterExtensionPoint()
{
	if (USigilExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<USigilExtensionSubsystem>())
	{
		TArray<UClass*> AllowedDataClasses = LoadAllowedDataClasses();

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPoint(
			ExtensionPointTag, ExtensionPointTagMatch, AllowedDataClasses,
			FExtendExtensionPointDelegate::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension)
		));

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPointForContext(
			ExtensionPointTag, GetOwningLocalPlayer(), ExtensionPointTagMatch, AllowedDataClasses,
			FExtendExtensionPointDelegate::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension)
		));
	}
}

void USigilGameUIExtensionPointWidget::RegisterExtensionPointForPlayerState(ULocalPlayer* LocalPlayer, APlayerState* PlayerState)
{
	if (USigilExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<USigilExtensionSubsystem>())
	{
		TArray<UClass*> AllowedDataClasses = LoadAllowedDataClasses();

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPointForContext(
			ExtensionPointTag, PlayerState, ExtensionPointTagMatch, AllowedDataClasses,
			FExtendExtensionPointDelegate::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension)
		));
	}
}

TArray<UClass*> USigilGameUIExtensionPointWidget::LoadAllowedDataClasses() const
{
	TArray<UClass*> AllowedDataClasses;
	AllowedDataClasses.Add(UUserWidget::StaticClass());

	for (const TSoftClassPtr<UObject>& DataClass : DataClasses)
	{
		if (!DataClass.IsNull())
		{
			AllowedDataClasses.Add(DataClass.LoadSynchronous());
		}
	}
	return AllowedDataClasses;
}

void USigilGameUIExtensionPointWidget::OnAddOrRemoveExtension(ESigilGameUIExtAction Action, const FSigilGameUIExtRequest& Request)
{
	if (Action == ESigilGameUIExtAction::Added)
	{
		UObject* Data = Request.Data;
		TSubclassOf<UUserWidget> WidgetClass(Cast<UClass>(Data));
		if (WidgetClass)
		{
			UUserWidget* Widget = CreateEntryInternal(WidgetClass);
			ExtensionMapping.Add(Request.ExtensionHandle, Widget);

			// Use UserWidgetInterface to notify it was registered.
			if (Widget->GetClass()->ImplementsInterface(USigilUserWidgetInterface::StaticClass()))
			{
				if (AActor* Actor = Cast<AActor>(Request.ContextObject))
				{
					ISigilUserWidgetInterface::Execute_SetOwningActor(Widget, Actor);
				}
				ISigilUserWidgetInterface::Execute_OnActivated(Widget);
			}
		}
		else if (DataClasses.Num() > 0)
		{
			if (GetWidgetClassForData.IsBound())
			{
				WidgetClass = GetWidgetClassForData.Execute(Data);

				// If the data is irrelevant they can just return no widget class.
				if (WidgetClass)
				{
					if (UUserWidget* Widget = CreateEntryInternal(WidgetClass))
					{
						ExtensionMapping.Add(Request.ExtensionHandle, Widget);
						ConfigureWidgetForData.ExecuteIfBound(Widget, Data);
						if (Widget->GetClass()->ImplementsInterface(USigilUserWidgetInterface::StaticClass()))
						{
							if (AActor* Actor = Cast<AActor>(Request.ContextObject))
							{
								ISigilUserWidgetInterface::Execute_SetOwningActor(Widget, Actor);
							}
							ISigilUserWidgetInterface::Execute_OnActivated(Widget);
						}
					}
				}
			}
		}
	}
	else
	{
		if (UUserWidget* Extension = ExtensionMapping.FindRef(Request.ExtensionHandle))
		{
			if (Extension->GetClass()->ImplementsInterface(USigilUserWidgetInterface::StaticClass()))
			{
				ISigilUserWidgetInterface::Execute_OnDeactivated(Extension);
				if (AActor* Actor = Cast<AActor>(Request.ContextObject))
				{
					ISigilUserWidgetInterface::Execute_SetOwningActor(Extension, nullptr);
				}
			}
			RemoveEntryInternal(Extension);
			ExtensionMapping.Remove(Request.ExtensionHandle);
		}
	}
}

#if WITH_EDITOR
void USigilGameUIExtensionPointWidget::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	// We don't care if the CDO doesn't have a specific tag.
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (!ExtensionPointTag.IsValid())
		{
			TSharedRef<FTokenizedMessage> Message = CompileLog.Error(FText::Format(
				LOCTEXT("USigilGameUIExtensionPointWidget_NoTag", "{0} has no ExtensionPointTag specified - All extension points must specify a tag so they can be located."),
				FText::FromString(GetName())));
			Message->AddToken(FUObjectToken::Create(this));
		}
	}
}
#endif

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
