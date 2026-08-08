// // Copyright 2025 RedMoonGames All Rights Reserved.
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "GameplayEffectTypes.h"
// #include "Runtime/Launch/Resources/Version.h"
// #if ENGINE_MINOR_VERSION < 5
// #include "InstancedStruct.h"
// #else
// #include "StructUtils/InstancedStruct.h"
// #endif
// #include "GGA_GameplayEffectContext.generated.h"
//
//
// /**
//  * 
//  */
// USTRUCT()
// struct GENERICGAMEPLAYABILITIES_API FGGA_GameplayEffectContext : public FGameplayEffectContext
// {
// 	GENERATED_BODY()
//
// 	virtual FGameplayEffectContext* Duplicate() const override;
// 	virtual UScriptStruct* GetScriptStruct() const override;
// 	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
//
// 	bool HasPayload(const UScriptStruct* PayloadType) const;
// 	FInstancedStruct GetPayload(const UScriptStruct* PayloadType, bool& bValid) const;
// 	bool SetPayload(const FInstancedStruct& NewPayload);
// 	bool AddPayload(const FInstancedStruct& NewPayload);
// 	bool RemovePayload(const FInstancedStruct& Payload);
// 	TArray<FInstancedStruct>& GetPayloads();
//
// protected:
// 	UPROPERTY()
// 	TArray<FInstancedStruct> Payloads;
// };
//
//
// template <>
// struct TStructOpsTypeTraits<FGGA_GameplayEffectContext> : TStructOpsTypeTraitsBase2<FGGA_GameplayEffectContext>
// {
// 	enum
// 	{
// 		WithNetSerializer = true,
// 		WithCopy = true
// 	};
// };
