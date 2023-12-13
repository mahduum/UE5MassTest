// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "PlaceableSpawnerSpectatorPawn.h"

#include "MassEntityConfigAsset.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "MassExecutor.h"
#include "MassSpawnerSubsystem.h"
#include "MassSpawnLocationProcessor.h"
#include "MassTestFragments.h"
#include "PlaceablesObserver.h"
#include "MassEntity/Private/MassArchetypeData.h"
#include "Misc/OutputDeviceDebug.h"


struct FMassEntityConfig;
// Sets default values
APlaceableSpawnerSpectatorPawn::APlaceableSpawnerSpectatorPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlaceableSpawnerSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
	InitProcessor = NewObject<UMassSpawnLocationProcessor>();
}

// Called every frame
void APlaceableSpawnerSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// const UWorld& World = *(this->GetWorld());
	// FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	//
	// FMassProcessingContext Context(EntityManager, DeltaTime);
	// UE::Mass::Executor::Run(*InitProcessor, Context);//provide context and systems
}

// Called to bind functionality to input
void APlaceableSpawnerSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlaceableSpawnerSpectatorPawn::SpawnEntityFromEntityManager(FVector InLocation)
{
	const UWorld& World = *(this->GetWorld());

	if (const FMassEntityTemplate& EntityTemplate = EntityConfig->GetConfig().GetOrCreateEntityTemplate(*(this->GetWorld()), *this); EntityTemplate.IsValid())
	{
		const FMassArchetypeHandle Archetype = EntityTemplate.GetArchetype();

		//EntityTemplate.GetCompositionDescriptor()//todo create from composition

		FMassEntityManager& Manager = UE::Mass::Utils::GetEntityManagerChecked(World);
		Manager.Initialize();
		UE_LOG(LogPlaceableSpawner, Display, TEXT("Entity manager retrieved."));

		//todo check if it will work with custom processor
		const FMassEntityHandle NewItem = Manager.CreateEntity(Archetype);//todo designed to be used by subsystem... ??? Created this way won't have observers
		//todo try attach visual with: FFarmVisualFragment
		Manager.GetFragmentDataChecked<FPlaceableFragment>(NewItem).Index = 99;
		
		UE_LOG(LogPlaceableSpawner, Display, TEXT("Created new entity with archetype. Archetype is valid: %d. Total entities of archetype count: %d, all entities: %d"),
			Archetype.IsValid(), Manager.DebugGetArchetypeEntitiesCount(Archetype), Manager.DebugGetEntityCount());

		FTransform Transform;
		Transform.SetLocation(InLocation);
		
		FTransform& MutableTransform = Manager.GetFragmentDataChecked<FTransformFragment>(NewItem).GetMutableTransform();
		MutableTransform.SetLocation(InLocation);
		MutableTransform.SetScale3D(FVector(1000.0f, 1000.0f, 1000.0f));

		const FMassArchetypeCompositionDescriptor& Composition = Manager.GetArchetypeComposition(Archetype);
		FMassObserverManager Observer = Manager.GetObserverManager();
		bool HasObservers = Observer.HasObserversForComposition(Composition, EMassObservedOperation::Add);
		UE_LOG(LogPlaceableSpawner, Display, TEXT("Entity manager direct spawn set fragment location to: %s, has observers: %d, is entity built: %d"),
			*Manager.GetFragmentDataChecked<FTransformFragment>(NewItem).GetTransform().GetLocation().ToString(), HasObservers, Manager.IsEntityBuilt(NewItem));

		FMassProcessingContext ProcessingContext(Manager, /*TimeDelta=*/0.0f);
		//EntityTemplate.DebugGetArchetypeDescription()
		UMassSpawnerSubsystem* MassSpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(this->GetWorld());
		//UE::Mass::Executor::Run(*InitProcessor, ProcessingContext);
	}
}

void APlaceableSpawnerSpectatorPawn::SpawnEntityFromEntitySpawnSubsystem(FVector InLocation)
{
	UMassSpawnerSubsystem* MassSpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(this->GetWorld());
	
	const TSubclassOf<UMassProcessor> SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
	FInstancedStruct SpawnData;
	SpawnData.InitializeAs<FMassTransformsSpawnData>();
	FMassTransformsSpawnData& Transforms = SpawnData.GetMutable<FMassTransformsSpawnData>();
	
	Transforms.Transforms.Reserve(NumberToSpawn);
	FTransform& Transform = Transforms.Transforms.AddDefaulted_GetRef();
	Transform.SetLocation(InLocation);
	
	UE_LOG(LogPlaceableSpawner, Display, TEXT("Spawn location to Entity Spawn System: %s"), *Transform.GetLocation().ToString());
	
	if (const FMassEntityTemplate& EntityTemplate = EntityConfig->GetConfig().GetOrCreateEntityTemplate(*(this->GetWorld()), *this); EntityTemplate.IsValid())
	{
		TArray<FMassEntityHandle> SpawnedHandles;
		//SpawnedHandles.AddDefaulted_GetRef();
		MassSpawnerSubsystem->SpawnEntities(EntityTemplate.GetTemplateID(), NumberToSpawn, SpawnData, SpawnDataProcessor, SpawnedHandles);
		FMassEntityManager& Manager = UE::Mass::Utils::GetEntityManagerChecked(*(this->GetWorld()));
		Manager.Initialize();
		UE_LOG(LogPlaceableSpawner, Display, TEXT("Current entities count for archetype: %d, all entities: %d, spawned handle: %d, is set: %d, is valid: %d"), Manager.DebugGetArchetypeEntitiesCount(EntityTemplate.GetArchetype()), Manager.DebugGetEntityCount(), SpawnedHandles[0].Index, SpawnedHandles[0].IsSet(), SpawnedHandles[0].IsValid());
	}
}

