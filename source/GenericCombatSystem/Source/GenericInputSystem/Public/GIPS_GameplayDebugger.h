// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once
#include "GameplayTagContainer.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"
#include "InputTriggers.h"

class APlayerController;
class AActor;

class FGIPS_GameplayDebuggerCategory_Input : public FGameplayDebuggerCategory
{
public:
	FGIPS_GameplayDebuggerCategory_Input();
	void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	
	void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
    
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	void OnShowInputBuffersToggle();
	void OnShowPassedInputEntriesToggle();
	void OnShowBlockedInputEntriesToggle();
	void OnShowBufferedInputEntriesToggle();
    
protected:
	void DrawInputBuffers(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const;
	void DrawInputEntries(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const;
	
	struct FRepData
	{
		FString ActorName;
		FString InputConfig;
		FString InputControlSetup;
		
		FGameplayTag BufferedInputTag;

		struct FInputBuffersDebug
		{
			FName WindowName;
			bool bIsActive;
			FName InputTagName;
			ETriggerEvent InputEvent;
		};

		TArray<FInputBuffersDebug> InputBuffers;
        
		void Serialize(FArchive&Ar);
	};
    
	FRepData DataPack;

private:

	// Save off the last expected draw size so that we can draw a border around it next frame (and hope we're the same size)
	float LastDrawDataEndSize = 0.0f;

	bool bShowInputBuffers = true;
	bool bShowPassedInputEntries = true;
	bool bShowBlockedInputEntries = true;
	bool bShowBufferedInputEntries = true;
};


#endif // WITH_GAMEPLAY_DEBUGG
