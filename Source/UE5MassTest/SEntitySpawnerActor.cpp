// Fill out your copyright notice in the Description page of Project Settings.


#include "SEntitySpawnerActor.h"

// Sets default values
ASEntitySpawnerActor::ASEntitySpawnerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASEntitySpawnerActor::BeginPlay()
{
	Super::BeginPlay();

	MassSpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
}

void ASEntitySpawnerActor::SpawnEntityFromSubsystem()
{
	const FMassEntityTemplate Template = EntityConfig.GetOrCreateEntityTemplate(*GetWorld(), *this);
	check(MassSpawnerSubsystem != nullptr);

	TArray<FMassEntityHandle> OutEntities;
	MassSpawnerSubsystem->SpawnEntities(Template, 50, OutEntities);
}

// Called every frame
void ASEntitySpawnerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

