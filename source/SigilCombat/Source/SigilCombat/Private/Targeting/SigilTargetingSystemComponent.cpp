// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Targeting/SigilTargetingSystemComponent.h"

#include "SigilCombatLogChannels.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Camera/PlayerCameraManager.h"
#include "Net/UnrealNetwork.h"
#include "TargetingSystem/TargetingSubsystem.h"

USigilTargetingSystemComponent::USigilTargetingSystemComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

USigilTargetingSystemComponent* USigilTargetingSystemComponent::GetTargetingSystemComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<USigilTargetingSystemComponent>() : nullptr;
}

void USigilTargetingSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TargetedActor, COND_OwnerOnly);
}


// Called when the game starts
void USigilTargetingSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void USigilTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RefreshTargeting(DeltaTime);
}

void USigilTargetingSystemComponent::RefreshTargeting(float DeltaTime)
{
	if (GetOwnerRole() >= ROLE_AutonomousProxy)
	{
		if (bAutoUpdatePotentialTargets || IsValid(TargetedActor) || CheckAssassination())
		{
			RefreshPotentialTargets();
		}

		bool LocalBool = PotentialTargets.Contains(TargetedActor);

		// Current TargetedActor no longer consider valid.
		if (TargetedActor && !LocalBool)
		{
			OnLockOff();
			SetTargetedActor(nullptr);
		}
	}
}

void USigilTargetingSystemComponent::SearchForActorToTarget()
{
	RefreshPotentialTargets();
	SelectFromPotentialTargets();
	//No target found/unlocked.
	if (!IsValid(TargetedActor) && !bAutoUpdatePotentialTargets)
	{
		PotentialTargets.Empty();
	}
}

AActor* USigilTargetingSystemComponent::SelectClosestActorFromPotentialTargets(float Radius) const
{
	if (PotentialTargets.Num() == 0)
	{
		return nullptr; // Return null if the array is empty
	}

	TArray<AActor*> FilteredPotentialTargets = PotentialTargets.FilterByPredicate([&](const AActor* Actor)
	{
		return FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation()) <= Radius;
	});

	if (FilteredPotentialTargets.Num() == 0)
	{
		return nullptr;
	}

	AActor* ClosestActor = FilteredPotentialTargets[0];
	float MinDistance = FVector::Dist(GetOwner()->GetActorLocation(), ClosestActor->GetActorLocation());

	for (int i = 1; i < FilteredPotentialTargets.Num(); i++)
	{
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), FilteredPotentialTargets[i]->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestActor = FilteredPotentialTargets[i];
		}
	}

	return ClosestActor;
}

bool USigilTargetingSystemComponent::FilterActorsWithPreset(UTargetingPreset* InTargetingPreset, const TArray<AActor*> InTargets, TArray<AActor*>& OutActors)
{
	if (InTargetingPreset == nullptr)
	{
		return false;
	}

	if (UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(GetWorld()))
	{
		FTargetingSourceContext SourceContext;
		SourceContext.SourceActor = GetOwner();

		FTargetingRequestHandle TargetingHandle = UTargetingSubsystem::MakeTargetRequestHandle(TargetingPreset, SourceContext);

		if (TargetingHandle.IsValid() && InTargets.Num() > 0)
		{
			FTargetingDefaultResultsSet& TargetingResults = FTargetingDefaultResultsSet::FindOrAdd(TargetingHandle);
			for (AActor* Target : InTargets)
			{
				if (!Target)
				{
					continue;
				}

				bool bAddResult = !TargetingResults.TargetResults.ContainsByPredicate([Target](const FTargetingDefaultResultData& Data) -> bool
				{
					return (Data.HitResult.GetActor() == Target);
				});

				if (bAddResult)
				{
					FTargetingDefaultResultData* ResultData = new(TargetingResults.TargetResults) FTargetingDefaultResultData();
					ResultData->HitResult.HitObjectHandle = FActorInstanceHandle(Target);
					ResultData->HitResult.Location = Target->GetActorLocation();
				}
			}
		}


		FTargetingRequestDelegate Delegate = FTargetingRequestDelegate::CreateWeakLambda(this, [&](FTargetingRequestHandle InTargetingHandle)
		{
			TargetingSubsystem->GetTargetingResultsActors(InTargetingHandle, OutActors);
		});

		FTargetingImmediateTaskData& ImmeidateTaskData = FTargetingImmediateTaskData::FindOrAdd(TargetingHandle);
		ImmeidateTaskData.bReleaseOnCompletion = true;

		TargetingSubsystem->ExecuteTargetingRequestWithHandle(TargetingHandle, Delegate);
	}

	return !OutActors.IsEmpty();
}

void USigilTargetingSystemComponent::SetTargetedActor(AActor* NewActor)
{
	SetTargetedActor(NewActor, true);
}

void USigilTargetingSystemComponent::SetTargetedActor(AActor* NewActor, bool bSendRpc)
{
	if (GetOwnerRole() < ROLE_AutonomousProxy)
	{
		return;
	}

	if (bSendRpc)
	{
		if (GetOwnerRole() >= ROLE_Authority)
		{
			ClientSetTargetedActor(NewActor);
		}
		else
		{
			ServerSetTargetedActor(NewActor);
		}
	}

	TargetedActor = NewActor;
}

void USigilTargetingSystemComponent::ClientSetTargetedActor_Implementation(AActor* NewActor)
{
	SetTargetedActor(NewActor, false);
}

void USigilTargetingSystemComponent::ServerSetTargetedActor_Implementation(AActor* NewActor)
{
	SetTargetedActor(NewActor, false);
}

void USigilTargetingSystemComponent::OnLockOff_Implementation()
{
	OnTargetLockOffEvent.Broadcast(TargetedActor);
}

void USigilTargetingSystemComponent::OnLockOn_Implementation()
{
	OnTargetLockOnEvent.Broadcast(TargetedActor);
}

void USigilTargetingSystemComponent::OnRefreshAssassination_Implementation()
{
	OnRefreshAssassinationEvent.Broadcast();
}

void USigilTargetingSystemComponent::SelectFromPotentialTargets()
{
	if (!IsValid(TargetedActor))
	{
		TMap<AActor*, float> LocalPotentialTargets;

		for (TObjectPtr<AActor>& Elem : PotentialTargets)
		{
			if (CanBeTargeted(Elem))
			{
				LocalPotentialTargets.Add(Elem, UKismetMathLibrary::Abs(CalculateViewAngle(Elem)));
			}
		}

		if (LocalPotentialTargets.Num() > 0)
		{
			TArray<AActor*> LocalTargets;
			TArray<float> LocalAngles;
			LocalPotentialTargets.GenerateKeyArray(LocalTargets);
			LocalPotentialTargets.GenerateValueArray(LocalAngles);

			int32 MinIndex;
			const float MaxValue = FMath::Min<float>(LocalAngles, &MinIndex);
			SetTargetedActor(LocalTargets[MinIndex]);
			OnLockOn();
		}
	}
	else
	{
		OnLockOff();
		SetTargetedActor(nullptr);
	}
}

void USigilTargetingSystemComponent::RefreshPotentialTargets()
{
	if (TargetingPreset == nullptr)
	{
		return;
	}

	if (UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(GetWorld()))
	{
		FTargetingSourceContext SourceContext;
		SourceContext.SourceActor = GetOwner();
		SourceContext.SourceObject = this;

		FTargetingRequestHandle TargetingHandle = UTargetingSubsystem::MakeTargetRequestHandle(TargetingPreset, SourceContext);
		FTargetingRequestDelegate Delegate = FTargetingRequestDelegate::CreateWeakLambda(this, [this,TargetingSubsystem](FTargetingRequestHandle InTargetingHandle)
		{
			TArray<AActor*> Results;
			TargetingSubsystem->GetTargetingResultsActors(InTargetingHandle, Results);
			PotentialTargets.Empty();
			PotentialTargets = Results;
			if (CheckAssassination()) OnRefreshAssassination();
		});

		FTargetingImmediateTaskData& ImmeidateTaskData = FTargetingImmediateTaskData::FindOrAdd(TargetingHandle);
		ImmeidateTaskData.bReleaseOnCompletion = true;

		TargetingSubsystem->ExecuteTargetingRequestWithHandle(TargetingHandle, Delegate);
	}
}

bool USigilTargetingSystemComponent::CanBeTargeted_Implementation(AActor* ActorToTarget)
{
	return true;
}


void USigilTargetingSystemComponent::StaticSwitchToNewTarget(bool RightDirection)
{
	if (TargetedActor)
	{
		TMap<AActor*, float> LocalPotentialTargets;

		for (TObjectPtr<AActor>& Elem : PotentialTargets)
		{
			if (Elem != TargetedActor)
			{
				float LocalDistance = UKismetMathLibrary::Vector_Distance(GetOwner()->GetActorLocation(), Elem->GetActorLocation());

				FRotator RequiredRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), Elem->GetActorLocation());
				FRotator DeltaRotation;

				ACharacter* LocalOwnerCharacter = Cast<ACharacter>(GetOwner());

				DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(LocalOwnerCharacter->GetControlRotation(), RequiredRotation);

				if (RightDirection == 1)
				{
					if (DeltaRotation.Yaw < 0.f && DeltaRotation.Yaw > -100.f)
					{
						LocalPotentialTargets.Add(Elem, DeltaRotation.Yaw);
					}
					else
					{
						LocalPotentialTargets.Add(Elem, -10000.f);
					}
				}
				else
				{
					if (DeltaRotation.Yaw > 0.f && DeltaRotation.Yaw < 100.f)
					{
						LocalPotentialTargets.Add(Elem, DeltaRotation.Yaw);
					}
					else
					{
						LocalPotentialTargets.Add(Elem, 10000.f);
					}
				}
			}
		}

		if (LocalPotentialTargets.Num() > 0)
		{
			TArray<AActor*> LocalTargets;
			TArray<float> LocalAngles;
			LocalPotentialTargets.GenerateKeyArray(LocalTargets);
			LocalPotentialTargets.GenerateValueArray(LocalAngles);

			if (RightDirection == 1)
			{
				int32 FoundIndex;
				const float MaxValue = FMath::Max<float>(LocalAngles, &FoundIndex);
				if (MaxValue > -10000.f)
				{
					OnLockOff();
					SetTargetedActor(LocalTargets[FoundIndex]);
					OnLockOn();
				}
			}
			else
			{
				int32 FoundIndex;
				const float MinValue = FMath::Min<float>(LocalAngles, &FoundIndex);
				if (MinValue < 10000.f)
				{
					OnLockOff();
					SetTargetedActor(LocalTargets[FoundIndex]);
					OnLockOn();
				}
			}
		}
	}
}


float USigilTargetingSystemComponent::CalculateViewAngle(const AActor* TargetActor)
{
	APawn* OwningPawn = GetPawn<APawn>();
	FRotator FinalRotation = FRotator::ZeroRotator;
	if (TargetActor && OwningPawn)
	{
		FRotator RequiredRotation = UKismetMathLibrary::FindLookAtRotation(OwningPawn->GetPawnViewLocation(), TargetActor->GetActorLocation());

		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(RequiredRotation, OwningPawn->GetViewRotation());
		FinalRotation = DeltaRotation;
	}

	return UKismetMathLibrary::Abs(UKismetMathLibrary::Abs(FinalRotation.Yaw) + UKismetMathLibrary::Abs(FinalRotation.Pitch));
}

void USigilTargetingSystemComponent::AddAssassinationListen()
{
	CheckAssassinationTargetRefCount = CheckAssassinationTargetRefCount + 1;
}

void USigilTargetingSystemComponent::RemoveAssassinationListen()
{
	CheckAssassinationTargetRefCount = CheckAssassinationTargetRefCount - 1;
	if (CheckAssassinationTargetRefCount < 0) CheckAssassinationTargetRefCount = 0;
}

bool USigilTargetingSystemComponent::CheckAssassination() const
{
	return CheckAssassinationTargetRefCount > 0;
}
