// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Bullet/GCS_BulletSubsystem.h"

#include "GCS_LogChannels.h"
#include "Bullet/GCS_BulletInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"


FString FGCS_BulletSpawnParameters::ToDebugString() const
{
	return FString::Format(TEXT("Owner:{0} DefinitionHandle:{1}"), {Owner ? Owner->GetName() : "Null", DefinitionHandle.ToDebugString()});
}

TArray<AGCS_BulletInstance*> UGCS_BulletSubsystem::SpawnBullets(const FGCS_BulletSpawnParameters& SpawnParameters)
{
	TArray<AGCS_BulletInstance*> RetInstances{};

	FGCS_BulletDefinition Definition;
	if (SpawnParameters.DefinitionHandle.IsNull() || !LoadBulletDefinition(SpawnParameters.DefinitionHandle, Definition))
	{
		return RetInstances;
	}

	RetInstances = GetOrCreateBulletInstances(SpawnParameters, Definition);

	for (int i = 0; i < RetInstances.Num(); ++i)
	{
		AGCS_BulletInstance* Instance = RetInstances[i];

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
			// if (AGCS_BulletInstance* ParentBulletInstance = BulletInstances[SpawnParameters.ParentId])
			// {
			// 	if (ParentBulletInstance->Definition.bUseSharedHitList)
			// 	{
			// 		Instance->HitActors = ParentBulletInstance->HitActors;
			// 	}
			// }
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
	for (AGCS_BulletInstance* Instance : RetInstances)
	{
		Instance->OnBulletBeginPlay();
	}
	return RetInstances;
}

TArray<FGuid> UGCS_BulletSubsystem::GetIdsFromBullets(TArray<AGCS_BulletInstance*> Instances)
{
	TArray<FGuid> Ids;
	for (AGCS_BulletInstance* BulletInstance : Instances)
	{
		Ids.Add(BulletInstance->BulletId);
	}
	return Ids;
}

TArray<AGCS_BulletInstance*> UGCS_BulletSubsystem::GetOrCreateBulletInstances(const FGCS_BulletSpawnParameters& SpawnParameters, const FGCS_BulletDefinition& Definition)
{
	TArray<AGCS_BulletInstance*> OutInstances;

	static int32 MaxAllowedLoops = 30;

	int32 Counter = 0;
	while (OutInstances.Num() < Definition.BulletCount)
	{
		if (AGCS_BulletInstance* Instance = TakeBulletFromPool(Definition.BulletActorClass.LoadSynchronous()))
		{
			OutInstances.Add(Instance);
		}
		else if (AGCS_BulletInstance* Instance2 = CreateBulletInstance(SpawnParameters, Definition))
		{
			OutInstances.Add(Instance2);
		}
		Counter++;
		if (Counter >= MaxAllowedLoops)
		{
			UE_LOG(LogGCS, Warning, TEXT("BulletSubsystem reach max allowed bullet spawn loops(%d)."), MaxAllowedLoops);
			break;
		}
	}
	return OutInstances;
}

AGCS_BulletInstance* UGCS_BulletSubsystem::TakeBulletFromPool(TSubclassOf<AGCS_BulletInstance> BulletClass)
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
		AGCS_BulletInstance* FoundInstance = BulletPools[Found];
		UE_LOG(LogGCS, Verbose, TEXT("Taking bullet(%s) from pool."), *BulletClass->GetName());
		BulletPools.RemoveAtSwap(Found);
		return FoundInstance;
	}
	return nullptr;
}

void UGCS_BulletSubsystem::DestroyBullet(FGuid BulledId)
{
	if (BulletInstances.Contains(BulledId))
	{
		AGCS_BulletInstance* BulletToRemove = BulletInstances[BulledId];
		BulletToRemove->SetDefinitionHandle(FDataTableRowHandle());
		BulletToRemove->SetOwner(nullptr);
		BulletInstances.Remove(BulledId);
		BulletPools.Add(BulletToRemove);
		UE_LOG(LogGCS, Verbose, TEXT("Return bullet(%s) back to pool."), *BulletToRemove->GetClass()->GetName());
	}
}

AGCS_BulletInstance* UGCS_BulletSubsystem::CreateBulletInstance(const FGCS_BulletSpawnParameters& SpawnParameters, const FGCS_BulletDefinition& Definition)
{
	if (Definition.BulletActorClass.IsNull())
	{
		UE_LOG(LogGCS, Error, TEXT("Failed to create bullet instance for definition(%s),missing BulletActorClass!!!"), *SpawnParameters.DefinitionHandle.ToDebugString());
		return nullptr;
	}

	UClass* BulletClass = Definition.BulletActorClass.LoadSynchronous();
	check(BulletClass);
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGCS_BulletInstance* NewInstance = GetWorld()->SpawnActor<AGCS_BulletInstance>(BulletClass, FTransform::Identity, ActorSpawnParameters);
	if (NewInstance)
	{
		UE_LOG(LogGCS, Verbose, TEXT("Create new bullet instance for class(%s)"), *BulletClass->GetName());
		return NewInstance;
	}

	UE_LOG(LogGCS, Error, TEXT("Failed to create new bullet instance for class(%s)"), *BulletClass->GetName());
	return nullptr;
}

bool UGCS_BulletSubsystem::LoadBulletDefinition(const FDataTableRowHandle& Handle, FGCS_BulletDefinition& OutDefinition)
{
	if (FGCS_BulletDefinition* Definition = Handle.GetRow<FGCS_BulletDefinition>(TEXT("LoadBulletDefinition")))
	{
		OutDefinition = *Definition;
		return true;
	}
	return false;
}
