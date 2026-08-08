// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Pickups/GIS_WorldItemComponent.h"
#include "UObject/Object.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GIS_InventorySubsystem.h"
#include "GIS_InventorySystemComponent.h"
#include "Items/GIS_ItemDefinition.h"
#include "Items/GIS_ItemInstance.h"
#include "GIS_LogChannels.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_WorldItemComponent)

UGIS_WorldItemComponent::UGIS_WorldItemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	bReplicateUsingRegisteredSubObjectList = true;
}

void UGIS_WorldItemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemInfo, Parameters)
}

UGIS_WorldItemComponent* UGIS_WorldItemComponent::GetWorldItemComponent(const AActor* Actor)
{
	if (IsValid(Actor))
	{
		return Actor->FindComponentByClass<UGIS_WorldItemComponent>();
	}
	return nullptr;
}

void UGIS_WorldItemComponent::CreateItemFromDefinition(FGIS_ItemDefinitionAmount ItemDefinition)
{
	if (!ItemDefinition.Definition.IsNull() && ItemDefinition.Amount >= 1)
	{
		if (!ItemInfo.IsValid())
		{
			UGIS_ItemInstance* NewItemInstance = UGIS_InventorySubsystem::Get(GetWorld())->CreateItem(GetOwner(), ItemDefinition.Definition.LoadSynchronous());
			if (NewItemInstance == nullptr)
			{
				GIS_CLOG(Error, "failed to create item instance from definition!");
			}
			else
			{
				SetItemInfo(NewItemInstance, ItemDefinition.Amount);
			}
		}
		else
		{
			GIS_CLOG(Warning, "Already have valid item info, skip creation.")
		}
	}
	else
	{
		GIS_CLOG(Error, "passed invalid definition setup,skip item instance creating!");
	}
}

bool UGIS_WorldItemComponent::HasValidDefinition() const
{
	return !Definition.Definition.IsNull() && Definition.Amount >= 1;
}


void UGIS_WorldItemComponent::SetItemInfo(UGIS_ItemInstance* InItem, int32 InAmount)
{
	if (InItem == nullptr || InAmount <= 0)
	{
		return;
	}

	if (ItemInfo.IsValid())
	{
		GIS_CLOG(Warning, "Already have valid item info.")
		return;
	}

	ItemInfo = FGIS_ItemInfo(InItem, InAmount);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemInfo, this)

	// add to ReplicatedSubObject list only if it was created by this component. 
	if (bReplicateUsingRegisteredSubObjectList && InItem->GetOuter() == GetOwner())
	{
		AddReplicatedSubObject(ItemInfo.Item);
	}
}

void UGIS_WorldItemComponent::ResetItemInfo()
{
	if (ItemInfo.IsValid())
	{
		// remove from replicated sub object list only if it was created by this component. 
		if (bReplicateUsingRegisteredSubObjectList && ItemInfo.Item->GetOuter() == GetOwner())
		{
			RemoveReplicatedSubObject(ItemInfo.Item);
		}
	}
}

UGIS_ItemInstance* UGIS_WorldItemComponent::GetItemInstance()
{
	return ItemInfo.Item;
}

UGIS_ItemInstance* UGIS_WorldItemComponent::GetDuplicatedItemInstance(AActor* NewOwner)
{
	if (ItemInfo.IsValid())
	{
		return UGIS_InventorySubsystem::Get(GetWorld())->DuplicateItem(NewOwner, ItemInfo.Item);
	}
	return nullptr;
}

FGIS_ItemInfo UGIS_WorldItemComponent::GetItemInfo() const
{
	return ItemInfo;
}

int32 UGIS_WorldItemComponent::GetItemAmount() const
{
	return ItemInfo.Amount;
}

void UGIS_WorldItemComponent::BeginPlay()
{
	if (HasValidDefinition() && GetOwner()->HasAuthority())
	{
		CreateItemFromDefinition(Definition);
	}
	Super::BeginPlay();
}

void UGIS_WorldItemComponent::OnRep_ItemInfo()
{
	if (ItemInfo.IsValid())
	{
		GIS_CLOG(Verbose, "item:%s replicated!", *ItemInfo.Item->GetDefinition()->GetName());
		ItemInfoSetEvent.Broadcast(ItemInfo);
	}
}
