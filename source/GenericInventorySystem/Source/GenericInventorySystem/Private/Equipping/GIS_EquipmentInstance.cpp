// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIS_EquipmentInstance.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS
#include "GIS_EquipmentSystemComponent.h"
#include "Items/GIS_ItemInstance.h"
#include "GIS_ItemFragment_Equippable.h"
#include "GIS_LogChannels.h"
#include "Pickups/GIS_WorldItemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_EquipmentInstance)

class FLifetimeProperty;
class UClass;
class USceneComponent;

UGIS_EquipmentInstance::UGIS_EquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsActive = false;
}

bool UGIS_EquipmentInstance::IsSupportedForNetworking() const
{
	return true;
}

UWorld* UGIS_EquipmentInstance::GetWorld() const
{
	if (OwningPawn)
	{
		return OwningPawn->GetWorld();
	}
	return nullptr;
}

void UGIS_EquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(ThisClass, SourceItem);

	//fix: https://forums.unrealengine.com/t/subobject-replication-for-blueprint-child-class/106205/4
	UBlueprintGeneratedClass* bpClass = Cast<UBlueprintGeneratedClass>(this->GetClass());
	if (bpClass != nullptr)
	{
		bpClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
	DOREPLIFETIME(ThisClass, EquipmentActors);
}

void UGIS_EquipmentInstance::ReceiveOwningPawn_Implementation(APawn* NewPawn)
{
	OwningPawn = NewPawn;
}

APawn* UGIS_EquipmentInstance::GetOwningPawn_Implementation() const
{
	return OwningPawn;
}

void UGIS_EquipmentInstance::ReceiveSourceItem_Implementation(UGIS_ItemInstance* NewItem)
{
	SourceItem = NewItem;
}

UGIS_ItemInstance* UGIS_EquipmentInstance::GetSourceItem_Implementation() const
{
	return SourceItem;
}

void UGIS_EquipmentInstance::OnEquipmentBeginPlay_Implementation()
{
	if (OwningPawn->HasAuthority())
	{
		SpawnAndSetupEquipmentActors(SourceItem->FindFragmentByClass<UGIS_ItemFragment_Equippable>()->ActorsToSpawn);
	}
}

void UGIS_EquipmentInstance::OnEquipmentEndPlay_Implementation()
{
	DestroyEquipmentActors();
}

#if UE_WITH_IRIS
void UGIS_EquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}


#endif // UE_WITH_IRIS

bool UGIS_EquipmentInstance::IsEquipmentActive_Implementation() const
{
	return bIsActive;
}

void UGIS_EquipmentInstance::OnActiveStateChanged_Implementation(bool NewActiveState)
{
	bIsActive = NewActiveState;
	SetupActiveStateForEquipmentActors(EquipmentActors);
	OnActiveStateChangedEvent.Broadcast(NewActiveState);
}

APawn* UGIS_EquipmentInstance::GetTypedOwningPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (APawn* Pawn = Execute_GetOwningPawn(this))
		{
			if (Pawn->IsA(ActualPawnType))
			{
				Result = Pawn;
			}
		}
	}
	return Result;
}

bool UGIS_EquipmentInstance::CanActivate_Implementation() const
{
	return true;
}

AActor* UGIS_EquipmentInstance::GetTypedEquipmentActor(TSubclassOf<AActor> DesiredClass) const
{
	if (UClass* RealClass = DesiredClass)
	{
		for (const TObjectPtr<AActor>& SpawnedActor : EquipmentActors)
		{
			if (SpawnedActor && SpawnedActor->GetClass()->IsChildOf(RealClass))
			{
				return SpawnedActor;
			}
		}
	}
	return nullptr;
}

void UGIS_EquipmentInstance::SpawnAndSetupEquipmentActors(const TArray<FGIS_EquipmentActorToSpawn>& ActorsToSpawn)
{
	if (OwningPawn == nullptr || !OwningPawn->HasAuthority())
	{
		return;
	}

	USceneComponent* AttachParent = GetAttachParentForSpawnedActors(OwningPawn);

	for (int32 i = 0; i < ActorsToSpawn.Num(); ++i)
	{
		const FGIS_EquipmentActorToSpawn& SpawnInfo = ActorsToSpawn[i];
		if (SpawnInfo.ActorToSpawn.IsNull())
		{
			continue;
		}
		AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn.LoadSynchronous(), FTransform::Identity, OwningPawn);
		if (NewActor == nullptr)
		{
			GIS_LOG(Warning, "%s Failed to spawn actor of class: %s at index: %d ", *GetName(), *SpawnInfo.ActorToSpawn->GetName(), i);
			continue;
		}
		BeforeSpawningActor(NewActor);
		NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
		if (SpawnInfo.bShouldAttach)
		{
			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			NewActor->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);
		}
		EquipmentActors.Add(NewActor);
	}
	SetupEquipmentActors(EquipmentActors);
}

void UGIS_EquipmentInstance::DestroyEquipmentActors()
{
	if (OwningPawn == nullptr || !OwningPawn->HasAuthority())
	{
		return;
	}
	for (AActor* Actor : EquipmentActors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}
		if (Actor->GetClass()->ImplementsInterface(UGIS_EquipmentInterface::StaticClass()))
		{
			if (Execute_IsEquipmentActive(Actor))
			{
				Execute_OnActiveStateChanged(Actor, false);
			}
			Execute_OnEquipmentEndPlay(Actor);
			// may be destroyed during unequip.
			if (IsValid(Actor))
			{
				Execute_ReceiveOwningPawn(Actor, nullptr);
				Execute_ReceiveSourceItem(Actor, nullptr);
				Actor->Destroy();
			}
		}
		else
		{
			Actor->Destroy();
		}
	}
	EquipmentActors.Empty();
}

USceneComponent* UGIS_EquipmentInstance::GetAttachParentForSpawnedActors_Implementation(APawn* Pawn) const
{
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		return Char->GetMesh();
	}
	if (Pawn)
	{
		return Pawn->FindComponentByClass<USkeletalMeshComponent>();
	}
	return nullptr;
}

void UGIS_EquipmentInstance::BeforeSpawningActor_Implementation(AActor* SpawningActor) const
{
}

void UGIS_EquipmentInstance::SetupEquipmentActors_Implementation(const TArray<AActor*>& InActors) const
{
	if (!OwningPawn)
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		for (int32 i = 0; i < InActors.Num(); i++)
		{
			UGIS_WorldItemComponent* WorldItem = InActors[i]->FindComponentByClass<UGIS_WorldItemComponent>();
			if (WorldItem != nullptr)
			{
				WorldItem->SetItemInfo(SourceItem, 1);
			}
		}
	}

	SetupInitialStateForEquipmentActors(EquipmentActors);
}

void UGIS_EquipmentInstance::OnRep_EquipmentActors()
{
	TryWaitEquipmentActors();
}

void UGIS_EquipmentInstance::TryWaitEquipmentActors()
{
	if (EquipmentActors.IsEmpty() || WaitEquipmentActorsTimer.IsValid())
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(WaitEquipmentActorsTimer, FTimerDelegate::CreateUObject(this, &ThisClass::WaitEquipmentActors), 0.2f, true);
	}
}

void UGIS_EquipmentInstance::WaitEquipmentActors()
{
	if (SourceItem == nullptr || OwningPawn == nullptr || !WaitEquipmentActorsTimer.IsValid())
	{
		return;
	}
	if (OwningPawn->HasAuthority())
	{
		WaitEquipmentActorsTimer.Invalidate();
		return;
	}
	const UGIS_ItemFragment_Equippable* Equippable = SourceItem->FindFragmentByClass<UGIS_ItemFragment_Equippable>();
	if (Equippable == nullptr || Equippable->bActorBased || Equippable->ActorsToSpawn.IsEmpty())
	{
		WaitEquipmentActorsTimer.Invalidate();
		return;
	}

	if (!IsEquipmentActorsValid(Equippable->ActorsToSpawn.Num()))
	{
		return;
	}

	WaitEquipmentActorsTimer.Invalidate();
	SetupInitialStateForEquipmentActors(EquipmentActors);
	GIS_CLOG(VeryVerbose, "Equipment Actors synced.")
}

bool UGIS_EquipmentInstance::IsEquipmentActorsValid(int32 Num) const
{
	if (EquipmentActors.Num() == Num)
	{
		bool bValid = true;
		for (int32 i = 0; i < EquipmentActors.Num(); i++)
		{
			if (!IsValid(EquipmentActors[i]))
			{
				bValid = false;
				break;
			}
		}
		return bValid;
	}
	return false;
}

void UGIS_EquipmentInstance::SetupInitialStateForEquipmentActors(const TArray<AActor*>& InActors) const
{
	for (int32 i = 0; i < InActors.Num(); i++)
	{
		AActor* Actor = InActors[i];

		if (Actor && Actor->GetClass()->ImplementsInterface(UGIS_EquipmentInterface::StaticClass()))
		{
			Execute_ReceiveOwningPawn(Actor, OwningPawn);
			Execute_ReceiveSourceItem(Actor, SourceItem);
			Execute_OnEquipmentBeginPlay(Actor);
			Execute_OnActiveStateChanged(Actor, bIsActive);
		}
	}
}

void UGIS_EquipmentInstance::SetupActiveStateForEquipmentActors(const TArray<AActor*>& InActors) const
{
	for (int32 i = 0; i < InActors.Num(); i++)
	{
		AActor* Actor = InActors[i];
		if (IsValid(Actor) && Actor->GetClass()->ImplementsInterface(UGIS_EquipmentInterface::StaticClass()))
		{
			Execute_OnActiveStateChanged(Actor, bIsActive);
		}
	}
}
