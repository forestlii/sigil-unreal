// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Bullet/SigilBulletSubsystem.h"

#include "SigilCombatLogChannels.h"
#include "Bullet/SigilBulletInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"


FString FSigilBulletSpawnParameters::ToDebugString() const
{
	return FString::Format(TEXT("Owner:{0} DefinitionHandle:{1}"), {Owner ? Owner->GetName() : "Null", DefinitionHandle.ToDebugString()});
}

TArray<ASigilBulletInstance*> USigilBulletSubsystem::SpawnBullets(const FSigilBulletSpawnParameters& SpawnParameters)
{
	TArray<ASigilBulletInstance*> RetInstances{};

	FSigilBulletDefinition Definition;
	if (SpawnParameters.DefinitionHandle.IsNull() || !LoadBulletDefinition(SpawnParameters.DefinitionHandle, Definition))
	{
		return RetInstances;
	}

	RetInstances = GetOrCreateBulletInstances(SpawnParameters, Definition);

	for (int i = 0; i < RetInstances.Num(); ++i)
	{
		ASigilBulletInstance* Instance = RetInstances[i];

		Instance->bServerInitiated = GetWorld()->GetNetMode() < NM_Client;
		Instance->bIsLocalPredicting = SpawnParameters.bIsLocalPredicting;

		if (SpawnParameters.OverrideBulletIds.IsValidIndex(i))
		{
			Instance->SetBulletId(SpawnParameters.OverrideBulletIds[0]);
		}
		else
		{
			Instance->SetBulletId(FGuid::NewGuid());
		}

		if (SpawnParameters.ParentId.IsValid() && BulletInstances.Contains(SpawnParameters.ParentId))
		{
			Instance->SetParentBulletId(SpawnParameters.ParentId);
		}

		if (SpawnParameters.Request)
		{
			Instance->Request = SpawnParameters.Request;
		}

		if (SpawnParameters.Owner)
		{
			Instance->SetOwner(SpawnParameters.Owner);
		}
		Instance->SetDefinitionHandle(SpawnParameters.DefinitionHandle);
		BulletInstances.Emplace(Instance->BulletId, Instance);
	}

	FRotator OriginalRotation = SpawnParameters.SpawnTransform.Rotator();
	for (int i = 0; i < RetInstances.Num(); ++i)
	{
		FTransform ModifiedTransform = SpawnParameters.SpawnTransform;
		FRotator RotationYawOffset(Definition.LaunchElevationAngle, Definition.LaunchAngle + Definition.LaunchAngleInterval * i, 0);
		ModifiedTransform.SetRotation(UKismetMathLibrary::ComposeRotators(OriginalRotation, RotationYawOffset).Quaternion());
		RetInstances[i]->SetActorTransform(ModifiedTransform, false, nullptr, ETeleportType::ResetPhysics);
	}

	//batch beginplay.
	for (ASigilBulletInstance* Instance : RetInstances)
	{
		Instance->OnBulletBeginPlay();
	}
	return RetInstances;
}

TArray<FGuid> USigilBulletSubsystem::GetIdsFromBullets(TArray<ASigilBulletInstance*> Instances)
{
	TArray<FGuid> Ids;
	for (ASigilBulletInstance* BulletInstance : Instances)
	{
		Ids.Add(BulletInstance->BulletId);
	}
	return Ids;
}

TArray<ASigilBulletInstance*> USigilBulletSubsystem::GetOrCreateBulletInstances(const FSigilBulletSpawnParameters& SpawnParameters, const FSigilBulletDefinition& Definition)
{
	TArray<ASigilBulletInstance*> OutInstances;

	static int32 MaxAllowedLoops = 30;

	int32 Counter = 0;
	while (OutInstances.Num() < Definition.BulletCount)
	{
		if (ASigilBulletInstance* Instance = TakeBulletFromPool(Definition.BulletActorClass.LoadSynchronous()))
		{
			OutInstances.Add(Instance);
		}
		else if (ASigilBulletInstance* Instance2 = CreateBulletInstance(SpawnParameters, Definition))
		{
			OutInstances.Add(Instance2);
		}
		Counter++;
		if (Counter >= MaxAllowedLoops)
		{
			UE_LOG(LogSigilCombat, Warning, TEXT("BulletSubsystem reach max allowed bullet spawn loops(%d)."), MaxAllowedLoops);
			break;
		}
	}
	return OutInstances;
}

ASigilBulletInstance* USigilBulletSubsystem::TakeBulletFromPool(TSubclassOf<ASigilBulletInstance> BulletClass)
{
	int32 Found = INDEX_NONE;
	for (int i = 0; i < BulletPools.Num(); i++)
	{
		if (BulletPools[i].GetClass() == BulletClass)
		{
			Found = i;
			break;
		}
	}
	if (Found != INDEX_NONE)
	{
		ASigilBulletInstance* FoundInstance = BulletPools[Found];
		UE_LOG(LogSigilCombat, Verbose, TEXT("Taking bullet(%s) from pool."), *BulletClass->GetName());
		BulletPools.RemoveAtSwap(Found);
		return FoundInstance;
	}
	return nullptr;
}

void USigilBulletSubsystem::DestroyBullet(FGuid BulledId)
{
	if (BulletInstances.Contains(BulledId))
	{
		ASigilBulletInstance* BulletToRemove = BulletInstances[BulledId];
		BulletToRemove->SetDefinitionHandle(FDataTableRowHandle());
		BulletToRemove->SetOwner(nullptr);
		BulletInstances.Remove(BulledId);
		BulletPools.Add(BulletToRemove);
		UE_LOG(LogSigilCombat, Verbose, TEXT("Return bullet(%s) back to pool."), *BulletToRemove->GetClass()->GetName());
	}
}

ASigilBulletInstance* USigilBulletSubsystem::CreateBulletInstance(const FSigilBulletSpawnParameters& SpawnParameters, const FSigilBulletDefinition& Definition)
{
	if (Definition.BulletActorClass.IsNull())
	{
		UE_LOG(LogSigilCombat, Error, TEXT("Failed to create bullet instance for definition(%s),missing BulletActorClass!!!"), *SpawnParameters.DefinitionHandle.ToDebugString());
		return nullptr;
	}

	UClass* BulletClass = Definition.BulletActorClass.LoadSynchronous();
	check(BulletClass);
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASigilBulletInstance* NewInstance = GetWorld()->SpawnActor<ASigilBulletInstance>(BulletClass, FTransform::Identity, ActorSpawnParameters);
	if (NewInstance)
	{
		UE_LOG(LogSigilCombat, Verbose, TEXT("Create new bullet instance for class(%s)"), *BulletClass->GetName());
		return NewInstance;
	}

	UE_LOG(LogSigilCombat, Error, TEXT("Failed to create new bullet instance for class(%s)"), *BulletClass->GetName());
	return nullptr;
}

bool USigilBulletSubsystem::LoadBulletDefinition(const FDataTableRowHandle& Handle, FSigilBulletDefinition& OutDefinition)
{
	if (FSigilBulletDefinition* Definition = Handle.GetRow<FSigilBulletDefinition>(TEXT("LoadBulletDefinition")))
	{
		OutDefinition = *Definition;
		return true;
	}
	return false;
}
