// Fill out your copyright notice in the Description page of Project Settings.

#include "PlaceablesObserver.h"
#include "Logging/LogMacros.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassTestFragments.h"
#include "PlaceableMassSpawnLocationSubsystem.h"
#include "PlaceableTrait.h"

DEFINE_LOG_CATEGORY(LogPlaceableSpawner);

UPlaceablesObserver::UPlaceablesObserver()
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

	EntityQuery.RegisterWithProcessor(*this);
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

		//todo set location to all entities but there will be only one, or we will have provided num entities and the same amount of locations
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++ EntityIndex)
		{
			EntityLocationList[EntityIndex].GetMutableTransform().SetLocation(NewLocation + EntityLocationList[EntityIndex].GetTransform().GetLocation());
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
