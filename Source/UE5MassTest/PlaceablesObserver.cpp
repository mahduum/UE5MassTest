// Fill out your copyright notice in the Description page of Project Settings.

#include "PlaceablesObserver.h"
#include "Logging/LogMacros.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassTestFragments.h"
#include "PlaceableMassSpawnLocationSubsystem.h"
#include "PlaceableTrait.h"
#include "Components/CapsuleComponent.h"
#include "Translators/MassCapsuleComponentTranslators.h"
#include "Translators/MassCharacterMovementTranslators.h"

DEFINE_LOG_CATEGORY(LogPlaceableSpawner);

UPlaceablesObserver::UPlaceablesObserver() : EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedType = FPlaceableFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	//bAutoRegisterWithProcessingPhases = true;//todo how is it initialized???
}

void UPlaceablesObserver::ConfigureQueries()
{
	//todo check if any of these processors get data from outside?
	EntityQuery.AddRequirement<FPlaceableFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	//EntityQuery.AddTagRequirement<FNoLocationTag>(EMassFragmentPresence::All);//todo reverse

	//EntityQuery.RegisterWithProcessor(*this);//if no entity query in constructor
}

void UPlaceablesObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE_LOG(LogPlaceableSpawner, Display, TEXT("Initializing location of spawned placeable."))
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		//const UPlaceableMassSpawnLocationSubsystem& SpawnLocationSubsystem = Context.GetSubsystemChecked<UPlaceableMassSpawnLocationSubsystem>();//todo this can be added as dependency requirement, try if it works
		check(PlaceableMassSpawnLocationSubsystem);

		const FVector NewLocation = PlaceableMassSpawnLocationSubsystem->CurrentSpawnLocation;
		const TArrayView<FTransformFragment> EntityLocationList = Context.GetMutableFragmentView<FTransformFragment>();
		auto PlaceableFragIndex = Context.GetFragmentView<FPlaceableFragment>().GetData()->Index;//todo make this generational, on each created get num entities or take the index from free
		//todo cd: list and bump the generation (every time an entity is removed its placeable id is moved to free list (Q: where is the generation stored to be bumped? We leave the data on registered list but we bump the generation up
		//todo cd: such that old queries won't take on it, but if we create new entity that it can reuse the index from the free list and it will have a different generation that will fit new queries with its new handle (gen + index)

		//TODO SELECTION: Maybe on clicked it will pass the actor ref to the signal system which in turn will run the search signal processor that will identify which clickable entity has been clicked and add a selected tag to it?

		//todo set location to all entities but there will be only one, or we will have provided num entities and the same amount of locations
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++ EntityIndex)
		{
			auto FragmentLocation = EntityLocationList[EntityIndex].GetTransform().GetLocation();
			EntityLocationList[EntityIndex].GetMutableTransform().SetLocation(NewLocation + FragmentLocation);
			uint32 ComponentId = 0;
			
			UE_LOG(LogPlaceableSpawner, Display, TEXT("Initializing location of spawned placeable with capsule comp id: %d, and location: %s"), ComponentId, *FragmentLocation.ToString());
			//Context.Defer().RemoveTag<FNoLocationTag>(Context.GetEntity(EntityIndex));//todo reverse
		}
	});
}

void UPlaceablesObserver::Initialize(UObject& InOwner)//called with the owner of entity manager when it initializes all the processors with FMassRuntimePipeline (Runtime->MassEntity in the Mass Plugin)
{
	UE_LOG(LogPlaceableSpawner, Display, TEXT("Initializing UPlaceableObserver from Initialize()."))
	Super::Initialize(InOwner);
	const UWorld* World = InOwner.GetWorld();
	PlaceableMassSpawnLocationSubsystem = UWorld::GetSubsystem<UPlaceableMassSpawnLocationSubsystem>(World);
}

//==============================CharacterMovementWrapperObserver=================================//

UCharacterMovementWrapperObserver::UCharacterMovementWrapperObserver() : EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedType = FCharacterMovementComponentWrapperFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UCharacterMovementWrapperObserver::ConfigureQueries()
{
	EntityQuery.AddRequirement<FCharacterMovementComponentWrapperFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FPlaceableFragment>(EMassFragmentAccess::ReadOnly);
}

void UCharacterMovementWrapperObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE_LOG(LogPlaceableSpawner, Display, TEXT("Initializing FCharacterMovementComponentWrapperFragments."))

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
	{
		UE_LOG(LogPlaceableSpawner, Display, TEXT("FCharacterMovementComponentWrapperFragments count on added new fragments: %d"), Context.GetNumEntities());
	});
}
