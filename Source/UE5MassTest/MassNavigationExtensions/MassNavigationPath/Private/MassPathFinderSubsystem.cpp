// Fill out your copyright notice in the Description page of Project Settings.

#include "MassPathFinderSubsystem.h"

//#include "BezierUtilities.h"
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
			[this]
			(uint32 QueryID, ENavigationQueryResult::Type Result, FNavPathSharedPtr NavPathSharedPtr)
			{
				if(Result == ENavigationQueryResult::Success)
				{
					UE_VLOG_UELOG(this, LogMassNavigationExt, Log, TEXT("Mass path has been found with points count: %d"), NavPathSharedPtr->GetPathPoints().Num());
				}

				//start convert points
				TArray<FNavPathPoint> Points = NavPathSharedPtr->GetPathPoints();
				TArray<FVector> Locations;
				TArray<FPathSplinePoint> PathSplinePoints;
				Locations.SetNum(Points.Num());//only for debug

				//PathSplinePoints are what in TessellateSplineShape method are CurvePoints, here they are used directly as the nav path is not curved spline
				PathSplinePoints.SetNum(Points.Num());//todo add up vectors and right vectors when path is created, make method

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
				for (const FRegisteredMassNavigationData Data : RegisteredData)//TODO use this to fit the ULevel, must provide the ULevel!
				{
					//NOTE: matrix for transforming shapes that are in their own space: Registered.Component->GetComponentTransform().ToMatrixWithScale()
					FMassNavigationDataStorage Storage = Data.NavigationPathsData->GetStorage();
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
					for (int32 i = 0; i < NumLanes; i++)
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
				}
				
				OnPathFoundDebug.Broadcast(Result, Locations);

				//TODO now the data is added now we have to prompt systems to start path following actions, we can assign the handle to the entity with the tag selected
				
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
