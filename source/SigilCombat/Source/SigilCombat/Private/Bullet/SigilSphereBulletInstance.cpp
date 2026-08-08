// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Bullet/SigilSphereBulletInstance.h"

#include "Components/SphereComponent.h"


// Sets default values
ASigilSphereBulletInstance::ASigilSphereBulletInstance()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
}

UShapeComponent* ASigilSphereBulletInstance::GetBulletShape_Implementation() const
{
	return Sphere;
}

// Called when the game starts or when spawned
void ASigilSphereBulletInstance::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASigilSphereBulletInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

