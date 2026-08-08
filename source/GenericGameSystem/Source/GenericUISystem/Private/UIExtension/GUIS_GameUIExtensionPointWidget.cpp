// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UIExtension/GUIS_GameUIExtensionPointWidget.h"
#include "Widgets/SOverlay.h"
#include "TimerManager.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor/WidgetCompilerLog.h"
#include "Misc/UObjectToken.h"
#include "GameFramework/PlayerState.h"
#include "UI/Common/GUIS_UserWidgetInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_GameUIExtensionPointWidget)

#define LOCTEXT_NAMESPACE "UIExtension"

/////////////////////////////////////////////////////
// UGUIS_GameUIExtensionPointWidget

UGUIS_GameUIExtensionPointWidget::UGUIS_GameUIExtensionPointWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGUIS_GameUIExtensionPointWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	ResetExtensionPoint();

	Super::ReleaseSlateResources(bReleaseChildren);
}

TSharedRef<SWidget> UGUIS_GameUIExtensionPointWidget::RebuildWidget()
{
	if (!IsDesignTime() && ExtensionPointTag.IsValid())
	{
		ResetExtensionPoint();
		RegisterExtensionPoint();

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

void UGUIS_GameUIExtensionPointWidget::RegisterForPlayerStateIfReady()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
		{
			RegisterExtensionPointForPlayerState(GetOwningLocalPlayer(), PS);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::OnCheckPlayerState, 0.2f);
	}
}

bool UGUIS_GameUIExtensionPointWidget::CheckPlayerState()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
		{
			if (TimerHandle.IsValid())
			{
				TimerHandle.Invalidate();
			}
			RegisterExtensionPointForPlayerState(GetOwningLocalPlayer(), PS);
			return true;
		}
	}

	return false;
}

void UGUIS_GameUIExtensionPointWidget::OnCheckPlayerState()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
		{
			if (TimerHandle.IsValid())
			{
				TimerHandle.Invalidate();
			}
			RegisterExtensionPointForPlayerState(GetOwningLocalPlayer(), PS);
		}
	}
}

void UGUIS_GameUIExtensionPointWidget::ResetExtensionPoint()
{
	ResetInternal();

	ExtensionMapping.Reset();
	for (FGUIS_GameUIExtPointHandle& Handle : ExtensionPointHandles)
	{
		Handle.Unregister();
	}
	ExtensionPointHandles.Reset();
}

void UGUIS_GameUIExtensionPointWidget::RegisterExtensionPoint()
{
	if (UGUIS_ExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UGUIS_ExtensionSubsystem>())
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

void UGUIS_GameUIExtensionPointWidget::RegisterExtensionPointForPlayerState(ULocalPlayer* LocalPlayer, APlayerState* PlayerState)
{
	if (UGUIS_ExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UGUIS_ExtensionSubsystem>())
	{
		TArray<UClass*> AllowedDataClasses = LoadAllowedDataClasses();

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPointForContext(
			ExtensionPointTag, PlayerState, ExtensionPointTagMatch, AllowedDataClasses,
			FExtendExtensionPointDelegate::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension)
		));
	}
}

TArray<UClass*> UGUIS_GameUIExtensionPointWidget::LoadAllowedDataClasses() const
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

void UGUIS_GameUIExtensionPointWidget::OnAddOrRemoveExtension(EGUIS_GameUIExtAction Action, const FGUIS_GameUIExtRequest& Request)
{
	if (Action == EGUIS_GameUIExtAction::Added)
	{
		UObject* Data = Request.Data;
		TSubclassOf<UUserWidget> WidgetClass(Cast<UClass>(Data));
		if (WidgetClass)
		{
			UUserWidget* Widget = CreateEntryInternal(WidgetClass);
			ExtensionMapping.Add(Request.ExtensionHandle, Widget);

			// Use UserWidgetInterface to notify it was registered.
			if (Widget->GetClass()->ImplementsInterface(UGUIS_UserWidgetInterface::StaticClass()))
			{
				if (AActor* Actor = Cast<AActor>(Request.ContextObject))
				{
					IGUIS_UserWidgetInterface::Execute_SetOwningActor(Widget, Actor);
				}
				IGUIS_UserWidgetInterface::Execute_OnActivated(Widget);
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
						if (Widget->GetClass()->ImplementsInterface(UGUIS_UserWidgetInterface::StaticClass()))
						{
							if (AActor* Actor = Cast<AActor>(Request.ContextObject))
							{
								IGUIS_UserWidgetInterface::Execute_SetOwningActor(Widget, Actor);
							}
							IGUIS_UserWidgetInterface::Execute_OnActivated(Widget);
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
			if (Extension->GetClass()->ImplementsInterface(UGUIS_UserWidgetInterface::StaticClass()))
			{
				IGUIS_UserWidgetInterface::Execute_OnDeactivated(Extension);
				if (AActor* Actor = Cast<AActor>(Request.ContextObject))
				{
					IGUIS_UserWidgetInterface::Execute_SetOwningActor(Extension, nullptr);
				}
			}
			RemoveEntryInternal(Extension);
			ExtensionMapping.Remove(Request.ExtensionHandle);
		}
	}
}

#if WITH_EDITOR
void UGUIS_GameUIExtensionPointWidget::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	// We don't care if the CDO doesn't have a specific tag.
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (!ExtensionPointTag.IsValid())
		{
			TSharedRef<FTokenizedMessage> Message = CompileLog.Error(FText::Format(
				LOCTEXT("UGUIS_GameUIExtensionPointWidget_NoTag", "{0} has no ExtensionPointTag specified - All extension points must specify a tag so they can be located."),
				FText::FromString(GetName())));
			Message->AddToken(FUObjectToken::Create(this));
		}
	}
}
#endif

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
