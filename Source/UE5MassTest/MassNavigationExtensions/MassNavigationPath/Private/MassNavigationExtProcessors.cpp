// Fill out your copyright notice in the Description page of Project Settings.


#include "MassNavigationExtProcessors.h"

#include "MassSignalSubsystem.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassNavigationDataStorageSubsystem.h"
#include "MassNavigationExtFragments.h"
#include "MassNavigationFragments.h"

#define UNSAFE_FOR_MT 1

UMassNavPathFollowProcessor::UMassNavPathFollowProcessor()
	: EntityQuery_Conditional(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	//ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);//???check
}

void UMassNavPathFollowProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
}

void UMassNavPathFollowProcessor::ConfigureQueries()
{
	EntityQuery_Conditional.AddRequirement<FMassNavigationShortPathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery_Conditional.AddRequirement<FMassNavigationPathLaneLocationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery_Conditional.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery_Conditional.AddSubsystemRequirement<UMassNavigationDataStorageSubsystem>(EMassFragmentAccess::ReadOnly);//it is now retrievable from context
}

void UMassNavPathFollowProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	if(!SignalSubsystem)
	{
		return;
	}

	//list entities to signal
	TArray<FMassEntityHandle> EntitiesToSignalShortPathDone;//todo the signal will be received by another system that will activate action move

	EntityQuery_Conditional.ForEachEntityChunk(EntityManager, Context, [this, &EntitiesToSignalShortPathDone](FMassExecutionContext& Context)
	{
		const UMassNavigationDataStorageSubsystem& NavStorageSubsystem = Context.GetSubsystemChecked<UMassNavigationDataStorageSubsystem>();

		const int32 NumEntities = Context.GetNumEntities();
		const TArrayView<FMassNavigationShortPathFragment> ShortPathList = Context.GetMutableFragmentView<FMassNavigationShortPathFragment>();
		const TArrayView<FMassNavigationPathLaneLocationFragment> PathLaneLocationList = Context.GetMutableFragmentView<FMassNavigationPathLaneLocationFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

		const float WorldDeltaTime = Context.GetDeltaTimeSeconds();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FMassNavigationShortPathFragment& ShortPath = ShortPathList[EntityIndex];
			FMassNavigationPathLaneLocationFragment& PathLaneLocation = PathLaneLocationList[EntityIndex];
			FMassMoveTargetFragment& MoveTarget = MoveTargetList[EntityIndex];
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			const float DeltaTime = WorldDeltaTime;

			bool bDisplayDebug = false;//todo how to activate mass gameplay debug???
#if WITH_MASSGAMEPLAY_DEBUG && UNSAFE_FOR_MT // this will result in bDisplayDebug == false and disabling of all the vlogs below
			bDisplayDebug = UE::Mass::Debug::IsDebuggingEntity(Entity);
			if (bDisplayDebug)
			{
				UE_VLOG(this, LogMassNavigationExt, Log, TEXT("Entity [%s] Updating nav path following"), *Entity.DebugGetDescription());
			}
#endif // WITH_MASSGAMEPLAY_DEBUG

			//at least two points to interpolate
			if (MoveTarget.GetCurrentAction() == EMassMovementAction::Move && ShortPath.NumPoints >=2)
			{
				const bool bWasDone = ShortPath.IsDone();

				//if it doesn't have steering (it has) and is still within nav boundaries:
				ShortPath.ProgressDistance += MoveTarget.DesiredSpeed.Get() * DeltaTime;

				// for (auto& Point : ShortPath.Points)
				// {
				// 	UE_LOG(LogMassNavigationExt, Log, TEXT("Point in range of short path: %f"), Point.Distance.Get());
				// }

				const FString ShortPathPointsDistances = FString::JoinBy(ShortPath.Points, TEXT(", "), [](const FMassNavigationPathPoint& Point) { return FString::Printf(TEXT("%f"),Point.Distance.Get()); });
				const FString ShortPathPointsDistancesAlongLane = FString::JoinBy(ShortPath.Points, TEXT(", "), [](const FMassNavigationPathPoint& Point) { return FString::Printf(TEXT("%f"), Point.DistanceAlongLane.Get()); });
				 UE_VLOG(this, LogMassNavigationExt, Log, TEXT("Entity [%s] starting action move, path done: %d, num points in short path: %d, progress distance: %f, short path points distances: [%s], short path points distances along path: [%s]."),
				 	*Entity.DebugGetDescription(), bWasDone, ShortPath.NumPoints, ShortPath.ProgressDistance, *ShortPathPointsDistances, *ShortPathPointsDistancesAlongLane);

				if (bWasDone == false)
				{
					const uint8 LastPointIndex = ShortPath.NumPoints - 1;
#if WITH_MASSGAMEPLAY_DEBUG
					ensureMsgf(PathLaneLocation.PathHandle == ShortPath.DebugPathLaneHandle, TEXT("Short path lane should match current lane location."));
#endif // WITH_MASSGAMEPLAY_DEBUG

					if(ShortPath.HasNoProgress())
					{
						PathLaneLocation.DistanceAlongPathLane = ShortPath.Points[0].DistanceAlongLane.Get();

						MoveTarget.Center = ShortPath.Points[0].Position;
						MoveTarget.Forward = ShortPath.Points[0].Tangent.GetVector();
						MoveTarget.DistanceToGoal = ShortPath.Points[LastPointIndex].Distance.Get();
						MoveTarget.bOffBoundaries = false;//todo implement ShortPath.Points[0].bOffLane;
						//todo how to display Verbose
						UE_CVLOG(bDisplayDebug, this, LogMassNavigationExt, Verbose, TEXT("Entity [%s] before start of nav lane %s at distance %.1f. Distance to goal: %.1f. Off Boundaries: %s"),
							*Entity.DebugGetDescription(),
							*PathLaneLocation.PathHandle.ToString(),
							PathLaneLocation.DistanceAlongPathLane,
							MoveTarget.DistanceToGoal,
							*LexToString((bool)MoveTarget.bOffBoundaries));
					}
					else if (ShortPath.IsProgressingOrAboutToBeDone())
					{
						//what point are we at given the progressed distance:
						uint8 PointIndex = 0;
						while (PointIndex < (ShortPath.NumPoints - 2))
						{
							FMassNavigationPathPoint& NextPoint = ShortPath.Points[PointIndex + 1];
							if (ShortPath.ProgressDistance <= NextPoint.Distance.Get())
							{
								break;
							}
							PointIndex++;
						}

						const FMassNavigationPathPoint& CurrPoint = ShortPath.Points[PointIndex];
						const FMassNavigationPathPoint& NextPoint = ShortPath.Points[PointIndex + 1];
						
						//get alpha progress between points in local distance
						const float T = (ShortPath.ProgressDistance - CurrPoint.Distance.Get()) / (NextPoint.Distance.Get() - CurrPoint.Distance.Get());

						//use progress alpha to interpolate total distance between two points along total lane distance
						PathLaneLocation.DistanceAlongPathLane = FMath::Min(FMath::Lerp(CurrPoint.DistanceAlongLane.Get(), NextPoint.DistanceAlongLane.Get(), T), PathLaneLocation.PathLength);

						UE_CVLOG(bDisplayDebug, this, LogMassNavigationExt, Verbose, TEXT("Entity [%s] lerping distance along lange from current: %.1f to next: %.1f, by alpha: %.1f, at path length: %.1f"),
							*Entity.DebugGetDescription(),
							CurrPoint.DistanceAlongLane.Get(),
							NextPoint.DistanceAlongLane.Get(),
							T,
							PathLaneLocation.PathLength
							);

						MoveTarget.Center = FMath::Lerp(CurrPoint.Position, NextPoint.Position, T);
						MoveTarget.Forward = FMath::Lerp(CurrPoint.Tangent.GetVector(), NextPoint.Tangent.GetVector(), T).GetSafeNormal();
						MoveTarget.DistanceToGoal = ShortPath.Points[LastPointIndex].Distance.Get() - FMath::Lerp(CurrPoint.Distance.Get(), NextPoint.Distance.Get(), T);
						MoveTarget.bOffBoundaries = false; //todo CurrPoint.bOffLane || NextPoint.bOffLane;

						UE_CVLOG(bDisplayDebug, this, LogMassNavigationExt, Verbose, TEXT("Entity [%s] along nav lane %s at distance %.1f. Distance to goal: %.1f. Off Boundaries: %s"),
							*Entity.DebugGetDescription(),
							*PathLaneLocation.PathHandle.ToString(),
							PathLaneLocation.DistanceAlongPathLane,
							MoveTarget.DistanceToGoal,
							*LexToString((bool)MoveTarget.bOffBoundaries));
					}
					else if (ShortPath.HasProgressedPastLastPoint())
					{
						PathLaneLocation.DistanceAlongPathLane = FMath::Min(ShortPath.Points[LastPointIndex].DistanceAlongLane.Get(), PathLaneLocation.PathLength);

						MoveTarget.Center = ShortPath.Points[LastPointIndex].Position;
						MoveTarget.Forward = ShortPath.Points[LastPointIndex].Tangent.GetVector();
						MoveTarget.DistanceToGoal = 0.0f;
						MoveTarget.bOffBoundaries = false;//todo ShortPath.Points[LastPointIndex].bOffLane;

						UE_CVLOG(bDisplayDebug, this, LogMassNavigationExt, Log, TEXT("Entity [%s] Finished path follow on lane %s at distance %f. Off Boundaries: %s"),
							*Entity.DebugGetDescription(), *PathLaneLocation.PathHandle.ToString(), PathLaneLocation.DistanceAlongPathLane, *LexToString((bool)MoveTarget.bOffBoundaries));

						if (bDisplayDebug)
						{
							UE_VLOG(this, LogMassNavigationExt, Log, TEXT("Entity [%s] End of nav path."), *Entity.DebugGetDescription());
						}

						// Check to see if need advance to next lane.
						if (ShortPath.NextPathLaneHandle.IsValid())
						{
							UE_VLOG(this, LogMassNavigationExt, Warning, TEXT("Entity [%s] has a valid handle to the next paht lane but crossing from path to another is not implemented!"), *Entity.DebugGetDescription());
						}

						ShortPath.bDone = true;
					}
					//send signals, how should respond?
					//how is this pocessor prompted to run?
				}
			}
			
		}
	});
}
