// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GUIS_UserWidgetInterface.generated.h"

/**
 * Interface for UserWidget functionality.
 * 通用UserWidget功能的接口。
 * @note Designed for UserWidgets (except UCommonActivatableWidget and its derivatives).
 * @注意 专为UserWidget设计（不包括UCommonActivatableWidget及其派生类）。
 * @details Automatically called when used as an extension UI.
 * @细节 用作扩展UI时自动调用。
 */
UINTERFACE()
class GENERICUISYSTEM_API UGUIS_UserWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implementation class for UserWidget interface.
 * UserWidget接口的实现类。
 */
class GENERICUISYSTEM_API IGUIS_UserWidgetInterface
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the owning actor of the UserWidget.
	 * 获取UserWidget的所属演员。
	 * @return The logical owning actor. 逻辑所属演员。
	 * @note Tracks data for an actor in the game world.
	 * @注意 跟踪游戏世界中演员的数据。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GUIS")
	AActor* GetOwningActor();

	/**
	 * Sets the owning actor of the UserWidget.
	 * 设置UserWidget的所属演员。
	 * @param NewOwningActor The new owning actor. 新的所属演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GUIS")
	void SetOwningActor(AActor* NewOwningActor);

	/**
	 * Called when the UserWidget is activated.
	 * UserWidget激活时调用。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GUIS")
	void OnActivated();

	/**
	 * Called when the UserWidget is deactivated.
	 * UserWidget禁用时调用。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GUIS")
	void OnDeactivated();
};
