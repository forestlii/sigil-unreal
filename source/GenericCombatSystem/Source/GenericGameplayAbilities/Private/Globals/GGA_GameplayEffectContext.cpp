// // Copyright 2025 RedMoonGames All Rights Reserved.
//
//
// #include "Globals/GGA_GameplayEffectContext.h"
//
// FGameplayEffectContext* FGGA_GameplayEffectContext::Duplicate() const
// {
// 	FGGA_GameplayEffectContext* NewContext = new FGGA_GameplayEffectContext();
// 	*NewContext = *this;
// 	if (GetHitResult())
// 	{
// 		// Does a deep copy of the hit result
// 		NewContext->AddHitResult(*GetHitResult(), true);
// 	}
// 	return NewContext;
// }
//
// UScriptStruct* FGGA_GameplayEffectContext::GetScriptStruct() const
// {
// 	return StaticStruct();
// }
//
// //Reference this:https://www.thegames.dev/?p=62
//
// bool FGGA_GameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
// {
// 	Super::NetSerialize(Ar, Map, bOutSuccess);
// 	enum RepFlag
// 	{
// 		REP_Payloads = 8,
// 		REP_MAX
// 	};
// 	uint16 RepBits = 0;
// 	if (Ar.IsSaving())
// 	{
// 		if (!Payloads.IsEmpty())
// 		{
// 			RepBits |= 1 << REP_Payloads;
// 		}
// 	}
//
// 	Ar.SerializeBits(&RepBits, REP_MAX);
//
// 	if (RepBits & 1 << REP_Payloads)
// 	{
// 		int32 ArrayNum = Payloads.Num();
// 		Ar << ArrayNum; // 序列化数组长度
// 		for (int32 i = 0; i < ArrayNum; i++)
// 		{
// 			Payloads[i].Serialize(Ar);
// 		}
// 	}
//
// 	bOutSuccess = true;
// 	return true;
// }
//
// bool FGGA_GameplayEffectContext::HasPayload(const UScriptStruct* PayloadType) const
// {
// 	bool bValid = false;
// 	if (PayloadType == nullptr)
// 	{
// 		return bValid;
// 	}
// 	FInstancedStruct Data = GetPayload(PayloadType, bValid);
// 	return bValid;
// }
//
// FInstancedStruct FGGA_GameplayEffectContext::GetPayload(const UScriptStruct* PayloadType, bool& bValid) const
// {
// 	check(PayloadType);
//
// 	FInstancedStruct SrcType;
// 	FInstancedStruct DestType;
// 	SrcType.InitializeAs(PayloadType);
//
// 	for (const FInstancedStruct& Payload : Payloads)
// 	{
// 		if (Payload.GetScriptStruct() == nullptr)
// 		{
// 			//Found invalid type.
// 			continue;
// 		}
// 		DestType.InitializeAs(Payload.GetScriptStruct());
//
// 		if (DestType.Identical(&SrcType, 0))
// 		{
// 			bValid = true;
// 			return Payload;
// 		}
// 	}
// 	bValid = false;
// 	return FInstancedStruct();
// }
//
// bool FGGA_GameplayEffectContext::SetPayload(const FInstancedStruct& NewPayload)
// {
// 	if (NewPayload.IsValid())
// 	{
// 		for (FInstancedStruct& Payload : Payloads)
// 		{
// 			if (Payload.GetScriptStruct() == NewPayload.GetScriptStruct())
// 			{
// 				Payload = NewPayload;
// 				return true;
// 			}
// 		}
// 	}
// 	return AddPayload(NewPayload);
// }
//
// bool FGGA_GameplayEffectContext::AddPayload(const FInstancedStruct& NewPayload)
// {
// 	if (NewPayload.IsValid() && !HasPayload(NewPayload.GetScriptStruct()))
// 	{
// 		Payloads.Add(NewPayload);
// 		return true;
// 	}
// 	return false;
// }
//
// bool FGGA_GameplayEffectContext::RemovePayload(const FInstancedStruct& Payload)
// {
// 	if (!Payload.IsValid())
// 	{
// 		return false;
// 	}
// 	TArray<FInstancedStruct>& TempPayloads = Payloads;
// 	for (int i = 0; i < TempPayloads.Num(); i++)
// 	{
// 		if (TempPayloads[i].GetScriptStruct() == Payload.GetScriptStruct())
// 		{
// 			TempPayloads.RemoveAt(i);
// 			return true;
// 		}
// 	}
// 	return false;
// }
//
//
// TArray<FInstancedStruct>& FGGA_GameplayEffectContext::GetPayloads()
// {
// 	return Payloads;
// }
