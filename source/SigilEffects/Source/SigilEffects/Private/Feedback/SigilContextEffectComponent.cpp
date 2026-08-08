// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Feedback/SigilContextEffectComponent.h"

#include "GameplayTagAssetInterface.h"
#include "Engine/World.h"
#include "Feedback/SigilContextEffectsSubsystem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "SigilEffectsLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilContextEffectComponent)

class UAnimSequenceBase;
class USceneComponent;


// Sets default values for this component's properties
USigilContextEffectComponent::USigilContextEffectComponent()
{
	// Disable component tick, enable Auto Activate
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
	// ...
}


// Called when the game starts
void USigilContextEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	CurrentContexts.AppendTags(DefaultEffectContexts);
	CurrentContextEffectsLibraries = DefaultContextEffectsLibraries;

	// On Begin Play, Load and Add Context Effects pairings
	if (const UWorld* World = GetWorld())
	{
		if (USigilContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<USigilContextEffectsSubsystem>())
		{
			ContextEffectsSubsystem->LoadAndAddContextEffectsLibraries(GetOwner(), CurrentContextEffectsLibraries);
		}
	}
	if (bAutoSetupTagsProvider)
	{
		SetupTagsProvider();
	}
}

void USigilContextEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// On End PLay, remove unnecessary context effects pairings
	if (const UWorld* World = GetWorld())
	{
		if (USigilContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<USigilContextEffectsSubsystem>())
		{
			ContextEffectsSubsystem->UnloadAndRemoveContextEffectsLibraries(GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USigilContextEffectComponent::SetupTagsProvider()
{
	if (GetOwner()->GetClass()->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
	{
		SetGameplayTagsProvider(GetOwner());
	}
	else
	{
		TArray<UActorComponent*> Components = GetOwner()->GetComponentsByInterface(UGameplayTagAssetInterface::StaticClass());
		if (Components.IsValidIndex(0))
		{
			SetGameplayTagsProvider(Components[0]);
		}
	}
}


void USigilContextEffectComponent::PlayContextEffectsWithInput_Implementation(FSigilSpawnContextEffectsInput Input)
{
	AggregateContexts(Input);
	InjectPhysicalSurfaceToContexts(Input.HitResult, Input.SourceContext);

	// Prep Components
	TArray<UAudioComponent*> AudioComponentsToAdd;
	TArray<UNiagaraComponent*> NiagaraComponentsToAdd;
	TArray<UParticleSystemComponent*> ParticleComponentsToAdd;


	// Cycle through Active Audio Components and cache
	for (UAudioComponent* ActiveAudioComponent : ActiveAudioComponents)
	{
		if (ActiveAudioComponent)
		{
			AudioComponentsToAdd.Add(ActiveAudioComponent);
		}
	}

	// Cycle through Active Niagara Components and cache
	for (UNiagaraComponent* ActiveNiagaraComponent : ActiveNiagaraComponents)
	{
		if (ActiveNiagaraComponent)
		{
			NiagaraComponentsToAdd.Add(ActiveNiagaraComponent);
		}
	}

	// Cycle through Active Particle Components and cache
	for (UParticleSystemComponent* ActiveParticleComponent : ActiveParticleComponents)
	{
		if (ActiveParticleComponent)
		{
			ParticleComponentsToAdd.Add(ActiveParticleComponent);
		}
	}

	// Get World
	if (const UWorld* World = GetWorld())
	{
		// Get Subsystem
		if (USigilContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<USigilContextEffectsSubsystem>())
		{
			// Set up components
			FSigilSpawnContextEffectsOutput Output;

			// Spawn effects
			ContextEffectsSubsystem->SpawnContextEffectsExt(GetOwner(), Input, Output);

			// Append resultant effects
			AudioComponentsToAdd.Append(Output.AudioComponents);
			NiagaraComponentsToAdd.Append(Output.NiagaraComponents);
			ParticleComponentsToAdd.Append(Output.ParticlesComponents);
		}
	}

	// Append Active Audio Components
	ActiveAudioComponents.Empty();
	ActiveAudioComponents.Append(AudioComponentsToAdd);

	// Append Active
	ActiveNiagaraComponents.Empty();
	ActiveNiagaraComponents.Append(NiagaraComponentsToAdd);

	ActiveParticleComponents.Empty();
	ActiveParticleComponents.Append(ParticleComponentsToAdd);
}

void USigilContextEffectComponent::AggregateContexts(FSigilSpawnContextEffectsInput& Input) const
{
	if (Input.SourceContextType == ESigilEffectsContextType::Merge)
	{
		FGameplayTagContainer TotalContexts;
		// Aggregate contexts
		TotalContexts.AppendTags(Input.SourceContext);
		TotalContexts.AppendTags(CurrentContexts);
		if (IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(GameplayTagsProvider))
		{
			FGameplayTagContainer RetTags;
			TagAssetInterface->GetOwnedGameplayTags(RetTags);
			TotalContexts.AppendTags(RetTags);
		}
		Input.SourceContext = TotalContexts;
	}
}


void USigilContextEffectComponent::InjectPhysicalSurfaceToContexts(const FHitResult& InHitResult, FGameplayTagContainer& Contexts)
{
	// Check if converting Physical Surface Type to Context
	if (!bConvertPhysicalSurfaceToContext)
	{
		return;
	}

	// Get Phys Mat Type Pointer
	TWeakObjectPtr<UPhysicalMaterial> PhysicalSurfaceTypePtr = InHitResult.PhysMaterial;

	// Check if pointer is okay
	if (PhysicalSurfaceTypePtr.IsValid())
	{
		// Get the Surface Type Pointer
		TEnumAsByte<EPhysicalSurface> PhysicalSurfaceType = PhysicalSurfaceTypePtr->SurfaceType;

		// If Settings are valid
		if (const USigilContextEffectsSettings* ContextEffectsSettings = GetDefault<USigilContextEffectsSettings>())
		{
			if (ContextEffectsSettings->SurfaceTypeToContextMap.IsEmpty())
			{
				SIGIL_EFFECTS_CLOG(Warning, "No surface type to context map, Please check ContextEffectsSetting in ProjectSettings!");
				if (FallbackPhysicalSurface.IsValid())
				{
					Contexts.AddTag(FallbackPhysicalSurface);
				}
			}
			else
			{
				// Convert Surface Type to known
				if (const FGameplayTag* SurfaceContextPtr = ContextEffectsSettings->SurfaceTypeToContextMap.Find(PhysicalSurfaceType))
				{
					FGameplayTag SurfaceContext = *SurfaceContextPtr;

					Contexts.AddTag(SurfaceContext);
				}
				else
				{
					SIGIL_EFFECTS_CLOG(Warning, "No surface type(%d) to context map found, Please check ContextEffectsSetting in ProjectSettings!", PhysicalSurfaceType.GetValue());
					if (FallbackPhysicalSurface.IsValid())
					{
						Contexts.AddTag(FallbackPhysicalSurface);
					}
				}
			}
		}
	}
	else
	{
		if (FallbackPhysicalSurface.IsValid())
		{
			Contexts.AddTag(FallbackPhysicalSurface);
		}
	}
}

void USigilContextEffectComponent::SetGameplayTagsProvider(UObject* Provider)
{
	if (!IsValid(Provider))
	{
		return;
	}
	if (IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(Provider))
	{
		GameplayTagsProvider = Provider;
	}
	else
	{
		SIGIL_EFFECTS_CLOG(Warning, "Passed in GameplayTagsProvider(%s) Doesn't implement GameplayTagAssetInterface, it can't provide gameplay tags.", *GetNameSafe(Provider->GetClass()));
	}
}

void USigilContextEffectComponent::UpdateEffectContexts(FGameplayTagContainer NewEffectContexts)
{
	// Reset and update
	CurrentContexts.Reset(NewEffectContexts.Num());
	CurrentContexts.AppendTags(NewEffectContexts);
}

void USigilContextEffectComponent::UpdateLibraries(
	TSet<TSoftObjectPtr<USigilContextEffectsLibrary>> NewContextEffectsLibraries)
{
	// Clear out existing Effects
	CurrentContextEffectsLibraries = NewContextEffectsLibraries;

	// Get World
	if (const UWorld* World = GetWorld())
	{
		// Get Subsystem
		if (USigilContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<USigilContextEffectsSubsystem>())
		{
			// Load and Add Libraries to Subsystem                  
			ContextEffectsSubsystem->LoadAndAddContextEffectsLibraries(GetOwner(), CurrentContextEffectsLibraries);
		}
	}
}
