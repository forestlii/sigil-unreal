// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Bullet/GCS_SphereBulletInstance.h"

#include "Components/SphereComponent.h"


// Sets default values
AGCS_SphereBulletInstance::AGCS_SphereBulletInstance()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
}

UShapeComponent* AGCS_SphereBulletInstance::GetBulletShape_Implementation() const
{
	return Sphere;
}

// Called when the game starts or when spawned
void AGCS_SphereBulletInstance::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGCS_SphereBulletInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

