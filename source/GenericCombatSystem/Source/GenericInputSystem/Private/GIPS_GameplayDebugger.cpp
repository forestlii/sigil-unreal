// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIPS_GameplayDebugger.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GIPS_InputConfig.h"
#include "GIPS_InputControlSetup.h"
#include "GIPS_InputFunctionLibrary.h"
#include "GIPS_InputSystemComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"


FGIPS_GameplayDebuggerCategory_Input::FGIPS_GameplayDebuggerCategory_Input()
{
	SetDataPackReplication(&DataPack);
	const FName KeyNameOne{"One"};
	const FName KeyNameTwo{"Two"};
	const FName KeyNameThree{"Three"};
	const FName KeyNameFour{"Four"};

	BindKeyPress(KeyNameOne, FGameplayDebuggerInputModifier::Shift, this, &FGIPS_GameplayDebuggerCategory_Input::OnShowInputBuffersToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(KeyNameTwo, FGameplayDebuggerInputModifier::Shift, this, &FGIPS_GameplayDebuggerCategory_Input::OnShowPassedInputEntriesToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(KeyNameThree, FGameplayDebuggerInputModifier::Shift, this, &FGIPS_GameplayDebuggerCategory_Input::OnShowBlockedInputEntriesToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(KeyNameFour, FGameplayDebuggerInputModifier::Shift, this, &FGIPS_GameplayDebuggerCategory_Input::OnShowBufferedInputEntriesToggle, EGameplayDebuggerInputMode::Local);
}

void FGIPS_GameplayDebuggerCategory_Input::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (const UGIPS_InputSystemComponent* InputSystem = UGIPS_InputSystemComponent::GetInputSystemComponent(DebugActor))
	{
		if (UGIPS_InputControlSetup* InputControlSetup = InputSystem->GetCurrentInputSetup())
		{
			if (UGIPS_InputConfig* InputConfig = InputSystem->GetInputConfig())
			{
				DataPack.ActorName = OwnerPC->GetPawn()->GetName();
				DataPack.InputConfig = InputConfig->GetName();
				DataPack.InputControlSetup = InputControlSetup->GetName();

				TMap<FGameplayTag, FGIPS_BufferedInput> ActiveWindows = InputSystem->GetActiveBufferWindows();

				for (const FGIPS_InputBufferWindow& Definition : InputConfig->InputBufferDefinitions)
				{
					FRepData::FInputBuffersDebug ItemData;

					ItemData.WindowName = UGIPS_InputFunctionLibrary::GetLastTagName(Definition.Tag);
					ItemData.bIsActive = ActiveWindows.Contains(Definition.Tag);
					ItemData.InputTagName = ItemData.bIsActive && ActiveWindows[Definition.Tag].InputTag.IsValid()
						                        ? UGIPS_InputFunctionLibrary::GetLastTagName(ActiveWindows[Definition.Tag].InputTag)
						                        : NAME_None;
					ItemData.InputEvent = ItemData.bIsActive ? ActiveWindows[Definition.Tag].TriggerEvent : ETriggerEvent::None;

					DataPack.InputBuffers.Add(ItemData);
				}
				FGIPS_BufferedInput Input = InputSystem->GetLastBufferedInput();
				DataPack.BufferedInputTag = Input.InputTag;
			}
		}
	}
}

void FGIPS_GameplayDebuggerCategory_Input::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	// Draw the sub-category bindings inline with the category header
	{
		CanvasContext.CursorX += 200.0f;
		CanvasContext.CursorY -= CanvasContext.GetLineHeight();
		const TCHAR* Active = TEXT("{green}");
		const TCHAR* Inactive = TEXT("{grey}");
		CanvasContext.Printf(TEXT("InputBuffers [%s%s{white}]\t PassedInputEntries [%s%s{white}]\t BlockedInputEntries [%s%s{white}]\t BufferedInputEntries [%s%s{white}]\t"),
		                     bShowInputBuffers ? Active : Inactive, *GetInputHandlerDescription(0),
		                     bShowPassedInputEntries ? Active : Inactive, *GetInputHandlerDescription(1),
		                     bShowBlockedInputEntries ? Active : Inactive, *GetInputHandlerDescription(2),
		                     bShowBufferedInputEntries ? Active : Inactive, *GetInputHandlerDescription(3)
		);
	}

	if (LastDrawDataEndSize <= 0.0f)
	{
		// Default to the full frame size
		LastDrawDataEndSize = CanvasContext.Canvas->SizeY - CanvasContext.CursorY - CanvasContext.CursorX;
	}

	constexpr FLinearColor BackgroundColor(0.1f, 0.1f, 0.1f, 0.8f);
	const FVector2D BackgroundPos{CanvasContext.CursorX, CanvasContext.CursorY};
	const FVector2D BackgroundSize(CanvasContext.Canvas->SizeX - (2.0f * CanvasContext.CursorX), LastDrawDataEndSize);

	// Draw a transparent dark background so that the text is easier to look at
	FCanvasTileItem Background(FVector2D(0.0f), BackgroundSize, BackgroundColor);
	Background.BlendMode = SE_BLEND_Translucent;

	CanvasContext.DrawItem(Background, BackgroundPos.X, BackgroundPos.Y);

	CanvasContext.Printf(TEXT("{white}Actor name: {yellow}%s \t{white}Config: {yellow}%s \t{white}Control: {yellow}%s"), *DataPack.ActorName, *DataPack.InputConfig, *DataPack.InputControlSetup);

	if (bShowInputBuffers)
	{
		DrawInputBuffers(CanvasContext, OwnerPC);
	}

	DrawInputEntries(CanvasContext, OwnerPC);
}

void FGIPS_GameplayDebuggerCategory_Input::FRepData::Serialize(FArchive& Ar)
{
	Ar << ActorName;
	Ar << InputConfig;
	Ar << InputControlSetup;
}

TSharedRef<FGameplayDebuggerCategory> FGIPS_GameplayDebuggerCategory_Input::MakeInstance()
{
	return MakeShareable(new FGIPS_GameplayDebuggerCategory_Input());
}

void FGIPS_GameplayDebuggerCategory_Input::OnShowInputBuffersToggle()
{
	bShowInputBuffers = !bShowInputBuffers;
}

void FGIPS_GameplayDebuggerCategory_Input::OnShowPassedInputEntriesToggle()
{
	bShowPassedInputEntries = !bShowPassedInputEntries;
}

void FGIPS_GameplayDebuggerCategory_Input::OnShowBlockedInputEntriesToggle()
{
	bShowBlockedInputEntries = !bShowBlockedInputEntries;
}

void FGIPS_GameplayDebuggerCategory_Input::OnShowBufferedInputEntriesToggle()
{
	bShowBufferedInputEntries = !bShowBufferedInputEntries;
}

void FGIPS_GameplayDebuggerCategory_Input::DrawInputBuffers(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const
{
	const float CanvasWidth = CanvasContext.Canvas->SizeX;

	int32 NumActive = 0;
	for (const FRepData::FInputBuffersDebug& ItemData : DataPack.InputBuffers)
	{
		NumActive += ItemData.bIsActive;
	}

	// Measure the individual string sizes, so that we can size the columns properly
	// We're picking a long-enough name for the object name sizes
	constexpr float Padding = 10.0f;
	static float WindowNameSize = 0.0f, ActiveNameSize = 0.0f, InputTagSize = 0.0f, InputEventSize = 0.0f;

	if (WindowNameSize <= 0.0f)
	{
		float TempSizeY = 0.0f;

		// We have to actually use representative strings because of the kerning
		CanvasContext.MeasureString(TEXT("WindowName: Combo"), WindowNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("Active: False"), ActiveNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("InputTag: InputTag.Attack"), InputTagSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("InputEvent: Triggered"), InputEventSize, TempSizeY);

		WindowNameSize += Padding;
	}

	const float ColumnWidth = WindowNameSize * 2 + ActiveNameSize + InputTagSize + InputEventSize;
	const int NumColumns = FMath::Max(1, FMath::FloorToInt(CanvasWidth / ColumnWidth));

	CanvasContext.Print(TEXT("Input Buffers:"));
	CanvasContext.CursorX += 200.0f;
	CanvasContext.CursorY -= CanvasContext.GetLineHeight();
	CanvasContext.Printf(TEXT("Legend:  {yellow}Total [%d]    {cyan}Active [%d]"), DataPack.InputBuffers.Num(), NumActive);

	CanvasContext.CursorX += Padding;

	for (const FRepData::FInputBuffersDebug& ItemData : DataPack.InputBuffers)
	{
		float CursorX = CanvasContext.CursorX;
		float CursorY = CanvasContext.CursorY;
		// Print positions manually to align them properly
		CanvasContext.PrintAt(CursorX + WindowNameSize * 0, CursorY, ItemData.bIsActive ? FColor::Cyan : FColor::Yellow, ItemData.WindowName.ToString());
		CanvasContext.PrintAt(CursorX + WindowNameSize * 1, CursorY, FString::Printf(TEXT("{grey}active: {white}%.35s"), ItemData.bIsActive ? TEXT("True") : TEXT("False")));
		CanvasContext.PrintAt(CursorX + WindowNameSize * 2, CursorY, FString::Printf(TEXT("{grey}InputTag: {white}%s"), *ItemData.InputTagName.ToString()));
		CanvasContext.PrintAt(CursorX + WindowNameSize * 3, CursorY,
		                      FString::Printf(TEXT("{grey}InputEvent: {white}%s"), *UGIPS_InputFunctionLibrary::GetTriggerEventString(ItemData.InputEvent)));

		// PrintAt would have reset these values, restore them.
		CanvasContext.CursorX = CursorX + (CanvasWidth / NumColumns);
		CanvasContext.CursorY = CursorY;

		// If we're going to overflow, go to the next line...
		if (CanvasContext.CursorX + ColumnWidth >= CanvasWidth)
		{
			CanvasContext.MoveToNewLine();
			CanvasContext.CursorX += Padding;
		}
	}

	// End the row with a newline
	if (CanvasContext.CursorX != CanvasContext.DefaultX)
	{
		CanvasContext.MoveToNewLine();
	}

	CanvasContext.Printf(TEXT("{grey}Last triggered input: {white}%s"), *(DataPack.BufferedInputTag.IsValid() ? DataPack.BufferedInputTag.GetTagName().ToString() : TEXT("None")));

	// End the category with a newline to separate from the other categories
	CanvasContext.MoveToNewLine();
}

void FGIPS_GameplayDebuggerCategory_Input::DrawInputEntries(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const
{
	const UGIPS_InputSystemComponent* InputSystem = UGIPS_InputSystemComponent::GetInputSystemComponent(FindLocalDebugActor());
	if (InputSystem == nullptr || (!bShowPassedInputEntries && !bShowBlockedInputEntries && !bShowBufferedInputEntries))
		return;

	const float CanvasWidth = CanvasContext.Canvas->SizeX;

	constexpr float Padding = 10.0f;

	constexpr float ColumnSize = 150;
	constexpr float ColumnWidth = ColumnSize * 3;
	const int NumColumns = FMath::Max(1, FMath::FloorToInt(CanvasContext.Canvas->SizeX / ColumnWidth));

	auto DrawEntries = [&](const FString& Header, const TArray<FGIPS_BufferedInput>& Entries)
	{
		CanvasContext.Printf(TEXT("%s"), *Header);

		CanvasContext.CursorX += Padding;

		for (int i = 0; i < Entries.Num(); ++i)
		{
			const FGIPS_BufferedInput& Entry = Entries[i];
			float CursorX = CanvasContext.CursorX;
			float CursorY = CanvasContext.CursorY;
			// Print positions manually to align them properly
			CanvasContext.PrintAt(CursorX + ColumnSize * 0, CursorY, i == Entries.Num() - 1 ? FColor::Cyan : FColor::Yellow,
			                      Entry.InputTag.IsValid() ? *UGIPS_InputFunctionLibrary::GetLastTagName(Entry.InputTag).ToString() : TEXT("None"));
			CanvasContext.PrintAt(CursorX + ColumnSize * 1, CursorY, FString::Printf(TEXT("{grey}TriggerEvent: {white}%s"), *UGIPS_InputFunctionLibrary::GetTriggerEventString(Entry.TriggerEvent)));
			CanvasContext.PrintAt(CursorX + ColumnSize * 2, CursorY, FString::Printf(TEXT("{grey}ActionValue: {white}%s"), *Entry.ActionData.GetValue().ToString()));

			// PrintAt would have reset these values, restore them.
			CanvasContext.CursorX = CursorX + (CanvasWidth / NumColumns);
			CanvasContext.CursorY = CursorY;

			CanvasContext.MoveToNewLine();
			CanvasContext.CursorX += Padding;
		}

		// End the row with a newline
		if (CanvasContext.CursorX != CanvasContext.DefaultX)
		{
			CanvasContext.MoveToNewLine();
		}
	};

	if (bShowPassedInputEntries)
	{
		auto Entries = InputSystem->GetPassedInputEntries();
		DrawEntries(TEXT("Passed Inputs:"), Entries);
	}

	if (bShowBlockedInputEntries)
	{
		auto Entries = InputSystem->GetBlockedInputEntries();
		DrawEntries(TEXT("Blocked Inputs:"), Entries);
	}

	if (bShowBufferedInputEntries)
	{
		auto Entries = InputSystem->GetBufferedInputEntries();
		DrawEntries(TEXT("Buffered Inputs:"), Entries);
	}

	// End the category with a newline to separate from the other categories
	CanvasContext.MoveToNewLine();
}

#endif
