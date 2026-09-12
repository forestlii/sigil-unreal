// Copyright (c) 2026 Likeon. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Equipping/SigilEquipmentInstance.h"
#include "Equipping/SigilEquipmentSystemComponent.h"
#include "SigilInventorySystemComponent.h"
#include "SigilEquipmentLifecycleTestTypes.generated.h"

UCLASS(Transient)
class USigilLifecycleTestInventory final : public USigilInventorySystemComponent
{
	GENERATED_BODY()
public:
	void Configure(const USigilItemCollectionDefinition* Definition) { CollectionDefinitions.Add(Definition); }
};

UCLASS(Transient)
class USigilLifecycleTestEquipment final : public USigilEquipmentSystemComponent
{
	GENERATED_BODY()
public:
	void Configure(FGameplayTag Tag) { TargetCollectionTag = Tag; }
	bool bResetOnAdded = false;
	UPROPERTY() TObjectPtr<UObject> LastAdded;
	UFUNCTION()
	void ObserveEquipment(UObject* Equipment, FGameplayTag Slot, bool bEquipped)
	{
		if (bEquipped)
		{
			LastAdded = Equipment;
			if (bResetOnAdded) { ResetEquipmentSystem(); }
		}
	}
};

UCLASS(Transient, NotPlaceable)
class ASigilLifecycleTestPawn final : public APawn
{
	GENERATED_BODY()
public:
	ASigilLifecycleTestPawn()
	{
		bReplicates = true;
		bReplicateUsingRegisteredSubObjectList = true;
	}
};

UCLASS(Transient)
class USigilLifecycleTestInstance final : public USigilEquipmentInstance
{
	GENERATED_BODY()
public:
	int32 EndCount = 0;
	int32 ActivateCount = 0;
	int32 Callback = 0;
	FGameplayTag Group;
	FGameplayTag Slot;
	UPROPERTY() TObjectPtr<USigilEquipmentSystemComponent> System;
	UPROPERTY() TObjectPtr<USigilItemInstance> ItemToEquip;
	FGameplayTag TargetSlot;

	virtual void OnActiveStateChanged_Implementation(bool bActive) override
	{
		Super::OnActiveStateChanged_Implementation(bActive);
		if (bActive) { ++ActivateCount; }
		if (!bActive && System)
		{
			const int32 Action = Callback;
			if (Action != 4) { Callback = 0; }
			if (Action == 1) { System->ResetEquipmentSystem(); }
			if (Action == 2) { System->UnequipBySlot(Slot); }
			if (Action == 3) { System->SetGroupActiveIndex(Group, 2); }
			if (Action == 7) { System->EquipItemToSlot(ItemToEquip, TargetSlot); }
		}
	}
	virtual void OnEquipmentEndPlay_Implementation() override
	{
		++EndCount;
		if (Callback == 4 && System)
		{
			Callback = 0;
			System->ResetEquipmentSystem();
		}
		Super::OnEquipmentEndPlay_Implementation();
	}
};
