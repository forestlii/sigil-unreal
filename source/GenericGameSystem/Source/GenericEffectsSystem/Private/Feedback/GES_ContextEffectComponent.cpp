// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Feedback/GES_ContextEffectComponent.h"

#include "GameplayTagAssetInterface.h"
#include "Engine/World.h"
#include "Feedback/GES_ContextEffectsSubsystem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GES_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GES_ContextEffectComponent)

class UAnimSequenceBase;
class USceneComponent;


// Sets default values for this component's properties
UGES_ContextEffectComponent::UGES_ContextEffectComponent()
{
	// Disable component tick, enable Auto Activate
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
	// ...
}


// Called when the game starts
void UGES_ContextEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	CurrentContexts.AppendTags(DefaultEffectContexts);
	CurrentContextEffectsLibraries = DefaultContextEffectsLibraries;

	// On Begin Play, Load and Add Context Effects pairings
	if (const UWorld* World = GetWorld())
	{
		if (UGES_ContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<UGES_ContextEffectsSubsystem>())
		{
			ContextEffectsSubsystem->LoadAndAddContextEffectsLibraries(GetOwner(), CurrentContextEffectsLibraries);
		}
	}
	if (bAutoSetupTagsProvider)
	{
		SetupTagsProvider();
	}
}

void UGES_ContextEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// On End PLay, remove unnecessary context effects pairings
	if (const UWorld* World = GetWorld())
	{
		if (UGES_ContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<UGES_ContextEffectsSubsystem>())
		{
			ContextEffectsSubsystem->UnloadAndRemoveContextEffectsLibraries(GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UGES_ContextEffectComponent::SetupTagsProvider()
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


void UGES_ContextEffectComponent::PlayContextEffectsWithInput_Implementation(FGES_SpawnContextEffectsInput Input)
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
		if (UGES_ContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<UGES_ContextEffectsSubsystem>())
		{
			// Set up components
			FGES_SpawnContextEffectsOutput Output;

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

void UGES_ContextEffectComponent::AggregateContexts(FGES_SpawnContextEffectsInput& Input) const
{
	if (Input.SourceContextType == EGES_EffectsContextType::Merge)
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


void UGES_ContextEffectComponent::InjectPhysicalSurfaceToContexts(const FHitResult& InHitResult, FGameplayTagContainer& Contexts)
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
		if (const UGES_ContextEffectsSettings* ContextEffectsSettings = GetDefault<UGES_ContextEffectsSettings>())
		{
			if (ContextEffectsSettings->SurfaceTypeToContextMap.IsEmpty())
			{
				GES_CLOG(Warning, "No surface type to context map, Please check ContextEffectsSetting in ProjectSettings!");
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
					GES_CLOG(Warning, "No surface type(%d) to context map found, Please check ContextEffectsSetting in ProjectSettings!", PhysicalSurfaceType.GetValue());
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

void UGES_ContextEffectComponent::SetGameplayTagsProvider(UObject* Provider)
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
		GES_CLOG(Warning, "Passed in GameplayTagsProvider(%s) Doesn't implement GameplayTagAssetInterface, it can't provide gameplay tags.", *GetNameSafe(Provider->GetClass()));
	}
}

void UGES_ContextEffectComponent::UpdateEffectContexts(FGameplayTagContainer NewEffectContexts)
{
	// Reset and update
	CurrentContexts.Reset(NewEffectContexts.Num());
	CurrentContexts.AppendTags(NewEffectContexts);
}

void UGES_ContextEffectComponent::UpdateLibraries(
	TSet<TSoftObjectPtr<UGES_ContextEffectsLibrary>> NewContextEffectsLibraries)
{
	// Clear out existing Effects
	CurrentContextEffectsLibraries = NewContextEffectsLibraries;

	// Get World
	if (const UWorld* World = GetWorld())
	{
		// Get Subsystem
		if (UGES_ContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<UGES_ContextEffectsSubsystem>())
		{
			// Load and Add Libraries to Subsystem                  
			ContextEffectsSubsystem->LoadAndAddContextEffectsLibraries(GetOwner(), CurrentContextEffectsLibraries);
		}
	}
}
