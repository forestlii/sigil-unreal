// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIS_MixinContainer.h"
#include "Engine/World.h"
#include "AssetRegistry/AssetData.h"
#include "GIS_ItemFragment.h"
#include "GIS_LogChannels.h"
#include "GIS_MixinOwnerInterface.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Items/GIS_ItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_MixinContainer)

uint32 GetTypeHash(const FGIS_Mixin& Entry)
{
	return HashCombine(GetTypeHash(Entry.Target), Entry.Timestamp);
}

int32 FGIS_MixinContainer::IndexOfTarget(const UObject* Target) const
{
	if (!IsValid(Target))
	{
		return INDEX_NONE;
	}
	return AcceleratedMap.Contains(Target->GetClass()) ? AcceleratedMap[Target->GetClass()] : INDEX_NONE;
}

int32 FGIS_MixinContainer::IndexOfTargetByClass(const TSubclassOf<UObject>& TargetClass) const
{
	check(IsValid(TargetClass));

	const int32* Idx = AcceleratedMap.Find(TargetClass);
	if (Idx != nullptr)
	{
		return *Idx;
	}

	return INDEX_NONE;
}

// bool FGIS_MixinContainer::GetDataByTargetClass(const TSubclassOf<UObject>& TargetClass, FInstancedStruct& OutData) const
// {
// 	if (AcceleratedMap.Contains(TargetClass) && Mixins.IsValidIndex(AcceleratedMap[TargetClass]))
// 	{
// 		OutData = Mixins[AcceleratedMap[TargetClass]].Data;
// 		return true;
// 	}
// 	return false;
// }

bool FGIS_MixinRecord::operator==(const FGIS_MixinRecord& Other) const
{
	return TargetPath == Other.TargetPath && Data.GetScriptStruct() == Other.Data.GetScriptStruct();
}

bool FGIS_MixinRecord::IsValid() const
{
	return !TargetPath.IsEmpty() && Data.IsValid();
}

bool FGIS_MixinContainer::GetDataByTarget(const UObject* Target, FInstancedStruct& OutData) const
{
	if (AcceleratedMap.Contains(Target) && Mixins.IsValidIndex(AcceleratedMap[Target]))
	{
		OutData = Mixins[AcceleratedMap[Target]].Data;
		return true;
	}
	return false;
}

int32 FGIS_MixinContainer::SetDataForTarget(const TObjectPtr<const UObject>& Target, const FInstancedStruct& Data)
{
	if (Target == nullptr || !Data.IsValid())
	{
		return INDEX_NONE;
	}

	if (!IsObjectLoadedFromDisk(Target))
	{
		return INDEX_NONE;
	}

	if (AcceleratedMap.Contains(Target))
	{
		return UpdateDataAt(AcceleratedMap[Target], Data);
	}

	// is not valid class -> data pair.
	if (!CheckCompatibility(Target, Data))
	{
		return INDEX_NONE;
	}

	int32 Idx = Mixins.AddDefaulted();
	FGIS_Mixin& NewMixin = Mixins[Idx];
	NewMixin.Target = Target;
	NewMixin.Data = Data;
	NewMixin.Timestamp = OwningObject->GetWorld()->GetTimeSeconds();
	if (IGIS_MixinOwnerInterface* MixinOwner = Cast<IGIS_MixinOwnerInterface>(OwningObject))
	{
		MixinOwner->OnMixinDataAdded(NewMixin.Target, NewMixin.Data);
	}
	MarkItemDirty(NewMixin);
	CacheMixins();

	return Idx;
}

bool FGIS_MixinContainer::IsObjectLoadedFromDisk(const UObject* Object) const
{
	if (!IsValid(Object))
	{
		return false;
	}

	// 获取资源路径
	FSoftObjectPath AssetPath(Object);
	if (AssetPath.IsNull())
	{
		return false;
	}

	// 检查资产注册表
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(AssetPath);
	return AssetData.IsValid();
}

int32 FGIS_MixinContainer::UpdateDataByTargetClass(const TSubclassOf<UObject>& TargetClass, const FInstancedStruct& Data)
{
	if (AcceleratedMap.Contains(TargetClass))
	{
		return UpdateDataAt(AcceleratedMap[TargetClass], Data);
	}
	return INDEX_NONE;
}

int32 FGIS_MixinContainer::UpdateDataAt(const int32 Idx, const FInstancedStruct& Data)
{
	if (Idx == INDEX_NONE || !Mixins.IsValidIndex(Idx))
	{
		return INDEX_NONE;
	}

	FGIS_Mixin& Entry = Mixins[Idx];

	if (!CheckCompatibility(Entry.Target, Data))
	{
		return INDEX_NONE;
	}

	Entry.Data = Data;
	Entry.Timestamp = OwningObject->GetWorld()->GetTimeSeconds();
	if (IGIS_MixinOwnerInterface* MixinOwner = Cast<IGIS_MixinOwnerInterface>(OwningObject))
	{
		MixinOwner->OnMixinDataUpdated(Entry.Target, Entry.Data);
	}
	MarkItemDirty(Entry);
	CacheMixins();

	return Idx;
}

void FGIS_MixinContainer::RemoveDataByTargetClass(const TSubclassOf<UObject>& TargetClass)
{
	const int32 Idx = AcceleratedMap.Contains(TargetClass) ? AcceleratedMap[TargetClass] : INDEX_NONE;

	if (Idx != INDEX_NONE)
	{
		const FGIS_Mixin& Entry = Mixins[Idx];
		if (IGIS_MixinOwnerInterface* MixinOwner = Cast<IGIS_MixinOwnerInterface>(OwningObject))
		{
			MixinOwner->OnMixinDataRemoved(Entry.Target, Entry.Data);
		}
		Mixins.RemoveAt(Idx);
		MarkArrayDirty();
	}

	CacheMixins();
}

bool FGIS_MixinContainer::CheckCompatibility(const UObject* Target, const FInstancedStruct& Data) const
{
	if (!IsValid(Target) || !Data.IsValid())
	{
		return false;
	}
	if (const IGIS_MixinTargetInterface* MixinTarget = Cast<IGIS_MixinTargetInterface>(Target))
	{
		return MixinTarget->GetCompatibleMixinDataType() == Data.GetScriptStruct();
	}
	return false;
}

TArray<FInstancedStruct> FGIS_MixinContainer::GetAllData() const
{
	TArray<FInstancedStruct> AllData;
	AllData.Reserve(Mixins.Num());

	for (const FGIS_Mixin& Mixin : Mixins)
	{
		AllData.Add(Mixin.Data);
	}

	return AllData;
}

TArray<FGIS_Mixin> FGIS_MixinContainer::GetSerializableMixins() const
{
	return Mixins.FilterByPredicate([this](const FGIS_Mixin& Mixin)
	{
		if (const IGIS_MixinTargetInterface* MixinTarget = Cast<IGIS_MixinTargetInterface>(Mixin.Target))
		{
			return MixinTarget->IsMixinDataSerializable() && Mixin.Data.IsValid() && MixinTarget->GetCompatibleMixinDataType() == Mixin.Data.GetScriptStruct() && IsObjectLoadedFromDisk(Mixin.Target);
		}
		return false;
	});
}

TArray<FGIS_MixinRecord> FGIS_MixinContainer::GetSerializableMixinRecords() const
{
	TArray<FGIS_MixinRecord> Records;
	TArray<FGIS_Mixin> FilteredMixins = GetSerializableMixins();

	for (const FGIS_Mixin& FilteredMixin : FilteredMixins)
	{
		FGIS_MixinRecord Record;
		const FSoftObjectPath AssetPath = FSoftObjectPath(FilteredMixin.Target);
		Record.TargetPath = AssetPath.ToString();
		Record.Data = FilteredMixin.Data;
		Records.Add(Record);
	}

	return Records;
}

void FGIS_MixinContainer::RestoreFromRecords(const TArray<FGIS_MixinRecord>& Records)
{
	TArray<FGIS_Mixin> ConvertedMixins = ConvertRecordsToMixins(Records);
	for (const FGIS_Mixin& ConvertedMixin : ConvertedMixins)
	{
		SetDataForTarget(ConvertedMixin.Target, ConvertedMixin.Data);
	}
}

TArray<FGIS_Mixin> FGIS_MixinContainer::ConvertRecordsToMixins(const TArray<FGIS_MixinRecord>& Records)
{
	TArray<FGIS_Mixin> Ret;
	for (const FGIS_MixinRecord& Record : Records)
	{
		if (!Record.IsValid())
		{
			continue;
		}
		const FSoftObjectPath TargetPath = FSoftObjectPath(Record.TargetPath);
		const TSoftObjectPtr<UObject> TargetObjectSoftPtr = TSoftObjectPtr<UObject>(TargetPath);
		const TObjectPtr<const UObject> TargetObject = !TargetObjectSoftPtr.IsNull() ? TargetObjectSoftPtr.LoadSynchronous() : nullptr;
		if (!IsValid(TargetObject))
		{
			continue;
		}
		const IGIS_MixinTargetInterface* TargetInterface = Cast<IGIS_MixinTargetInterface>(TargetObject);
		if (TargetInterface == nullptr)
		{
			continue;
		}
		if (!TargetInterface->IsMixinDataSerializable())
		{
			GIS_LOG(Warning, "Skip restoring mixin's data, as target(%s,class:%s) existed in record no longer considered serializable!",
			        *GetNameSafe(TargetObject), *GetNameSafe(TargetObject->GetClass()));
			continue;
		}

		if (TargetInterface->GetCompatibleMixinDataType() != Record.Data.GetScriptStruct())
		{
			GIS_LOG(Warning,
			        "Skip restoring mixin's data, as target(%s,class:%s)'s data type(%s) in record no longer compatible with the new type(%s).",
			        *GetNameSafe(TargetObject), *GetNameSafe(TargetObject->GetClass()),
			        *GetNameSafe(Record.Data.GetScriptStruct()), *GetNameSafe(TargetInterface->GetCompatibleMixinDataType()));
			continue;
		}
		FGIS_Mixin Mixin;
		Mixin.Target = TargetObject;
		Mixin.Data = Record.Data;
		Ret.Add(Mixin);
	}
	return Ret;
}

TArray<FInstancedStruct> FGIS_MixinContainer::GetAllSerializableData() const
{
	TArray<FInstancedStruct> AllData;
	AllData.Reserve(Mixins.Num());

	for (const FGIS_Mixin& Mixin : Mixins)
	{
		if (const IGIS_MixinTargetInterface* MixinTarget = Cast<IGIS_MixinTargetInterface>(Mixin.Target))
		{
			if (MixinTarget->IsMixinDataSerializable())
			{
				AllData.Add(Mixin.Data);
			}
		}
	}

	return AllData;
}

void FGIS_MixinContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FGIS_Mixin& Mixin = Mixins[Index];
		if (Mixin.Timestamp != Mixin.LastReplicatedTimestamp)
		{
			Mixin.LastReplicatedTimestamp = Mixin.Timestamp;
			if (IGIS_MixinOwnerInterface* MixinOwner = Cast<IGIS_MixinOwnerInterface>(OwningObject))
			{
				MixinOwner->OnMixinDataAdded(Mixin.Target, Mixin.Data);
			}
		}
	}

	CacheMixins();
}

void FGIS_MixinContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FGIS_Mixin& Mixin = Mixins[Index];
		if (Mixin.Timestamp != Mixin.LastReplicatedTimestamp)
		{
			Mixin.LastReplicatedTimestamp = Mixin.Timestamp;
			if (IGIS_MixinOwnerInterface* MixinOwner = Cast<IGIS_MixinOwnerInterface>(OwningObject))
			{
				MixinOwner->OnMixinDataUpdated(Mixin.Target, Mixin.Data);
			}
		}
	}

	CacheMixins();
}

void FGIS_MixinContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		const FGIS_Mixin& Mixin = Mixins[Index];
		if (IGIS_MixinOwnerInterface* MixinOwner = Cast<IGIS_MixinOwnerInterface>(OwningObject))
		{
			MixinOwner->OnMixinDataRemoved(Mixin.Target, Mixin.Data);
		}
	}

	CacheMixins();
}

void FGIS_MixinContainer::CacheMixins()
{
	const uint32 MixinsHash = GetTypeHash(Mixins);
	if (MixinsHash == LastCachedHash)
	{
		// Same hash, no need to cache things again.
		return;
	}

	const int32 Size = Mixins.Num();
	AcceleratedMap.Empty(Size);

	for (int Idx = 0; Idx < Size; ++Idx)
	{
		const UObject* Target = Mixins[Idx].Target;
		AcceleratedMap.Add(Target, Idx);
	}

	LastCachedHash = GetTypeHash(Mixins);
}

bool FGIS_MixinContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FastArrayDeltaSerialize<FGIS_Mixin, FGIS_MixinContainer>(Mixins, DeltaParams, *this);
}
