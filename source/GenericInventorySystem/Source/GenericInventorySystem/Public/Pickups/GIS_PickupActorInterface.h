// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GIS_PickupActorInterface.generated.h"

/**
 * Interface for filtering pickup class selection.
 * 用于筛选拾取类选择的接口。
 * @details Acts as a marker interface for actors that support pickup functionality.
 * @细节 作为支持拾取功能的演员的标记接口。
 */
UINTERFACE()
class UGIS_PickupActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface class for pickup actor functionality.
 * 拾取演员功能的接口类。
 */
class GENERICINVENTORYSYSTEM_API IGIS_PickupActorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	// 在此添加接口函数。此类将被继承以实现该接口。
};