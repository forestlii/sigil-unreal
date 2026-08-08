// Copyright 2025 RedMoonGames All Rights Reserved.


#include "CombatFlow/GCS_AttackRequest.h"

#include "GCS_CombatInterface.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "Utility/GCS_CombatFunctionLibrary.h"
#include "Weapon/GCS_WeaponInterface.h"

FDataTableRowHandle UGCS_AttackRequest_Base::GetAttackDefinitionHandle_Implementation() const
{
	return FDataTableRowHandle();
}

FGCS_AttackDefinition UGCS_AttackRequest_Base::GetAttackDefinition() const
{
	if (const FGCS_AttackDefinition* Def = GetAttackDefinitionHandle().GetRow<FGCS_AttackDefinition>(TEXT("Get Attack Definition from AttackRequest")))
	{
		return *Def;
	}
	return FGCS_AttackDefinition();
}

FDataTableRowHandle UGCS_AttackRequest_Melee::GetAttackDefinitionHandle_Implementation() const
{
	return AttackDefinitionHandle;
}

FGCS_BulletDefinition UGCS_AttackRequest_Bullet::GetBulletDefinition() const
{
	if (auto Def = BulletDefinitionHandle.GetRow<FGCS_BulletDefinition>(TEXT("Get Bullet Definition from AttackRequest_Bullet")))
	{
		return *Def;
	}
	return FGCS_BulletDefinition();
}

FDataTableRowHandle UGCS_AttackRequest_Bullet::GetAttackDefinitionHandle_Implementation() const
{
	return GetBulletDefinition().AttackDefinition;
}

FVector UGCS_AttackRequest_Bullet::GetPawnTargetingSourceLocation_Implementation(APawn* SourcePawn) const
{
	check(SourcePawn);

	FVector RetLocation = SourcePawn->GetActorLocation();
	if (SourceSocketName != NAME_None)
	{
		if (UMeshComponent* Mesh = UGCS_CombatFunctionLibrary::GetMainMeshComponent(SourcePawn))
		{
			if (Mesh->DoesSocketExist(SourceSocketName))
			{
				RetLocation = Mesh->GetSocketLocation(SourceSocketName);
			}
		}
		if (ACharacter* Char = Cast<ACharacter>(SourcePawn))
		{
			if (Char->GetMesh() && Char->GetMesh()->DoesSocketExist(SourceSocketName))
			{
				RetLocation = Char->GetMesh()->GetSocketLocation(SourceSocketName);
			}
		}
	}

	return RetLocation + LocationOffset;
}


FVector UGCS_AttackRequest_Bullet::GetWeaponTargetingSourceLocation_Implementation(APawn* SourcePawn) const
{
	check(SourcePawn)

	FVector RetLocation = SourcePawn->GetActorLocation();

	if (UObject* CombatImplementer = UGCS_CombatFunctionLibrary::GetCombatInterfaceImplementer(SourcePawn))
	{
		if (UObject* WeaponImplementer = IGCS_CombatInterface::Execute_GCS_GetWeapon(CombatImplementer, nullptr))
		{
			if (UPrimitiveComponent* PrimitiveComponent = IGCS_WeaponInterface::Execute_GetPrimitiveComponent(WeaponImplementer))
			{
				if (SourceWeaponSocketName != NAME_None && PrimitiveComponent->DoesSocketExist(SourceWeaponSocketName))
				{
					RetLocation = PrimitiveComponent->GetSocketLocation(SourceSocketName);
				}
				else
				{
					RetLocation = PrimitiveComponent->GetOwner()->GetActorLocation();
				}
			}
		}
	}

	return RetLocation + LocationOffset;
}

FTransform UGCS_AttackRequest_Bullet::GetTargetingTransform_Implementation(APawn* SourcePawn, EGCS_AbilityTargetingSourceType Source) const
{
	check(SourcePawn);

	// The caller should determine the transform without calling this if the mode is custom!
	check(Source != EGCS_AbilityTargetingSourceType::Custom);


	static double FocalDistance = 1024.0f;
	FVector FocalLoc;

	bool bFocalBased = SourcePawn->Controller != nullptr && (Source == EGCS_AbilityTargetingSourceType::CameraTowardsFocus || Source == EGCS_AbilityTargetingSourceType::PawnTowardsFocus || Source ==
		EGCS_AbilityTargetingSourceType::WeaponTowardsFocus);

	if (bFocalBased)
	{
		APlayerController* PC = Cast<APlayerController>(SourcePawn->Controller);
		FVector CamLoc;
		FRotator CamRot;

		if (PC != nullptr)
		{
			PC->GetPlayerViewPoint(CamLoc, CamRot);
		}
		else
		{
			CamLoc = SourceSocketName != NAME_None ? GetPawnTargetingSourceLocation(SourcePawn) : SourcePawn->GetActorLocation() + FVector(0, 0, SourcePawn->BaseEyeHeight);
			CamRot = SourcePawn->Controller->GetControlRotation();
		}

		// Determine initial focal point to 
		FVector AimDir = CamRot.Vector().GetSafeNormal();
		FocalLoc = CamLoc + (AimDir * FocalDistance);

		// valid camera and want camera's location
		if (Source == EGCS_AbilityTargetingSourceType::CameraTowardsFocus)
		{
			FVector SourceLoc = GetWeaponTargetingSourceLocation(SourcePawn);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(SourcePawn); // 忽略玩家自身
			bool bHit = false;
			if (UWorld* World = PC->GetWorld())
			{
				FHitResult CameraHit;
				bHit = World->LineTraceSingleByChannel(
					CameraHit,
					CamLoc,
					FocalLoc,
					ECC_Visibility, // 可见性通道
					QueryParams
				);
				// 更新有效目标点
				if (bHit) {
					FocalLoc = CameraHit.Location;
				}
				if (bShowDebugLine)
				{
					// 绘制摄像机到目标的射线
					const FColor DebugColor = bHit ? FColor::Green : FColor::Red;
					const float DebugDuration = 5.0f; // 显示5秒
					const float DebugThickness = 2.0f;
					// 主射线（摄像机到目标）
					DrawDebugLine(
						World,
						CamLoc,
						FocalLoc,
						DebugColor,
						false, // 不持久
						DebugDuration,
						0,
						DebugThickness
					);
					// 如果命中，绘制命中点
					if (bHit)
					{
						DrawDebugPoint(
							World,
							CameraHit.Location,
							10.0f, // 大小
							FColor::Red,
							false,
							DebugDuration
						);
						// 绘制命中法线
						DrawDebugDirectionalArrow(
							World,
							CameraHit.Location,
							CameraHit.Location + CameraHit.Normal * 50.0f,
							10.0f,
							FColor::Blue,
							false,
							DebugDuration,
							0,
							1.0f
						);
					}
					// 绘制从枪口到目标的射线（子弹路径）
					DrawDebugLine(
						World,
						SourceLoc,
						FocalLoc,
						FColor::Yellow,
						false,
						DebugDuration,
						0,
						1.0f
					);
				}
			}
			FVector ShootDirection = (FocalLoc - SourceLoc).GetSafeNormal();
			// If we're camera -> focus then we're done
			return FTransform(ShootDirection.Rotation(), SourceLoc);
		}
	}


	// valid camera nd want pawn's location.
	if (bFocalBased && Source == EGCS_AbilityTargetingSourceType::PawnTowardsFocus)
	{
		FVector SourceLoc = GetPawnTargetingSourceLocation(SourcePawn);

		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// valid camera and want weapon's location.
	if (bFocalBased && Source == EGCS_AbilityTargetingSourceType::WeaponTowardsFocus)
	{
		FVector SourceLoc = GetWeaponTargetingSourceLocation(SourcePawn);

		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// Either we want the weapon's location, or we failed to find a camera
	if (Source == EGCS_AbilityTargetingSourceType::WeaponForward || Source == EGCS_AbilityTargetingSourceType::WeaponTowardsFocus)
	{
		FVector SourceLoc = GetWeaponTargetingSourceLocation(SourcePawn);
		return FTransform(SourcePawn->GetActorQuat(), SourceLoc);
	}

	// Either we want the pawn's location, or we failed to find a camera
	if (Source == EGCS_AbilityTargetingSourceType::PawnForward || Source == EGCS_AbilityTargetingSourceType::PawnTowardsFocus)
	{
		// Either we want the pawn's location, or we failed to find a camera
		FVector SourceLoc = GetPawnTargetingSourceLocation(SourcePawn);
		return FTransform(SourcePawn->GetActorQuat(), SourceLoc);
	}

	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(SourcePawn->GetActorQuat(), SourcePawn->GetActorLocation());
}
