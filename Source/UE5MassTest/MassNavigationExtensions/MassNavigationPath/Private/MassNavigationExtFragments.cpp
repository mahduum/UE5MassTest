#include "MassNavigationExtFragments.h"

#include "MassNavigationDataStorageSubsystem.h"
#include "MassPathQuery.h"

namespace MassNavigationExt::MassNavigationPath
{
	/*Iterator to move normal or in reverse*/
	struct FCachedPathLaneSegmentIterator
	{
		FCachedPathLaneSegmentIterator(const FMassNavigationCachedPathLaneFragment& InCachedPath, const float DistanceAlongPath, const bool bInMoveReverse)
			: CachedPath(InCachedPath)
			, SegmentInc(bInMoveReverse ? -1 : 1)
			, bMoveReverse(bInMoveReverse)
		{
			check(CachedPath.NumPoints >= 2);
			CurrentSegment = CachedPath.FindSegmentIndexAtDistance(DistanceAlongPath);
			LastSegment = bInMoveReverse ? 0 : ((int32)CachedPath.NumPoints - 2);
		}

		bool HasReachedDistance(const float Distance) const
		{
			if (CurrentSegment == LastSegment)
			{
				return true;
			}

			if (bMoveReverse)
			{
				const float SegStartDistance = CachedPath.PathLanePointProgressions[CurrentSegment].Get();
				if (Distance > SegStartDistance)
				{
					return true;
				}
			}
			else
			{
				const float SegEndDistance = CachedPath.PathLanePointProgressions[CurrentSegment + 1].Get();
				if (Distance < SegEndDistance)
				{
					return true;
				}
			}

			return false;
		}

		void Next()
		{
			if (CurrentSegment != LastSegment)
			{
				CurrentSegment += SegmentInc;
			}
		}

		const FMassNavigationCachedPathLaneFragment& CachedPath;
		int32 CurrentSegment = 0;
		int32 LastSegment = 0;
		int32 SegmentInc = 0;
		bool bMoveReverse = false;
	};

} // UE::MassMovement::ZoneGraphPath

void FMassNavigationCachedPathLaneFragment::CachePathLaneData(const FMassNavigationDataStorage& MassNavigationDataStorage,
	const FMassNavigationPathLaneHandle CurrentPathLaneHandle, const float CurrentDistanceAlongLane,
	const float TargetDistanceAlongLane, const float InflateDistance)
{
	//TODO
	//what short path holds? any lane related data? how many points original zone lane has? how are they created?
	const FPathLaneData& Lane = MassNavigationDataStorage.PathLanes[CurrentPathLaneHandle.Index];//each parallel lane is cached, each lane may have different width

	const float StartDistance = FMath::Min(CurrentDistanceAlongLane, TargetDistanceAlongLane);
	const float EndDistance = FMath::Max(CurrentDistanceAlongLane, TargetDistanceAlongLane);
	const float CurrentLaneLength = MassNavigationDataStorage.PathLanePointsProgressions[Lane.PointsEnd - 1];

	// If cached data contains the request part of the lane, early out.
	const float InflatedStartDistance = FMath::Max(0.0f, StartDistance - InflateDistance);
	const float InflatedEndDistance = FMath::Min(EndDistance + InflateDistance, CurrentLaneLength);

	if(ensureMsgf(PathLanePointProgressions.IsEmpty() || PathLanePoints.IsEmpty() || PathLaneTangentVectors.IsEmpty(), TEXT("Some points array are empty!")))
	{
		return;
	}
	
	if (PathLaneHandle == CurrentPathLaneHandle
		&& NumPoints > 0
		&& InflatedStartDistance >= PathLanePointProgressions[0].Get()
		&& InflatedEndDistance <= PathLanePointProgressions[NumPoints - 1].Get())
	{
		return;
	}

	Reset();
	CacheID++;

	PathLaneHandle = CurrentPathLaneHandle;
	PathLaneWidth = FMassInt16Real(Lane.Width);
	PathLaneLength = CurrentLaneLength;

	const int32 LaneNumPoints = Lane.PointsEnd - Lane.PointsBegin;
	if (ensureMsgf(LaneNumPoints < 2, TEXT("Inadequate number of lane points: %d"), LaneNumPoints))
	{
		return;
	}
	
	if (LaneNumPoints <= (int32)MaxPoints)
	{
		// If we can fit all the lane's points, just do a copy.
		NumPoints = (uint8)LaneNumPoints;
		for (int32 Index = 0; Index < (int32)NumPoints; Index++)
		{
			PathLanePoints[Index] = MassNavigationDataStorage.PathLanePoints[Lane.PointsBegin + Index];
			PathLaneTangentVectors[Index] = FMassSnorm8Vector2D(FVector2D(MassNavigationDataStorage.PathLaneTangentVectors[Lane.PointsBegin + Index]));
			PathLanePointProgressions[Index] = FMassInt16Real10(MassNavigationDataStorage.PathLanePointsProgressions[Lane.PointsBegin + Index]);
		}
	}
	else // calculate the range of points we should copy to cached lane
	{
		// Find the segment of the lane that is important and copy that.
		int32 StartSegmentIndex = 0;
		int32 EndSegmentIndex = 0;
		MassNavigationExt::MassNavigationPath::Query::CalculateLaneSegmentIndexAtDistance(MassNavigationDataStorage, CurrentPathLaneHandle, StartDistance, StartSegmentIndex);
		MassNavigationExt::MassNavigationPath::Query::CalculateLaneSegmentIndexAtDistance(MassNavigationDataStorage, CurrentPathLaneHandle, EndDistance, EndSegmentIndex);

		// Expand if close to start of a segment start.
		if ((StartSegmentIndex - 1) >= Lane.PointsBegin && (StartDistance - InflateDistance) < MassNavigationDataStorage.PathLanePointsProgressions[StartSegmentIndex])
		{
			StartSegmentIndex--;
		}
		// Expand if close to end segment end.
		if ((EndSegmentIndex + 1) < (Lane.PointsEnd - 2) && (EndDistance + InflateDistance) > MassNavigationDataStorage.PathLanePointsProgressions[EndSegmentIndex + 1])
		{
			EndSegmentIndex++;
		}
	
		NumPoints = (uint8)FMath::Min((EndSegmentIndex - StartSegmentIndex) + 2, (int32)MaxPoints);

		for (int32 Index = 0; Index < (int32)NumPoints; Index++)
		{
			check((StartSegmentIndex + Index) >= Lane.PointsBegin && (StartSegmentIndex + Index) < Lane.PointsEnd);
			PathLanePoints[Index] = MassNavigationDataStorage.PathLanePoints[StartSegmentIndex + Index];
			PathLaneTangentVectors[Index] = FMassSnorm8Vector2D(FVector2D(MassNavigationDataStorage.PathLaneTangentVectors[StartSegmentIndex + Index]));
			PathLanePointProgressions[Index] = FMassInt16Real10(MassNavigationDataStorage.PathLanePointsProgressions[StartSegmentIndex + Index]);
		}
	}

	// TODO add this section after basic functionality works!
	// Calculate extra space around the lane on adjacent lanes. TODO to use on crossed paths
	// TArray<FZoneGraphLinkedLane> LinkedLanes;
	// TODO implement for mass path query
	// UE::ZoneGraph::Query::GetLinkedLanes(MassNavigationDataStorage, CurrentPathLaneHandle, EZoneLaneLinkType::Adjacent, EZoneLaneLinkFlags::Left|EZoneLaneLinkFlags::Right, EZoneLaneLinkFlags::None, LinkedLanes);

	// float AdjacentLeftWidth = 0.0f;
	// float AdjacentRightWidth = 0.0f;
	// for (const FZoneGraphLinkedLane& LinkedLane : LinkedLanes)
	// {
	// 	if (LinkedLane.HasFlags(EZoneLaneLinkFlags::Left))
	// 	{
	// 		const FPathLaneData& AdjacentLane = MassNavigationDataStorage.PathLanes[LinkedLane.DestLane.Index];
	// 		AdjacentLeftWidth += AdjacentLane.Width;
	// 	}
	// 	else if (LinkedLane.HasFlags(EZoneLaneLinkFlags::Right))
	// 	{
	// 		const FPathLaneData& AdjacentLane = MassNavigationDataStorage.PathLanes[LinkedLane.DestLane.Index];
	// 		AdjacentRightWidth += AdjacentLane.Width;
	// 	}
	// }
	// PathLaneLeftSpace = FMassInt16Real(AdjacentLeftWidth);
	// PathLaneRightSpace = FMassInt16Real(AdjacentRightWidth);
}

bool FMassNavigationShortPathFragment::RequestPath(const FMassNavigationCachedPathLaneFragment& CachedPathLane,
	const FMassNavigationShortPathRequest& Request, const float InCurrentDistanceAlongLane,
	const float AgentRadius)
{
	Reset();

	if (CachedPathLane.NumPoints < 2)
	{
		return false;
	}


	// The current distance can come from a quantized lane distance. Check against quantized bounds, but clamp it to the actual path length when calculating the path.
	static_assert(std::is_same_v<decltype(FMassNavigationPathPoint::Distance), FMassInt16Real>, "Assuming FMassNavigationPathPoint::Distance is quantized to 10 units.");
	const float LaneLengthQuantized = FMath::CeilToFloat(CachedPathLane.PathLaneLength / 10.0f) * 10.0f;

	static constexpr float Epsilon = 0.1f;
	ensureMsgf(InCurrentDistanceAlongLane >= -Epsilon && InCurrentDistanceAlongLane <= (LaneLengthQuantized + Epsilon), TEXT("Current distance %f should be within the lane bounds 0.0 - %f"), InCurrentDistanceAlongLane, LaneLengthQuantized);

	const float CurrentDistanceAlongLane = FMath::Min(InCurrentDistanceAlongLane, CachedPathLane.PathLaneLength);
	
	// Set common lane parameters
#if WITH_MASSGAMEPLAY_DEBUG
	DebugPathLaneHandle = CachedPathLane.PathLaneHandle;
#endif

	bMoveReverse = Request.bMoveReverse;
	EndOfPathIntent = Request.EndOfPathIntent;
	bPartialResult = false;

	const float DeflatedLaneHalfWidth = FMath::Max(0.0f, CachedPathLane.PathLaneWidth.Get() - AgentRadius) * 0.5f;
	const float DeflatedLaneLeft = DeflatedLaneHalfWidth + CachedPathLane.PathLaneLeftSpace.Get();
	const float DeflatedLaneRight = DeflatedLaneHalfWidth + CachedPathLane.PathLaneRightSpace.Get();

	const float TargetDistanceAlongLane = FMath::Clamp(Request.TargetDistance, 0.0f, CachedPathLane.PathLaneLength);
	const float MinDistanceAlongLane = FMath::Min(CurrentDistanceAlongLane, TargetDistanceAlongLane);
	const float MaxDistanceAlongLane = FMath::Max(CurrentDistanceAlongLane, TargetDistanceAlongLane);
	
	const float TangentSign = Request.bMoveReverse ? -1.0f : 1.0f;

	// Slop factors used when testing if a point is conservatively inside the lane.
	constexpr float OffLaneCapSlop = 10.0f;
	constexpr float OffLaneEdgeSlop = 1.0f;

	// Calculate how the start point relates to the corresponding location on lane.
	FVector StartPathLanePosition;
	FVector StartPathLaneTangent;
	CachedPathLane.GetPointAndTangentAtDistance(CurrentDistanceAlongLane, StartPathLanePosition, StartPathLaneTangent);
	float StartDistanceAlongPath = CurrentDistanceAlongLane;

	FVector StartPosition = Request.StartPosition;
	// Calculate start point's relation to the start point location on lane.
	const FVector StartDelta = StartPosition - StartPathLanePosition;
	const FVector StartLeftDir = FVector::CrossProduct(StartPathLaneTangent, FVector::UpVector);
	float StartPathLaneOffset = FloatCastChecked<float>(FVector::DotProduct(StartLeftDir, StartDelta), UE::LWC::DefaultFloatPrecision);
	float StartLaneForwardOffset = FloatCastChecked<float>(FVector::DotProduct(StartPathLaneTangent, StartDelta) * TangentSign, UE::LWC::DefaultFloatPrecision);
	// The point is off-lane if behind the start, or beyond the boundary.
	const bool bStartOffLane = StartLaneForwardOffset < -OffLaneCapSlop
								|| StartPathLaneOffset < -(DeflatedLaneRight + OffLaneEdgeSlop)
								|| StartPathLaneOffset > (DeflatedLaneLeft + OffLaneEdgeSlop);
	StartPathLaneOffset = FMath::Clamp(StartPathLaneOffset, -DeflatedLaneRight, DeflatedLaneLeft);

	if (bStartOffLane)
	{
		// The start point was off-lane, move the start location along the lane a bit further to have smoother connection.
		const float StartForwardOffset = FMath::Clamp(Request.AnticipationDistance.Get() + StartLaneForwardOffset, 0.0f, Request.AnticipationDistance.Get());
		StartDistanceAlongPath += StartForwardOffset * TangentSign; // Not clamping this distance intentionally so that the halfway point and clamping later works correctly.
	}

	// Calculate how the end point relates to the corresponding location on lane.
	const bool bHasEndOfPathPoint = Request.bIsEndOfPathPositionSet;
	float EndDistanceAlongPath = TargetDistanceAlongLane;
	FVector EndPathLanePosition = FVector::ZeroVector;
	FVector EndPathLaneTangent = FVector::ZeroVector;
	bool bEndOffPathLane = false;
	float EndPathLaneOffset = StartPathLaneOffset;

	if (bHasEndOfPathPoint)
	{
		// Calculate end point's relation to the end point location on lane.
		CachedPathLane.GetPointAndTangentAtDistance(TargetDistanceAlongLane, EndPathLanePosition, EndPathLaneTangent);
		const FVector EndPosition = Request.EndOfPathPosition;
		const FVector EndDelta = EndPosition - EndPathLanePosition;
		const FVector LeftDir = FVector::CrossProduct(EndPathLaneTangent, FVector::UpVector);
		EndPathLaneOffset = FloatCastChecked<float>(FVector::DotProduct(LeftDir, EndDelta), UE::LWC::DefaultFloatPrecision);
		const float EndLaneForwardOffset = FloatCastChecked<float>(FVector::DotProduct(EndPathLaneTangent, EndDelta) * TangentSign, UE::LWC::DefaultFloatPrecision);
		// The point is off-lane if further than the, or beyond the boundary.
		bEndOffPathLane = EndLaneForwardOffset > OffLaneCapSlop
						|| EndPathLaneOffset < -(DeflatedLaneRight + OffLaneEdgeSlop)
						|| EndPathLaneOffset > (DeflatedLaneLeft + OffLaneEdgeSlop);
		EndPathLaneOffset = FMath::Clamp(EndPathLaneOffset, -DeflatedLaneRight, DeflatedLaneLeft);

		// Move the end location along the lane a bit back to have smoother connection.
		const float EndForwardOffset = FMath::Clamp(Request.AnticipationDistance.Get() - EndLaneForwardOffset, 0.0f, Request.AnticipationDistance.Get());
		EndDistanceAlongPath -= EndForwardOffset * TangentSign; // Not clamping this distance intentionally so that the halfway point and clamping later works correctly.
	}

	// Clamp the path move distances to current lane. We use halfway point to split the anticipation in case it gets truncated.
	const float HalfwayDistanceAlongLane = FMath::Clamp((StartDistanceAlongPath + EndDistanceAlongPath) * 0.5f, MinDistanceAlongLane, MaxDistanceAlongLane);

	if (Request.bMoveReverse)
	{
		StartDistanceAlongPath = FMath::Clamp(StartDistanceAlongPath, HalfwayDistanceAlongLane, MaxDistanceAlongLane);
		EndDistanceAlongPath = FMath::Clamp(EndDistanceAlongPath, MinDistanceAlongLane, HalfwayDistanceAlongLane);
	}
	else
	{
		StartDistanceAlongPath = FMath::Clamp(StartDistanceAlongPath, MinDistanceAlongLane, HalfwayDistanceAlongLane);
		EndDistanceAlongPath = FMath::Clamp(EndDistanceAlongPath, HalfwayDistanceAlongLane, MaxDistanceAlongLane);
	}

	// Check if the mid path got clamped away. This can happen if start of end or both are off-mesh, or just a short path.
	const float MidPathMoveDistance = FMath::Abs(EndDistanceAlongPath - StartDistanceAlongPath); 
	const bool bHasMidPath = MidPathMoveDistance > KINDA_SMALL_NUMBER;

	// If end position is not set to a specific location, use proposed offset
	if (!bHasEndOfPathPoint)
	{
		// Slope defines how much the offset can change over the course of the path.
		constexpr float MaxLaneOffsetSlope = 1.0f / 10.0f;
		const float MaxOffset = MidPathMoveDistance * MaxLaneOffsetSlope;
		const float LaneOffset = FMath::Clamp(Request.EndOfPathOffset.Get(), -MaxOffset, MaxOffset);
		EndPathLaneOffset = FMath::Clamp(EndPathLaneOffset + LaneOffset, -DeflatedLaneRight, DeflatedLaneLeft);
	}

	// Always add off-lane start point.
	if (bStartOffLane)
	{
		FMassNavigationPathPoint& StartPoint = Points[NumPoints++];
		StartPoint.DistanceAlongLane = FMassInt16Real10(CurrentDistanceAlongLane);
		StartPoint.Position = Request.StartPosition;
		StartPoint.Tangent = FMassSnorm8Vector2D(StartPathLaneTangent * TangentSign);
		StartPoint.bOffPathLane = true;
		StartPoint.bIsPathLaneExtrema = false;

		// Update start point to be inside the lane.
		CachedPathLane.GetPointAndTangentAtDistance(StartDistanceAlongPath, StartPathLanePosition, StartPathLaneTangent);
		const FVector LeftDir = FVector::CrossProduct(StartPathLaneTangent, FVector::UpVector);
		StartPosition = StartPathLanePosition + LeftDir * StartPathLaneOffset;

		// Adjust the start point to point towards the first on-lane point.
		const FVector DirToClampedPoint = StartPosition - StartPoint.Position;
		StartPoint.Tangent = FMassSnorm8Vector2D(DirToClampedPoint.GetSafeNormal());
	}

	// The second point is added if there was no off-lane start point, or we have mid path.
	// This ensures that there's always at least one start point, and that no excess points are added if both start & end are off-lane close to each other.
	if (!bStartOffLane || bHasMidPath)
	{
		// Add first on-lane point.
		FMassNavigationPathPoint& Point = Points[NumPoints++];
		Point.DistanceAlongLane = FMassInt16Real10(StartDistanceAlongPath);
		Point.Position = StartPosition;
		Point.Tangent = FMassSnorm8Vector2D(StartPathLaneTangent * TangentSign);
		Point.bOffPathLane = false;
		Point.bIsPathLaneExtrema = false;
	}

	// Add in between points.
	const float InvDistanceRange = 1.0f / (EndDistanceAlongPath - StartDistanceAlongPath); // Used for lane offset interpolation. 
	float PrevDistanceAlongLane = StartDistanceAlongPath;

	MassNavigationExt::MassNavigationPath::FCachedPathLaneSegmentIterator SegmentIterator(CachedPathLane, StartDistanceAlongPath, Request.bMoveReverse);
	while (!SegmentIterator.HasReachedDistance(EndDistanceAlongPath))
	{
		// The segment endpoint is start when moving backwards (i.e. the segment index), and end when moving forwards.
		const int32 CurrentSegmentEndPointIndex = SegmentIterator.CurrentSegment + (SegmentIterator.bMoveReverse ? 0 : 1);
		const float DistanceAlongLane = CachedPathLane.PathLanePointProgressions[CurrentSegmentEndPointIndex].Get();

		if (FMath::IsNearlyEqual(PrevDistanceAlongLane, DistanceAlongLane) == false)
		{
			if (NumPoints < MaxPoints)
			{
				const FVector& PathLanePosition = CachedPathLane.PathLanePoints[CurrentSegmentEndPointIndex];
				const FVector PathLaneTangent = CachedPathLane.PathLaneTangentVectors[CurrentSegmentEndPointIndex].GetVector();

				const float PathLaneOffsetT = (DistanceAlongLane - StartDistanceAlongPath) * InvDistanceRange;
				const float PathLaneOffset = FMath::Lerp(StartPathLaneOffset, EndPathLaneOffset, PathLaneOffsetT);
				const FVector LeftDir = FVector::CrossProduct(PathLaneTangent, FVector::UpVector);

				FMassNavigationPathPoint& Point = Points[NumPoints++];
				Point.DistanceAlongLane = FMassInt16Real10(DistanceAlongLane);
				Point.Position = PathLanePosition + LeftDir * PathLaneOffset;
				Point.Tangent = FMassSnorm8Vector2D(PathLaneTangent * TangentSign);
				Point.bOffPathLane = false;
				Point.bIsPathLaneExtrema = false;

				PrevDistanceAlongLane = DistanceAlongLane;
			}
			else
			{
				bPartialResult = true;
				break;
			}
		}
		
		SegmentIterator.Next();
	}

	// The second last point is added if there is no end point, or we have mid path.
	// This ensures that there's always at least one end point, and that no excess points are added if both start & end are off-lane close to each other.
	if (!bHasEndOfPathPoint || bHasMidPath)
	{
		if (NumPoints < MaxPoints)
		{
			// Interpolate last point on mid path.
			FVector PathLanePosition;
			FVector PathLaneTangent;
			CachedPathLane.InterpolatePointAndTangentOnSegment(SegmentIterator.CurrentSegment, EndDistanceAlongPath, PathLanePosition, PathLaneTangent);

			const float LaneOffset = EndPathLaneOffset;
			const FVector LeftDir = FVector::CrossProduct(PathLaneTangent, FVector::UpVector);

			FMassNavigationPathPoint& Point = Points[NumPoints++];
			Point.DistanceAlongLane = FMassInt16Real10(EndDistanceAlongPath);
			Point.Position = PathLanePosition + LeftDir * LaneOffset;
			Point.Tangent = FMassSnorm8Vector2D(PathLaneTangent * TangentSign);
			Point.bOffPathLane = false;
			Point.bIsPathLaneExtrema = !Request.bIsEndOfPathPositionSet && CachedPathLane.IsDistanceAtLaneExtrema(EndDistanceAlongPath);
		}
		else
		{
			bPartialResult = true;
		}
	}

	checkf(NumPoints >= 1, TEXT("Path should have at least 1 point at this stage but has none."));

	// Add end of path point if set.
	if (bHasEndOfPathPoint)
	{
		if (NumPoints < MaxPoints)
		{
			const FVector EndPosition = Request.EndOfPathPosition;

			// Use provided direction if set, otherwise use direction from last point on lane to end of path point
			const FVector EndDirection = (Request.bIsEndOfPathDirectionSet) ?
				Request.EndOfPathDirection.Get() :
				(EndPosition - Points[NumPoints-1].Position).GetSafeNormal();
			
			FMassNavigationPathPoint& Point = Points[NumPoints++];
			Point.DistanceAlongLane = FMassInt16Real10(TargetDistanceAlongLane);
			Point.Position = EndPosition;
			Point.Tangent = FMassSnorm8Vector2D(EndDirection);
			Point.bOffPathLane = bEndOffPathLane;
			Point.bIsPathLaneExtrema = false;
		}
		else
		{
			bPartialResult = true;
		}
	}

	checkf(NumPoints >= 2, TEXT("Path should have at least 2 points at this stage, has %d."), NumPoints);

	// Calculate movement distance at each point.
	float PathDistance = 0.0f;
	Points[0].Distance.Set(PathDistance);
	for (uint8 PointIndex = 1; PointIndex < NumPoints; PointIndex++)
	{
		FMassNavigationPathPoint& PrevPoint = Points[PointIndex - 1];
		FMassNavigationPathPoint& Point = Points[PointIndex];
		const FVector PrevPosition = PrevPoint.Position;
		const FVector Position = Point.Position;
		const float DeltaDistance = FloatCastChecked<float>(FVector::Dist(PrevPosition, Position), UE::LWC::DefaultFloatPrecision);
		PathDistance += DeltaDistance;
		Point.Distance.Set(PathDistance);
	}
	
	// If the last point on path reaches end of the lane, set the next handle to the next lane. It will be update when path finishes.
	// TODO to implement, maybe use bReverse?
	// if (!bPartialResult && Request.NextLaneHandle.IsValid())
	// {
	// 	const FMassNavigationPathPoint& LastPoint = Points[NumPoints - 1];
	//
	// 	if (Request.NextExitLinkType == EZoneLaneLinkType::Adjacent || LastPoint.bIsLaneExtrema)
	// 	{
	// 		NextPathLaneHandle = Request.NextLaneHandle;
	// 		NextExitLinkType = Request.NextExitLinkType;
	// 	}
	// }

	return true;
}

// ActivateActionMove called from task will request a path and in result will cache the lane and will also produce a ShortPath from the cached part of the path lane
// todo FMassNavigationPathPoint minimal data for short path points
