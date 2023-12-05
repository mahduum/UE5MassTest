// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassNavigationSubsystem.h"
#include "HierarchicalHashGrid2D.h"
#include "UObject/Object.h"
#include "PlaceableMassSpawnLocationSubsystem.generated.h"

//for retrieval by index and by generation, is created when the actual data is registered like in ::RegisterZoneGraphData
//and retrieved by first checking index and then generation with:

// const AZoneGraphData* UZoneGraphSubsystem::GetZoneGraphData(const FZoneGraphDataHandle DataHandle) const
// {
// 	if (int32(DataHandle.Index) < RegisteredZoneGraphData.Num() && int32(DataHandle.Generation) == RegisteredZoneGraphData[DataHandle.Index].Generation)
// 	{
// 		return RegisteredZoneGraphData[DataHandle.Index].ZoneGraphData;
// 	}
// 	return nullptr;
// }


USTRUCT()
struct FPlaceableSpawnLocationDataHandle
{
	GENERATED_BODY()

	static const uint16 InvalidGeneration;	// 0

	FPlaceableSpawnLocationDataHandle() = default;
	FPlaceableSpawnLocationDataHandle(const uint16 InIndex, const uint16 InGeneration) : Index(InIndex), Generation(InGeneration) {}

	UPROPERTY(Transient)
	uint16 Index = 0;

	UPROPERTY(Transient)
	uint16 Generation = 0;//player can provide more locations, or something else might change them that is why the processor accessing the value need to know if it is still valid or the entity accessing it is no longer the same entity

	bool operator==(const FPlaceableSpawnLocationDataHandle& Other) const
	{
		return Index == Other.Index && Generation == Other.Generation;
	}

	bool operator!=(const FPlaceableSpawnLocationDataHandle& Other) const
	{
		return !operator==(Other);
	}

	friend uint32 GetTypeHash(const FPlaceableSpawnLocationDataHandle& Handle)
	{
		return uint32(Handle.Index) | (uint32(Handle.Generation) << 16);
	}

	void Reset()
	{
		Index = 0;
		Generation = InvalidGeneration;
	}

	bool IsValid() const { return Generation != InvalidGeneration; }	// Any index is valid, but Generation = 0 means invalid.
};

// Actual data.
USTRUCT()
struct FRegisteredPlaceableSpawnLocationData
{
	GENERATED_BODY()

	void Reset(int32 InGeneration = 1)
	{
		SpawnLocation = FVector::Zero();
		bInUse = false;
		Generation = InGeneration;
	}

	UPROPERTY()
	FVector SpawnLocation = FVector::Zero();//this may be and actor that provides the locations az TObjectPtr<TSubclassOf<AActor>>, this may be the player that has retrievable location for obstacle?
	//substitute spawn location with dedicated actor that is responsible for creating locations:
	//TObjectPtr<APlaceableSpawner> SpawnLocationData = nullptr;//todo the actor may hold all the history of locations?
	
	int32 Generation = 1;	// Starting at generation 1 so that 0 can be invalid.
	bool bInUse = false;	// Extra bit indicating that the data is in meant to be in use. This tried to capture the case where SpawnLocation might get nullified without notifying.
};

typedef THierarchicalHashGrid2D<2, 4, FMassNavigationObstacleItem> FPlaceablesHashGrid2D;//
/**
 * 
 */
UCLASS()
class UE5MASSTEST_API UPlaceableMassSpawnLocationSubsystem : public UWorldSubsystem//todo what is the difference??
{
	GENERATED_BODY()

	//add registered data like this:
	TConstArrayView<FRegisteredPlaceableSpawnLocationData> GetRegisteredSpawnLocationData() const { return RegisteredSpawnLocationData; }
	FPlaceableSpawnLocationDataHandle RegisterSpawnLocationData(FVector& InLocationData);
	void UnregisterSpawnLocationData(FVector& LocationData);
	void RemoveRegisteredDataItem(int32 Index);

	//that is accessed by handle like in ZoneGraphSubsystem
	const FPlaceablesHashGrid2D& GetObstacleGrid() const { return PlaceablesGrid; }
	FPlaceablesHashGrid2D& GetObstacleGridMutable() { return PlaceablesGrid; }

protected:

	FPlaceablesHashGrid2D PlaceablesGrid;//todo get the grid from subsystem

	FCriticalSection DataRegistrationSection;
	UPROPERTY()
	TArray<FRegisteredPlaceableSpawnLocationData> RegisteredSpawnLocationData;
	TArray<int32> ZoneGraphDataFreeList;

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMassEntitySpawnMessageSent, int, SpawnCount);
	UPROPERTY(BlueprintAssignable, Category="Mass Spawn Location")
	FMassEntitySpawnMessageSent OnSpawnLocationReceived;
	
	//temporary quick solution to provide location
	UPROPERTY(BlueprintReadWrite, BlueprintSetter="AddSpawnLocation", Category="Mass Spawn Location")
	FVector CurrentSpawnLocation = FVector::ZeroVector;//instead make a friend and allow only him to set this

	UFUNCTION(BlueprintSetter, Category="Mass Spawn Location")
	FORCEINLINE void AddSpawnLocation(FVector InLocation);

	//todo get entity on selected location from grid, make selection processor, modify selected entities
};
