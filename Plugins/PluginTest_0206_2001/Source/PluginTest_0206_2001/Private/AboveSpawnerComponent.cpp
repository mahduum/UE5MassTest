// Fill out your copyright notice in the Description page of Project Settings.


#include "AboveSpawnerComponent.h"

// Sets default values for this component's properties
UAboveSpawnerComponent::UAboveSpawnerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UAboveSpawnerComponent::SpawnActorAbove()
{
	if (ActorClassToSpawn && GetOwner())
	{
		auto OwnerTransform = GetOwner()->GetTransform();
		auto CurrentLocation = OwnerTransform.GetLocation();
		CurrentLocation.Z += 500.f;
		OwnerTransform.SetLocation(CurrentLocation);

		GetWorld()->SpawnActor<AActor>(ActorClassToSpawn, OwnerTransform);
	}
}


// Called when the game starts
void UAboveSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

