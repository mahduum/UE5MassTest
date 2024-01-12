// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassNavigationExtTypes.h"
#include "MassNavigationShortPathRequest.h"
#include "MassNavigationTypes.h"
#include "MassNavigationExtFragments.generated.h"

/**
 * 
 */
USTRUCT()
struct UE5MASSTEST_API FMassNavigationShortPathRequestFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Short path request Handle to current lane. */
	UPROPERTY(Transient)
	FMassNavigationShortPathRequest PathRequest;//todo processed like FZoneGraphShortPathRequest
};

/** Describes current path or a partial path, in zone graph lane location belonged to one of the parallel paths within zone graph. */ 
USTRUCT()
struct UE5MASSTEST_API FMassNavigationPathLaneLocationFragment : public FMassFragment//FMassZoneGraphLaneLocationFragment
{
	GENERATED_BODY()

	/** Handle to current lane. */
	FMassNavigationPathLaneHandle PathHandle;//ZoneGraph types//FMassNavigationPathLaneHandle
	
	/** Distance along current lane. */
	float DistanceAlongPathLane = 0.0f;
	
	/** Cached lane length, used for clamping and testing if at end of lane. */
	float PathLength = 0.0f;
};


/** Describes part of a ZoneGraph lane/MassNavigation path. */
USTRUCT()
struct UE5MASSTEST_API FMassNavigationCachedPathLaneFragment : public FMassFragment
{
	GENERATED_BODY()

	static constexpr uint8 MaxPoints = 5;

	void Reset()
	{
		PathLaneHandle.Reset();
		PathLaneLength = 0.0f;
		//LaneWidth = FMassInt16Real(0.0f);
		NumPoints = 0;
	}

	/** Caches portion of a path from storage based on the progression distance along the path, finds a point with progression distance
	 * that is greater than the current progression and caches partial path onward from the previous point to that one. */
	void CachePathLaneData(const FMassNavigationDataStorage& MassNavigationDataStorage, const FMassNavigationPathLaneHandle CurrentPathLaneHandle,
					   const float CurrentDistanceAlongLane, const float TargetDistanceAlongLane, const float InflateDistance);

	int32 FindSegmentIndexAtDistance(const float DistanceAlongPath) const
	{
		int32 SegmentIndex = 0;
		while (SegmentIndex < ((int32)NumPoints - 2))
		{
			if (DistanceAlongPath < PathLanePointProgressions[SegmentIndex + 1].Get())
			{
				break;
			}
			SegmentIndex++;
		}

		return SegmentIndex;
	}

	float GetInterpolationTimeOnSegment(const int32 SegmentIndex, const float DistanceAlongPath) const
	{
		check(SegmentIndex >= 0 && SegmentIndex <= (int32)NumPoints - 2);
		const float StartDistance = PathLanePointProgressions[SegmentIndex].Get();
		const float EndDistance = PathLanePointProgressions[SegmentIndex + 1].Get();
		const float SegLength = EndDistance - StartDistance;
		const float InvSegLength = SegLength > KINDA_SMALL_NUMBER ? 1.0f / SegLength : 0.0f;
		return FMath::Clamp((DistanceAlongPath - StartDistance) * InvSegLength, 0.0f, 1.0f);
	}
	
	void InterpolatePointAndTangentOnSegment(const int32 SegmentIndex, const float DistanceAlongPath, FVector& OutPoint, FVector& OutTangent) const
	{
		const float T = GetInterpolationTimeOnSegment(SegmentIndex, DistanceAlongPath);
		OutPoint = FMath::Lerp(PathLanePoints[SegmentIndex], PathLanePoints[SegmentIndex + 1], T);
		OutTangent = FVector(FMath::Lerp(PathLaneTangentVectors[SegmentIndex].Get(), PathLaneTangentVectors[SegmentIndex + 1].Get(), T), 0.0f);
	}

	FVector InterpolatePointOnSegment(const int32 SegmentIndex, const float DistanceAlongPath) const
	{
		const float T = GetInterpolationTimeOnSegment(SegmentIndex, DistanceAlongPath);
		return FMath::Lerp(PathLanePoints[SegmentIndex], PathLanePoints[SegmentIndex + 1], T);
	}

	void GetPointAndTangentAtDistance(const float DistanceAlongPath, FVector& OutPoint, FVector& OutTangent) const
	{
		if (NumPoints == 0)
		{
			OutPoint = FVector::ZeroVector;
			OutTangent = FVector::ForwardVector;
			return;
		}
		if (NumPoints == 1)
		{
			OutPoint = PathLanePoints[0];
			OutTangent = FVector(PathLaneTangentVectors[0].Get(), 0.0f);
			return;
		}

		const int32 SegmentIndex = FindSegmentIndexAtDistance(DistanceAlongPath);
		InterpolatePointAndTangentOnSegment(SegmentIndex, DistanceAlongPath, OutPoint, OutTangent);
	}

	FVector GetPointAtDistance(const float DistanceAlongPath) const
	{
		if (NumPoints == 0)
		{
			return FVector::ZeroVector;
		}
		if (NumPoints == 1)
		{
			return PathLanePoints[0];
		}

		const int32 SegmentIndex = FindSegmentIndexAtDistance(DistanceAlongPath);
		return InterpolatePointOnSegment(SegmentIndex, DistanceAlongPath);
	}

	bool IsDistanceAtLaneExtrema(const float Distance) const
	{
		static constexpr float Epsilon = 0.1f;
		return Distance <= Epsilon || (Distance - PathLaneLength) >= -Epsilon;
	}

	FMassNavigationPathLaneHandle PathLaneHandle;
	
	/** Lane points */
	TStaticArray<FVector, MaxPoints> PathLanePoints;

	/** Cached length of the lane / whole path. */
	float PathLaneLength = 0.0f;

	/** Lane tangents */
	TStaticArray<FMassSnorm8Vector2D, MaxPoints> PathLaneTangentVectors;//assigned from storage

	/** lane Advance distances */
	TStaticArray<FMassInt16Real10, MaxPoints> PathLanePointProgressions;//minimize memory footprint by reducing the accuracy of float distance

	// /** Cached width of the lane. */
	FMassInt16Real PathLaneWidth = FMassInt16Real(0.0f);//todo may need this for bi directional fixed paths
	
	/** Additional space left of the lane */
	FMassInt16Real PathLaneLeftSpace = FMassInt16Real(0.0f);//todo may need this for bi directional fixed paths, used for avoidance
	
	/** Additional space right of the lane */
	FMassInt16Real PathLaneRightSpace = FMassInt16Real(0.0f);//todo may need this for bi directional fixed paths, used for avoidance

	/** ID incremented each time the cache is updated. */
	uint16 CacheID = 0;
	
	/** Number of points on path. */
	uint8 NumPoints = 0;
};

USTRUCT()
struct UE5MASSTEST_API FMassNavigationPathPoint
{
	GENERATED_BODY()

	/** Position of the path. */
	FVector Position = FVector::ZeroVector;

	/** Tangent direction of the path. */
	FMassSnorm8Vector2D Tangent;

	/** Position of the point along the original path. (Could potentially be uint16 at 10cm accuracy) */
	FMassInt16Real10 DistanceAlongLane = FMassInt16Real10(0.0f);

	/** Distance along the offset path from first point. (Could potentially be uint16 at 10cm accuracy) */
	FMassInt16Real Distance = FMassInt16Real(0.0f);

	/** True if this point is assumed to be off lane. */
	uint8 bOffPathLane : 1;

	/** True if this point is path start or end point. */
	uint8 bIsPathLaneExtrema : 1;
};

USTRUCT()
struct UE5MASSTEST_API FMassNavigationShortPathFragment : public FMassFragment
{
	GENERATED_BODY()

	FMassNavigationShortPathFragment() = default;
	
	static constexpr uint8 MaxPoints = 3;

	void Reset()
	{
#if WITH_MASSGAMEPLAY_DEBUG
		DebugPathLaneHandle.Reset();
#endif
		// NextPathLaneHandle.Reset();todo?
		// NextExitLinkType = EZoneLaneLinkType::None;
		ProgressDistance = 0.0f;
		NumPoints = 0;
		bMoveReverse = false;
		EndOfPathIntent = EMassMovementAction::Stand;
		bPartialResult = false;
		bDone = false;
	}

	/** Requests path along the current lane */
	bool RequestPath(const FMassNavigationCachedPathLaneFragment& CachedPathLane, const FMassNavigationShortPathRequest& Request, const float InCurrentDistanceAlongLane, const float AgentRadius);

	/** Requests path to stand at current position. */ //todo
	bool RequestStand(const FMassNavigationCachedPathLaneFragment& CachedPath, const float CurrentDistanceAlongLane, const FVector& CurrentPosition);
	
	bool IsDone() const
	{
		// @todo MassMovement: should we remove NumPoints == 0? The logic used to be quite different when it was really needed.
		return NumPoints == 0 || bDone;
	}

#if WITH_MASSGAMEPLAY_DEBUG
	/** Current lane handle, for debug */
	FMassNavigationPathLaneHandle DebugPathLaneHandle;
#endif
	
	/** If valid, the this lane will be set as current lane after the path follow is completed. */
	FMassNavigationPathLaneHandle NextPathLaneHandle;//todo make a system of paths walked in frequently that become visible on grass
	
	/** Current progress distance along the lane. */
	float ProgressDistance = 0.0f;
	
	/** Path points */
	TStaticArray<FMassNavigationPathPoint, MaxPoints> Points;

	// /** If next lane is set, this is how to reach the lane from current lane. */
	// EZoneLaneLinkType NextExitLinkType = EZoneLaneLinkType::None;
	
	/** Number of points on path. */
	uint8 NumPoints = 0;
	
	/** Intent at the end of the path. */
	EMassMovementAction EndOfPathIntent = EMassMovementAction::Stand;

	/** True if we're moving reverse */
	uint8 bMoveReverse : 1;

	/** True if the path was partial. */
	uint8 bPartialResult : 1;

	/** True when path follow is completed. */
	uint8 bDone : 1;
};
