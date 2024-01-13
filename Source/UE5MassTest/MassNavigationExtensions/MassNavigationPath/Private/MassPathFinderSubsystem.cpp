// Fill out your copyright notice in the Description page of Project Settings.

#include "MassPathFinderSubsystem.h"

//#include "BezierUtilities.h"

#include "MassActorSubsystem.h"
#include "MassEntityView.h"
#include "MassNavigationExtFragments.h"
#include "MassNavigationExtUtils.h"
#include "MassNavigationFragments.h"
#include "MassPathQuery.h"
#include "NavigationSystem.h"



static void CalculatePathLaneProgression(TConstArrayView<FVector> LanePoints, const int32 PointsBegin, const int32 PointsEnd, TArray<float>& OutLaneProgression)
{
	float TotalDist = 0.0f;
	for (int32 i = PointsBegin; i < PointsEnd - 1; i++)
	{
		OutLaneProgression[i] = TotalDist;
		TotalDist += FVector::Dist(LanePoints[i], LanePoints[i + 1]);
	}
	OutLaneProgression[PointsEnd - 1] = TotalDist;
}

void UMassPathFinderSubsystem::AsyncFindMassPath(const FVector& StartLocation, const FVector& EndLocation, const APawn* Agent)
{
	FNavPathSharedPtr p = nullptr;
	//todo this might 
	if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		const ANavigationData* NavData = NavSystem->GetDefaultNavDataInstance(FNavigationSystem::Create);

		if (!NavData)
		{
			UE_VLOG_UELOG(this, LogMassNavigationExt, Error, TEXT("Navigation system invalid!"));

			return;
		}
		// Perform asynchronous A* pathfinding from the entity's current location to the target location
		const FNavPathQueryDelegate QueryDelegate = FNavPathQueryDelegate::CreateLambda(
			[this, Agent]
			(uint32 QueryID, ENavigationQueryResult::Type Result, FNavPathSharedPtr NavPathSharedPtr)
			{
				if(Result == ENavigationQueryResult::Success)
				{
					const FString PointsCoords = FString::JoinBy(NavPathSharedPtr->GetPathPoints(), TEXT(", "), [](const FNavPathPoint& Point)
					{
						return Point.Location.ToString();
					});
					UE_VLOG_UELOG(this, LogMassNavigationExt, Log, TEXT("Mass path has been found with points count: %d, points coords: [%s]"), NavPathSharedPtr->GetPathPoints().Num(), *PointsCoords);
				}
				else
				{
					UE_VLOG_UELOG(this, LogMassNavigationExt, Warning, TEXT("Mass path has not been found, result: %s"), *UEnum::GetValueAsString(Result));
					return;
				}

				//start convert points
				TArray<FNavPathPoint> Points = NavPathSharedPtr->GetPathPoints();
				TArray<FVector> Locations;
				TArray<FPathSplinePoint> PathSplinePoints;

				//PathSplinePoints are what in TessellateSplineShape method are CurvePoints, here they are used directly as the nav path is not curved spline

				for (auto It = Points.CreateIterator(); It; ++It )
				{
					Locations.Add(It->Location);
					PathSplinePoints.Add(FPathSplinePoint(It->Location, FVector::UpVector));
				}

				//calculate forward vectors for path points

				if (PathSplinePoints.Num() <= 1)
				{
					UE_VLOG_UELOG(this, LogMassNavigationExt, Error, TEXT("Not enough points for mass navigation path, current points count: %d!"), PathSplinePoints.Num());
					return;
				}

				//forward for the first point
				PathSplinePoints[0].Forward = (PathSplinePoints[1].Position - PathSplinePoints[0].Position).GetSafeNormal();
				//forward for the last point
				PathSplinePoints[PathSplinePoints.Num()-1].Forward = (PathSplinePoints[PathSplinePoints.Num() - 1].Position - PathSplinePoints[PathSplinePoints.Num() - 2].Position).GetSafeNormal();
				
				for (int32 i = 1; i < (PathSplinePoints.Num() - 1); i++)
				{
					const auto WorldTangentForward = (PathSplinePoints[i + 1].Position - PathSplinePoints[i - 1].Position).GetSafeNormal();
					PathSplinePoints[i].Forward = WorldTangentForward;
				}

				//end convert points

				TConstArrayView<FRegisteredMassNavigationData> RegisteredData = NavStorageSubsystem->GetRegisteredMassNavigationData();
				for (const FRegisteredMassNavigationData Data : RegisteredData)//TODO use this to fit the ULevel, MUST PROVIDE the ULevel!***************************************************
				{
					//NOTE: matrix for transforming shapes that are in their own space: Registered.Component->GetComponentTransform().ToMatrixWithScale()
					FMassNavigationDataStorage& Storage = Data.NavigationPathsData->GetStorageMutable();
					const int32 PathIndex = Storage.Paths.Num();
					//implement query bounds for paths todo
					//TODO make path and set data, add handle, as done in TessellatePolygonShape
					//Initialize data as in FZoneGraphBuilder::AppendShapeToZoneStorage:
					FPathData NewPathData = Storage.Paths.AddDefaulted_GetRef();
					NewPathData.LanesBegin = Storage.PathLanes.Num();

					//Adding single lane: //todo consider adding some kind of profile settings as for zone graph lanes
					const int32 NumLanes = 1;
					const uint16 FirstPointID = 0;
					const uint16 LastPointID = uint16(Points.Num() - 1);

					//tessellate bezier - for now there is no bezier so we just leave the points with sharp corners as they are, there is no tangent data to calculate control points for bezier  

					TArray<FPathSplinePoint> PathLanePoints;
					
					float CurWidth = 0.0f;
					for (int32 i = 0; i < NumLanes; i++)//todo remove the loop since is will only be one here
					{
						//todo consider use path lane profiles
						FPathLaneData& PathLane = Storage.PathLanes.AddDefaulted_GetRef();
						PathLane.PathIndex = PathIndex;
						PathLane.Width = 50.0f;//todo move to settings
						PathLane.StartEntryId = FirstPointID;
						PathLane.EndEntryId = LastPointID;
						CurWidth += PathLane.Width;
						
						// todo consider add internal adjacent links.
						//const int32 CurrentLaneIndex = Storage.PathLanes.Num() - 1;
						//AddAdjacentLaneLinks(CurrentLaneIndex, i, LaneProfile.Lanes, OutInternalLinks);

						//Only for single lane:
						const float HalfWidth = PathLane.Width * 0.5f;

						//lane center, for now use it unchanged for single lane centered along the path and later if forward-backward movement paired lane will be available
						const float LanePos = HalfWidth - (CurWidth + PathLane.Width * 0.5f);
						PathLanePoints.Reset();
						//for offset if more lanes than 1
						for (int32 j = 0; j < PathSplinePoints.Num(); j++)
						{
							const auto& Point = PathSplinePoints[j];
							auto& OffsetPoint = PathLanePoints.Add_GetRef(Point);
							OffsetPoint.Position += LanePos * FVector::Zero();//no offset for now, just a centered lane.
						}

						PathLane.PointsBegin = Storage.PathLanePoints.Num();

						for (const auto& Point : PathLanePoints)
						{
							Storage.PathLanePoints.Add(Point.Position);
							Storage.PathLaneUpVectors.Add(Point.Up);
							Storage.PathLaneTangentVectors.Add(Point.Forward);
						}
						
						PathLane.PointsEnd = Storage.PathLanePoints.Num();
					}

					NewPathData.LanesEnd = Storage.PathLanes.Num();

					//progressions
					Storage.PathLanePointsProgressions.AddZeroed(Storage.PathLanePoints.Num() - Storage.PathLanePointsProgressions.Num());
					for (int32 i = NewPathData.LanesBegin; i < NewPathData.LanesEnd; i++)
					{
						const auto& PathLane = Storage.PathLanes[i];
						CalculatePathLaneProgression(Storage.PathLanePoints, PathLane.PointsBegin, PathLane.PointsEnd, Storage.PathLanePointsProgressions);
					}

					//TODO now the data is added now we have to prompt systems to start path following actions, we can assign the handle to the entity with the tag selected

					/* Requesting move section */
					if(Agent == nullptr)
					{
						UE_VLOG_UELOG(this, LogMassNavigationExt, Error, TEXT("Provided null pointer for pawn!"));
						return;
					}
					
					const FMassEntityHandle Entity = ActorSubsystem->GetEntityHandleFromActor(Agent);//todo this could be also attached/inserted in path data
					//if I signal entity then i cannot send payload,
					//otherwise signaled entity with a specific signal will trigger on received on a given processor
					//after signalling through the subsystem, entities are put into a buffer and subscribed processor in Execute will run overriden SignalEntites method on them
					//signalled entity may look for the path by itself?
					//with signaling entity is more effective and faster, because we are not iterating through entities
					//can we pass path index to processor? we can through aux data. we can also pass in the restricted amount of entities.
					//processor will only set the data for path following, and other processor will update data.
	
					//set external data directly and then check if the processor will run by itself (set appropriate move type "Move" - check how it went)
					//setting only location fragment, short path will be set in other way?:
					FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*((this->GetWorld())));
					const FMassEntityView EntityView(EntityManager, Entity);
					//TODO get all the fragments like they are in task follow path and request action, ref UMassZoneGraphNavigationTrait for fragments in trait
					FMassNavigationPathLaneLocationFragment& PathLaneLocationFragment = EntityView.GetFragmentData<FMassNavigationPathLaneLocationFragment>();
					FMassMoveTargetFragment& MoveTargetFragment = EntityView.GetFragmentData<FMassMoveTargetFragment>();
					FMassNavigationShortPathRequestFragment& ShortPathRequestFragment = EntityView.GetFragmentData<FMassNavigationShortPathRequestFragment>();
					FMassNavigationShortPathFragment& ShortPathFragment = EntityView.GetFragmentData<FMassNavigationShortPathFragment>();
					FMassNavigationCachedPathLaneFragment& CachedPathLaneFragment = EntityView.GetFragmentData<FMassNavigationCachedPathLaneFragment>();
					const FAgentRadiusFragment& AgentRadiusFragment = EntityView.GetFragmentData<FAgentRadiusFragment>();

					//TODO NOTE: this will be used in state tree when a different task in state tree finds for us path and puts the result in TargetLocation (with a difference
					//that it will also provide a path as there are no ready lanes in world.):
					//FMassNavigationTargetLocation RequestedTargetLocation;
					//RequestedTargetLocation.TargetDistance = Storage.PathLanePointsProgressions[Storage.PathLanePointsProgressions.Num() - 1];
					//InstanceData.EscapeTargetLocation.TargetDistance = LaneLocation.DistanceAlongLane + MoveDistance * MoveDir; - target distance is partial in zone graph
		
				
					
					//in assigning this zone graph subsystem would look up for the nearest lane and take handle from it
					//even if entity is currently moving it will still be withing navigable path to the point from which the new path starts
					//but we must stop old path from being followed, we must prompt a reset/new setup like in the path follow
					//so first we must signal entities to stop following path or lane changed something like:
					//MassCrowdSubsystem.OnEntityLaneChanged(Context.GetEntity(EntityIndex), LaneTracking.TrackedLaneHandle, LaneLocation.LaneHandle); or:
					//but also signal paths done.
					//TODO how to prepare mass execution context for signaling:
					//void UMassCrowdLaneTrackingSignalProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals)
					//context is narrowed down in base signal proc:
					/*
					* Context.SetEntityCollection(FMassArchetypeEntityCollection(Set.Archetype, Set.Entities, FMassArchetypeEntityCollection::FoldDuplicates));
					SignalEntities(EntityManager, Context, SignalNameLookup);
					Context.ClearEntityCollection();
					 */
	
					//with zone graph and lanes signal change if the short path is done and if the next lane exists
					//we must first abort short path, and reset path location by Reset() on activate action move
	
					//set lane location init data first and check if we really need to stop entity from moving first
					//Path handle pointed to FMassNavigationPathLaneLocation which in zone graph was the geometrical actual path lane
					//object and was only needed to retrieve new path points data.
					int32 PathLaneIndex = Storage.PathLanes.Num() - 1;
					PathLaneLocationFragment.PathHandle = FMassNavigationPathLaneHandle(PathLaneIndex, Storage.DataHandle);//FMassNavigationPathLaneLocation not set???
					float LaneLength = 0.f;
					if(!MassNavigationExt::MassNavigationPath::Query::GetLaneLength(Storage, PathLaneIndex, LaneLength))
					{
						UE_LOG(LogMassNavigationExt, Error, TEXT("Lane length not found in storage!"));
					}
					
					PathLaneLocationFragment.PathLength = LaneLength;
					
					//modify path request that is in the fragment:
					FMassNavigationShortPathRequest& ShortPathRequest = ShortPathRequestFragment.PathRequest;
					ShortPathRequest.StartPosition = MoveTargetFragment.Center;
					ShortPathRequest.bMoveReverse = false;
					ShortPathRequest.TargetDistance = Storage.PathLanePointsProgressions[Storage.PathLanePointsProgressions.Num() - 1];
					ShortPathRequest.EndOfPathIntent = EMassMovementAction::Stand;
					int32 LastPathLanePointIndex = Storage.PathLanes[PathLaneIndex].PointsEnd - 1;
					auto LastPathLanePoint = Storage.PathLanePoints[LastPathLanePointIndex];
					auto LastPathLanePointDirection = Storage.PathLaneTangentVectors[LastPathLanePointIndex];
					ShortPathRequest.EndOfPathPosition = LastPathLanePoint;
					ShortPathRequest.bIsEndOfPathPositionSet = true;
					ShortPathRequest.EndOfPathDirection = FMassSnorm8Vector(LastPathLanePointDirection);
					ShortPathRequest.bIsEndOfPathDirectionSet = true;
					ShortPathRequest.AnticipationDistance = FMassInt16Real(50.0f);
					ShortPathRequest.EndOfPathOffset.Set(FMath::RandRange(-AgentRadiusFragment.Radius, AgentRadiusFragment.Radius));

					constexpr float DesiredSpeed = 300.0f; //this is replaced by instanced data

					MoveTargetFragment.CreateNewAction(EMassMovementAction::Move, *GetWorld());
					MassNavigationExt::Actions::ActivateActionMove(*GetWorld(), this, Entity, *NavStorageSubsystem, PathLaneLocationFragment,
						ShortPathRequest, AgentRadiusFragment.Radius, DesiredSpeed, MoveTargetFragment, ShortPathFragment, CachedPathLaneFragment);
				}
				//storage.pathlanes empty
				OnPathFoundDebug.Broadcast(Result, Locations);
				
				// FMassProcessingContext ProcContext;
				// FMassEntityManager EntityManager;// = FMassEntityManager
				// FMassExecutionContext ExecContext = EntityManager.CreateExecutionContext(0.0f);
				// FMassExecutionContext ExecContext = ProcContext.EntityManager->CreateExecutionContext(0.0f);
				// FMassArchetypeEntityCollectionWithPayload CollectionWithPayload()
				// ExecContext.SetEntityCollection()

				// FStructArrayView PaloadView(Payload);
				// TArray<FMassArchetypeEntityCollectionWithPayload> Result;
				// FMassArchetypeEntityCollectionWithPayload::CreateEntityRangesWithPayload(*EntityManager, EntitiesSubSet, FMassArchetypeEntityCollection::FoldDuplicates
				// , FMassGenericPayloadView(MakeArrayView(&PaloadView, 1)), Result);
			});
		const FPathFindingQuery Query (Agent, *NavData, StartLocation, EndLocation);
		NavSystem->FindPathAsync(FNavAgentProperties::DefaultProperties/*Agent->GetNavAgentPropertiesRef()*/, Query, QueryDelegate, EPathFindingMode::Regular);
	}
}

void UMassPathFinderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	NavStorageSubsystem = Collection.InitializeDependency<UMassNavigationDataStorageSubsystem>();
	checkfSlow(AgentSubsystem != nullptr, TEXT("MassAgentSubsystem is required"));

	ActorSubsystem = Collection.InitializeDependency<UMassActorSubsystem>();
	checkfSlow(AgentSubsystem != nullptr, TEXT("MassAgentSubsystem is required"));
}

static void FlattenSplineSegments(TConstArrayView<FPathSplinePoint> Points, const float Tolerance, TArray<FPathSplinePoint>& OutPoints)
{
	const int32 NumPoints = Points.Num();
	int StartIdx = 0;
	int Idx = 1;

	//points are already in world pos so no need to transform them
	OutPoints.Add(FPathSplinePoint(Points[StartIdx].Position, Points[StartIdx].Up));
	
	TArray<FVector> TempPoints;
	TArray<float> TempProgression;

	while (Idx < NumPoints)
	{
		FVector StartPosition(ForceInitToZero), StartControlPoint(ForceInitToZero), EndControlPoint(ForceInitToZero), EndPosition(ForceInitToZero);
		//GetCubicBezierPointsFromShapeSegment(Points[StartIdx], Points[Idx], LocalToWorld, StartPosition, StartControlPoint, EndControlPoint, EndPosition);

		// TODO: The Bezier tessellation does not take into account the roll when calculating tolerance.
		// Maybe we should have a templated version which would do the up axis interpolation too.

		TempPoints.Reset();
		TempPoints.Add(StartPosition);
		//UE::CubicBezier::Tessellate(TempPoints, StartPosition, StartControlPoint, EndControlPoint, EndPosition, Tolerance);

		TempProgression.SetNum(TempPoints.Num());

		// Interpolate up vector for points
		float TotalDist = FVector::Dist(StartPosition, TempPoints[0]);
		for (int32 i = 0; i < TempPoints.Num() - 1; i++)
		{
			TempProgression[i] = TotalDist;
			TotalDist += FVector::Dist(TempPoints[i], TempPoints[i + 1]);
		}
		TempProgression[TempProgression.Num() - 1] = TotalDist;

		// Add points and interpolate up axis
		//const FQuat StartRotation = Points[StartIdx].Rotation.Quaternion();
		//const FQuat EndRotation = Points[Idx].Rotation.Quaternion();
		for (int32 i = 0; i < TempPoints.Num(); i++)
		{
			const float Alpha = TempProgression[i] / TotalDist;
			//FQuat Rotation = FMath::Lerp(StartRotation, EndRotation, Alpha);
			//const FVector WorldUp = LocalToWorld.TransformVector(Rotation.RotateVector(FVector::UpVector)).GetSafeNormal();
			//OutPoints.Add(FShapePoint(TempPoints[i], WorldUp));
		}

		StartPosition = EndPosition;
		StartIdx = Idx;
		Idx++;
	}
}
