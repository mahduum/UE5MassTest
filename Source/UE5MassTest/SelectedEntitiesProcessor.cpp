// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectedEntitiesProcessor.h"

#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "PlaceablesObserver.h"
#include "PlaceableTrait.h"
#include "MassExecutionContext.h"
#include "VisualLogger/VisualLogger.h"

USelectedEntitiesProcessor::USelectedEntitiesProcessor()
{
	//todo add execution order
	bAutoRegisterWithProcessingPhases = false;
}

void USelectedEntitiesProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);//todo initialize somewhere all to stand when added
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FIsSelectedTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void USelectedEntitiesProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
	{
		const FVector MoveTargetLocation = Context.GetAuxData().Get<FVector>();
		UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("Received move target location as aux data: %s"), *MoveTargetLocation.ToString());

		float CumulativeSlackRadius = 0.01f;
		const int32 NumEntities = Context.GetNumEntities();

		const TConstArrayView<FAgentRadiusFragment> RadiusFrag = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FTransformFragment> CurrentLocation = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetsFrag = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			const FAgentRadiusFragment Radius = RadiusFrag[EntityIndex];
			CumulativeSlackRadius += Radius.Radius;
		}
		
		//auto MoveTarget = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FMassMoveTargetFragment& MoveTarget = MoveTargetsFrag[EntityIndex];
			MoveTarget.Center.Set(MoveTargetLocation.X, MoveTargetLocation.Y, MoveTargetLocation.Z);
			MoveTarget.SlackRadius = CumulativeSlackRadius;
			MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
			MoveTarget.DesiredSpeed.Set(400.0f);
			MoveTarget.DistanceToGoal = FVector::Distance(CurrentLocation[EntityIndex].GetTransform().GetLocation(), MoveTargetLocation);
			const UWorld* World = EntityManager.GetWorld();
			check(World);
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
			
			UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("Debug transform change from: %s, to: %s"), *CurrentLocation[EntityIndex].GetTransform().GetLocation().ToString(), *MoveTargetLocation.ToString());

			//todo steering requires mass movement parameters that usually reside in state tree need to link all this:
			/*
			* 	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
				EntityQuery.AddConstSharedRequirement<FMassMovingSteeringParameters>(EMassFragmentPresence::All);
				EntityQuery.AddConstSharedRequirement<FMassStandingSteeringParameters>(EMassFragmentPresence::All);

				const FMassMovementParameters& MovementParams = Context.GetExternalData(MovementParamsHandle);
				TStateTreeExternalDataHandle<FMassMovementParameters> MovementParamsHandle;
				Linker.LinkExternalData(MovementParamsHandle);

			 */
		}
		
		//todo, they can always have this fragment but with no data? It can have fragment on it but with action set to stand
		//task has this:
		/*
		 *
		 *Context.Defer().AddFragment_RuntimeCheck<FTypedElementLocalTransformColumn>(Entity);
		* 
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
			return UE::MassNavigation::ActivateActionMove(*World, Context.GetOwner(), Context.GetEntity(), ZoneGraphSubsystem, LaneLocation, PathRequest, AgentRadius.Radius, DesiredSpeed, MoveTarget, ShortPath, CachedLane);
		 */
	});
}
