// Fill out your copyright notice in the Description page of Project Settings.


#include "ClickedActorSignalProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassTestFragments.h"
#include "PlaceablesObserver.h"
#include "PlaceableTrait.h"
#include "MassSignalSubsystem.h"
#include "VisualLogger/VisualLogger.h"


UClickedActorSignalProcessor::UClickedActorSignalProcessor()
{
	//todo check if the order is correct
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Behavior);
}

void UClickedActorSignalProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FPlaceableFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	DeselectQuery = EntityQuery;
	DeselectQuery.AddTagRequirement<FIsSelectedTag>(EMassFragmentPresence::All);
	DeselectQuery.RegisterWithProcessor(*this);

	EntityQuery.AddTagRequirement<FIsSelectedTag>(EMassFragmentPresence::None);//todo check if this does not hamper deselecting?
	
	//ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);//todo it is not required here? Unless we want to signal all not selected entities?
}

/*This is called by the base class when it receives the signal we subscribed to.*/

void UClickedActorSignalProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                                                  FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntitySignals](const FMassExecutionContext& Context)
	{
		for (auto Entity : Context.GetEntities())
		{
			TArray<FName> Signals;
			EntitySignals.GetSignalsForEntity(Entity, Signals);
			auto Names = FString::JoinBy(Signals, TEXT(", "), [](const FName& Name) {return Name.ToString();});

			UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("%s: Received signals: %s"), *Entity.DebugGetDescription(), *Names);

			TransientEntitiesToSignal.Add(Entity);//used as marker for deselection
			Context.Defer().AddTag<FIsSelectedTag>(Entity);
		}
	});

	//what entities are we signalling?
	/*
	 * 1. Iterate over chunks.
	 * 2. Get subsystem if is needed. (like MassComponentHitSubsystem - uses RegisterForComponentHits)
	 * 3. From Entity signals we can retrieve all signals for given entity (because there may be more than 1, depending how many we subscribed to)
	 * 4. Context contains only the entities that received signals in the lookup, and we can check what signals given entity had received
	 *					TArray<FName> Signals;
						EntitySignals.GetSignalsForEntity(Entity, Signals);
	   5. Context also have matching queried fragments for entities
						
	 */
	//Super::SignalEntities(EntityManager, Context, EntitySignals);
	//todo run query on entieties that are selected to deselect
}

void UClickedActorSignalProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	Super::Execute(EntityManager, Context);
	
	if (TransientEntitiesToSignal.Num())
	{
		UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("Transient entities count on execute deselect query: %d, query matching entities: %d"),
		TransientEntitiesToSignal.Num(), DeselectQuery.GetNumMatchingEntities(EntityManager));
		//then clear whatever selections we had before
		DeselectQuery.ForEachEntityChunk(EntityManager, Context, [this](const FMassExecutionContext& Context)
		{
			for (const auto Entity : Context.GetEntities())
			{
				UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("%s: Has tag removed: %s"), *Entity.DebugGetDescription(), *FIsSelectedTag::StaticStruct()->GetName());
				Context.Defer().RemoveTag<FIsSelectedTag>(Entity);
			}
		});

		TransientEntitiesToSignal.Reset();
	}

	//todo: or I can also signal all the entities from query minus the one I selected?
	//todo: or I can simply have two different queries... BUT the base has already the main query... so 
}

void UClickedActorSignalProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, UE::Mass::Signals::Selected);//todo after subscribing how SignalEntities call is prompted? Delegate signal received does actual work as it is passed entites on which to do the job
}
