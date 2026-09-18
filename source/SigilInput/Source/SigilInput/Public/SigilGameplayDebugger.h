// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once
#include "GameplayTagContainer.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"
#include "InputTriggers.h"

class APlayerController;
class AActor;

class FSigilGameplayDebuggerCategory_Input : public FGameplayDebuggerCategory
{
public:
	FSigilGameplayDebuggerCategory_Input();
	void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	
	void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
    
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	/**
	 * Resolves the input system for a debug target: the actor itself, or the controller of a pawn target when the component lives on its PlayerController.
	 * 解析调试目标的输入系统：先查目标自身；若目标是Pawn且组件在其PlayerController上，则查该Pawn的Controller。
	 */
	static const class USigilInputSystemComponent* ResolveInputSystem(const AActor* DebugActor);

	/**
	 * Display name for the debug header: the controlled pawn, otherwise the host with an explicit NoPawn marker.
	 * 调试标题显示名：优先受控Pawn，否则显示宿主名并标注NoPawn。
	 */
	static FString GetDisplayName(const class USigilInputSystemComponent* InputSystem);

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
