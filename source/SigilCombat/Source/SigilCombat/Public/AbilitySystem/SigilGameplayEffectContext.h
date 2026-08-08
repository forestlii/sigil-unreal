// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "UObject/Object.h"
#include "SigilGameplayEffectContext.generated.h"

/**
 * Custom gameplay effect context for combat system.
 * 战斗系统的自定义游戏效果上下文。
 */
USTRUCT()
struct SIGILCOMBAT_API FSigilGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	/**
	 * Duplicates the context.
	 * 复制上下文。
	 * @return The duplicated context. 复制的上下文。
	 */
	virtual FGameplayEffectContext* Duplicate() const override;

	/**
	 * Gets the script struct for the context.
	 * 获取上下文的脚本结构。
	 * @return The script struct. 脚本结构。
	 */
	virtual UScriptStruct* GetScriptStruct() const override;

	/**
	 * Serializes the context for network transmission.
	 * 为网络传输序列化上下文。
	 * @param Ar The archive. 存档。
	 * @param Map The package map. 包映射。
	 * @param bOutSuccess Whether serialization succeeded. 序列化是否成功。
	 * @return True if serialization was successful. 如果序列化成功返回true。
	 */
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	/**
	 * Sets the attack definition handle.
	 * 设置攻击定义句柄。
	 * @param Handle The attack definition handle. 攻击定义句柄。
	 */
	virtual void SetAttackDefinitionHandle(const FDataTableRowHandle Handle);

	/**
	 * Gets the attack definition handle.
	 * 获取攻击定义句柄。
	 * @return The attack definition handle. 攻击定义句柄。
	 */
	virtual FDataTableRowHandle GetAttackDefinitionHandle() const;

protected:
	/**
	 * The attack definition data table.
	 * 攻击定义数据表。
	 */
	UPROPERTY()
	TObjectPtr<const UDataTable> AtkDataTable{nullptr};

	/**
	 * The row name in the attack definition table.
	 * 攻击定义表中的行名。
	 */
	UPROPERTY()
	FName AtkDataTableRowName{NAME_None};
};

/**
 * Struct traits for FSigilGameplayEffectContext.
 * FSigilGameplayEffectContext的结构特性。
 */
template <>
struct TStructOpsTypeTraits<FSigilGameplayEffectContext> : TStructOpsTypeTraitsBase2<FSigilGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

/**
 * Payload for gameplay effect context.
 * 游戏效果上下文的载荷。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilGameplayEffectContext_Payload
{
	GENERATED_BODY()

	/**
	 * Attack definition data table.
	 * 攻击定义数据表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS", meta=(RowType="/Script/SigilCombat.SigilAttackDefinition"))
	TObjectPtr<const UDataTable> AtkDataTable{nullptr};

	/**
	 * Row name in the attack definition table.
	 * 攻击定义表中的行名。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FName AtkRowName{NAME_None};

	/**
	 * Bullet definition data table.
	 * 子弹定义数据表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS", meta=(RowType="/Script/SigilCombat.SigilBulletDefinition"))
	TObjectPtr<const UDataTable> BulletDataTable{nullptr};

	/**
	 * Row name in the bullet definition table.
	 * 子弹定义表中的行名。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FName BulletRowName{NAME_None};
};
