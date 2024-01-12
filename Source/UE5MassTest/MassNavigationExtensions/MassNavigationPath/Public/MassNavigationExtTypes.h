// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCommonTypes.h"
#include "MassNavigationTypes.h"
#include "UObject/Object.h"
#include "MassNavigationExtTypes.generated.h"

/**
 * 
 */
struct FPathSplinePoint
{
	FPathSplinePoint() = default;
	FPathSplinePoint(const FVector& InPosition, const FVector& InUp) : Position(InPosition), Up(InUp) {}

	FVector Position = FVector::ZeroVector;
	FVector Up = FVector::UpVector;
	FVector Right = FVector::RightVector;
	FVector Forward = FVector::ForwardVector;//or tangent?
};

USTRUCT()
struct UE5MASSTEST_API FPathLaneData
{
	GENERATED_BODY()

	int32 GetLinkCount() const { return LinksEnd - LinksBegin; }
	int32 GetNumPoints() const { return PointsEnd - PointsBegin; }
	int32 GetLastPoint() const { return PointsEnd - 1; }

	// Width of the lane
	UPROPERTY()
	float Width = 0.0f;

	// // Lane tags
	// UPROPERTY()
	// FZoneGraphTagMask Tags = FZoneGraphTagMask(1);	// Default Tag

	// First point of the lane polyline in FZoneGraphStorage::LanePoints.
	UPROPERTY()
	int32 PointsBegin = 0;

	// One past the last point of the lane polyline.
	UPROPERTY()
	int32 PointsEnd = 0;

	// First link in FZoneGraphStorage::LaneLinks.
	UPROPERTY()
	int32 LinksBegin = 0;

	// One past the last lane link.
	UPROPERTY()
	int32 LinksEnd = 0;

	// Index of the zone/general path to destination/path this lane belongs to.
	UPROPERTY()
	int32 PathIndex = 0;

	// Source data entry ID, this generally corresponds to input data point index.
	UPROPERTY()
	uint16 StartEntryId = 0;

	// Source data entry ID.
	UPROPERTY()
	uint16 EndEntryId = 0;
};

USTRUCT()
struct UE5MASSTEST_API FPathData
{
	GENERATED_BODY()

	int32 GetLaneCount() const { return LanesEnd - LanesBegin; }

	// First point of the zone/general path to destination boundary polyline in FZoneGraphStorage::BoundaryPoints.
	UPROPERTY()
	int32 BoundaryPointsBegin = 0;

	// One past the last point of the zone/general path to destination boundary polyline.
	UPROPERTY()
	int32 BoundaryPointsEnd = 0;

	// First lane of the zone/general path to destination in FZoneGraphStorage::PathLanes.
	UPROPERTY()
	int32 LanesBegin = 0;

	// One past the last lane.
	UPROPERTY()
	int32 LanesEnd = 0;

	// Bounding box of the zone/general path to destination
	UPROPERTY()
	FBox Bounds = FBox(ForceInit);

	// // Zone tags
	// UPROPERTY()
	// FZoneGraphTagMask Tags = FZoneGraphTagMask(1);	// Default Tag
};

USTRUCT()
struct UE5MASSTEST_API FMassNavigationPathDataHandle//FZoneGraphDataHandle
{
	GENERATED_BODY()


	FMassNavigationPathDataHandle() = default;
	FMassNavigationPathDataHandle(const uint16 InIndex, const uint16 InGeneration) : Index(InIndex), Generation(InGeneration) {}

	UPROPERTY(Transient)
	uint16 Index = 0;

	UPROPERTY(Transient)
	uint16 Generation = 0;
	
	//const uint16 InvalidGenerationIndex = 0;	// 0

	bool operator==(const FMassNavigationPathDataHandle& Other) const
	{
		return Index == Other.Index && Generation == Other.Generation;
	}

	bool operator!=(const FMassNavigationPathDataHandle& Other) const
	{
		return !operator==(Other);
	}

	friend uint32 GetTypeHash(const FMassNavigationPathDataHandle& Handle)
	{
		return uint32(Handle.Index) | (uint32(Handle.Generation) << 16);
	}

	void Reset()
	{
		Index = 0;
		Generation = 0;
	}

	bool IsValid() const { return Generation != 0; }	// Any index is valid, but Generation = 0 means invalid.
};

USTRUCT()
struct UE5MASSTEST_API FMassNavigationPathLaneHandle//FZoneGraphLaneHandle
{
	GENERATED_BODY()

	FMassNavigationPathLaneHandle() = default;
	FMassNavigationPathLaneHandle(const int32 InLaneIndex, const FMassNavigationPathDataHandle InDataHandle) : Index(InLaneIndex), DataHandle(InDataHandle) {}

	UPROPERTY(Transient)
	int32 Index = INDEX_NONE;

	/*Handle to retrieve the actual data from subsystem as storage*/
	UPROPERTY(Transient)
	FMassNavigationPathDataHandle DataHandle;//FZoneGraphDataHandle handle to access the whole storage

	bool operator==(const FMassNavigationPathLaneHandle& Other) const
	{
		return Index == Other.Index && DataHandle == Other.DataHandle;
	}

	bool operator!=(const FMassNavigationPathLaneHandle& Other) const
	{
		return !operator==(Other);
	}

	friend uint32 GetTypeHash(const FMassNavigationPathLaneHandle& Handle)
	{
		return HashCombine(Handle.Index, GetTypeHash(Handle.DataHandle));
	}

	void Reset()
	{
		Index = INDEX_NONE;
		DataHandle.Reset();
	}

	FString ToString() const { return FString::Printf(TEXT("[%d/%d]"), DataHandle.Index, Index); }
	
	bool IsValid() const { return (Index != INDEX_NONE) && DataHandle.IsValid(); }
};

/* Used in follow path processors when changing or assigning new path lanes to follow is retrieved from BVTree by zones/general paths
 * and then lanes in it, it is then assigned to `FMassNavigationPathLaneLocationFragment` but only as a handle to its data
 * NOTE: this may be usable if I go with the network of paths when no more paths are created.
 */
USTRUCT()
struct UE5MASSTEST_API FMassNavigationPathLaneLocation//FZoneGraphLaneLocation
{
	GENERATED_BODY()
		
	UPROPERTY(Transient)
	FVector Position = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(Transient)
	FVector Tangent = FVector::ForwardVector;

	UPROPERTY(Transient)
	FVector Up = FVector::UpVector;

	UPROPERTY(Transient)
	FMassNavigationPathLaneHandle PathLaneHandle;

	UPROPERTY(Transient)
	int32 LaneSegment = 0;

	UPROPERTY(Transient)
	float DistanceAlongLane = 0.0f;

	void Reset()
	{
		Position = FVector::ZeroVector;
		Direction = FVector::ForwardVector;
		Tangent = FVector::ForwardVector;
		Up = FVector::UpVector;
		PathLaneHandle.Reset();
		LaneSegment = 0;
		DistanceAlongLane = 0.0f;
	}

	bool IsValid() const { return PathLaneHandle.IsValid(); }
};

/** Minimal amount of data to search and compare lane location. */
USTRUCT()
struct FMassNavigationCompactPathLaneLocation
{
	GENERATED_BODY()

	FMassNavigationCompactPathLaneLocation() = default;
	FMassNavigationCompactPathLaneLocation(const FMassNavigationPathLaneLocation& Location) : PathLaneHandle(Location.PathLaneHandle), DistanceAlongLane(Location.DistanceAlongLane) {}
	FMassNavigationCompactPathLaneLocation(const FMassNavigationPathLaneHandle Handle, const float Distance) : PathLaneHandle(Handle), DistanceAlongLane(Distance) {}

	UPROPERTY(Transient)
	FMassNavigationPathLaneHandle PathLaneHandle;

	UPROPERTY(Transient)
	float DistanceAlongLane = 0.0f;
};

/** Section of a lane */
USTRUCT()
struct FMassNavigationLinkedSection
{
	GENERATED_BODY()

	bool operator==(const FMassNavigationLinkedSection& Other) const
	{
		return PathLaneHandle == Other.PathLaneHandle && StartDistanceAlongLane == Other.StartDistanceAlongLane && EndDistanceAlongLane == Other.EndDistanceAlongLane;
	}
	
	UPROPERTY(Transient)
	FMassNavigationPathLaneHandle PathLaneHandle;

	UPROPERTY(Transient)
	float StartDistanceAlongLane = 0.0f;

	UPROPERTY(Transient)
	float EndDistanceAlongLane = 0.0f;
};

/** Linked lane, used for query results. See also: FZoneLaneLinkData */
USTRUCT()
struct UE5MASSTEST_API FMassNavigationLinkedLane
{
	GENERATED_BODY()

	FMassNavigationLinkedLane() = default;
	FMassNavigationLinkedLane(const FMassNavigationPathLaneHandle InDestLane) : DestLane(InDestLane) {} //, Type(InType), Flags((uint8)InFlags) {}

	void Reset()
	{
		DestLane.Reset();
		//Type = EZoneLaneLinkType::None;
		//Flags = 0;
	}

	bool IsValid() const { return DestLane.IsValid(); }

	// bool HasFlags(const EZoneLaneLinkFlags InFlags) const { return (Flags & (uint8)InFlags) != 0; }
	// EZoneLaneLinkFlags GetFlags() const { return (EZoneLaneLinkFlags)Flags; }
	// void SetFlags(const EZoneLaneLinkFlags InFlags) { Flags = (uint8)InFlags; }
	
	/** Destination lane handle */
	UPROPERTY()
	FMassNavigationPathLaneHandle DestLane = {};

	/** Type of the connection. */
	// UPROPERTY()
	// EZoneLaneLinkType Type = EZoneLaneLinkType::None;

	/** Specifics about the connection type, see EZoneLaneLinkFlags. */
	// UPROPERTY()
	// uint8 Flags = 0;
};

/* All the data arrays are lined up. */
USTRUCT()
struct UE5MASSTEST_API FMassNavigationDataStorage//FZoneGraphStorage
{
	GENERATED_BODY()

	void Reset()
	{
		Paths.Reset();
		PathLanes.Reset();
		//BoundaryPoints.Reset();
		PathLanePoints.Reset();
		PathLaneUpVectors.Reset();
		PathLaneTangentVectors.Reset();
		PathLanePointsProgressions.Reset();
		//LaneLinks.Reset();
		//Bounds.Init();
	};
	// const FZoneData& GetZoneDataFromLaneIndex(int32 LaneIndex) const { return Paths[PathLanes[LaneIndex].ZoneIndex]; }
	//
	// // All the zones.
	UPROPERTY()
	TArray<FPathData> Paths;//originally in the zone/general path to destination graph api, every zone/general path to destination shape component constitutes an FZoneData todo maybe needed for caching - must combine with the next one?
	
	// All the lanes, referred by zones, each lane is autonomous path within the zone/general path to destination data / zone/general path to destination shape component
	UPROPERTY()
	TArray<FPathLaneData> PathLanes;//lane specific data within the zone/general path to destination (zone/general path to destination shape component and parallel lanes within the same profile, each lane profile may have different number of lanes)todo needed for caching
	
	// // All the zone/general path to destination boundary points, referred by zones.
	// UPROPERTY()
	// TArray<FVector> BoundaryPoints;
	//
	// All the path points, referred by paths.
	UPROPERTY()
	TArray<FVector> PathLanePoints;//LanePoints
	//
	// // All the lane up vectors, referred by lanes.
	UPROPERTY()
	TArray<FVector> PathLaneUpVectors;//todo 
	
	// All the lane tangent vectors, referred by lanes.
	UPROPERTY()
	TArray<FVector> PathLaneTangentVectors;//todo
	//
	// // All the paths progression distances, referred by paths, to choose points for cached paths.
	UPROPERTY()
	TArray<float> PathLanePointsProgressions; //LanePointProgressions;
	//
	// // All the lane links, referred by lanes.
	// UPROPERTY()
	// TArray<FZoneLaneLinkData> LaneLinks;
	//
	// // Bounding box of all zones/paths held by actor, may be useful when finding trail composed from different paths' segments
	// UPROPERTY()
	// FBox Bounds = FBox(ForceInit);
	//
	// // BV-Tree of Paths, queried with bounds returns the indices of overlapping items, can be used when //todo for switching between different paths in the network of paths, finding nearest paths or overlapping paths
	// // Built from `ZoneBVTree.Build(MakeStridedView(ZoneStorage.Zones, &FZoneData::Bounds));` takes in zones and makes the view of its member `Bounds` as allows accessing it as though they were contiguous elements
	// // by taking first element and adding internal offset of the stride of the whole element type of `Zones` containing the member `Bounds`. In build it created nodes with assigned bounds from FBoxes of `Bounds`
	// // and then recursively subdivides the array of FBoxes. First node calculates combined bounds of all elements from 0 to num elements, then uses Algo::Sort to sort node along the longest box axis from the shortest to the longest
	// // then it recursively splits sorted in half and then repeats until tree is built
	// UPROPERTY()
	// FZoneGraphBVTree ZoneBVTree;
	//
	// // The handle that this storage represents, updated when data is registered to ZoneGraphSubsystem, used for query results.
	FMassNavigationPathDataHandle DataHandle;
};


USTRUCT()
struct UE5MASSTEST_API FMassNavigationTargetLocation
{
	GENERATED_BODY()

	void Reset()
	{
		PathLaneHandle.Reset();
		bMoveReverse = false;
		TargetDistance = 0.0f;
		EndOfPathPosition.Reset();
		AnticipationDistance.Set(50.0f);
		EndOfPathIntent = EMassMovementAction::Move;
	}

	/** Current lane handle. (Could be debug only) */
	FMassNavigationPathLaneHandle PathLaneHandle;
	
	/** Target distance along current lane, equal to distance along the lane plus planned next move distance
	 * in case of mass nav ext this distance for now will be the distance to final target along the path. */
	float TargetDistance = 0.0f;
	
	/** Optional end of path location. */
	TOptional<FVector> EndOfPathPosition;

	/** Optional end of path direction, used only if EndOfPathPosition is set. */
	TOptional<FVector> EndOfPathDirection;

	/** If start or end of path is off-lane, the distance along the lane is pushed forward/back along the lane to make smoother transition. */
	FMassInt16Real AnticipationDistance = FMassInt16Real(50.0f);

	/** True, if we're moving reverse along the lane. */
	bool bMoveReverse = false;
	
	/** Movement intent at the end of the path */
	EMassMovementAction EndOfPathIntent = EMassMovementAction::Move;
};
