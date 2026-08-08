// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DeveloperSettings.h"
#include "SigilSigilUISettings.generated.h"

class USigilGameModalWidget;
class USigilGameUIPolicy;

/**
 * Developer settings for the Generic UI System.
 * 通用UI系统的开发者设置。
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Generic UI System Settings"))
class SIGILUI_API USigilSigilUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the Generic UI System settings.
	 * 获取通用UI系统设置。
	 * @return The UI system settings. UI系统设置。
	 */
	UFUNCTION(BlueprintPure, Category="GUIS|Settings", meta = (DisplayName = "Get Generic UI System Settings"))
	static const USigilSigilUISettings* Get();

	/**
	 * Default UI policy class for the game layout.
	 * 游戏布局的默认UI策略类。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GUIS|Settings")
	TSoftClassPtr<USigilGameUIPolicy> GameUIPolicyClass;
};
