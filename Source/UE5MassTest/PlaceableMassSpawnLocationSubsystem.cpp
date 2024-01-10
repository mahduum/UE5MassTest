// Fill out your copyright notice in the Description page of Project Settings.
#include "PlaceableMassSpawnLocationSubsystem.h"

#include "MassSpawnerSubsystem.h"
#include "PlaceablesObserver.h"
//DEFINE_LOG_CATEGORY(LogPlaceableMassSpawn)
const uint16 FPlaceableSpawnLocationDataHandle::InvalidGeneration = 0;

FPlaceableSpawnLocationDataHandle UPlaceableMassSpawnLocationSubsystem::RegisterSpawnLocationData(FVector& InLocationData)//todo this must be an Actor ref to make sense like AZoneGraphData with subsystem interface APlaceableSpawner
{
	// if (!IsValid(InLocationData))
	// {
	// 	return FPlaceableSpawnLocationDataHandle();
	// }
	//
	// if (InLocationData.IsRegistered())
	// {
	// 	UE_LOG(LogPlaceableMassSpawn, Error, TEXT("Trying to register already registered location data."));
	// 	return FPlaceableSpawnLocationDataHandle();
	// }

	FScopeLock Lock(&DataRegistrationSection);

	if (RegisteredSpawnLocationData.FindByPredicate([&InLocationData](const FRegisteredPlaceableSpawnLocationData& Item) { return Item.bInUse && Item.SpawnLocation == InLocationData; }) != nullptr)
	{
		//UE_LOG(LogPlaceableMassSpawn, Error, TEXT("Location data already exists in RegisteredSpawnLocationData."));
		return FPlaceableSpawnLocationDataHandle();
	}

	const int32 Index = (ZoneGraphDataFreeList.Num() > 0) ? ZoneGraphDataFreeList.Pop(/*bAllowShrinking=*/ false) : RegisteredSpawnLocationData.AddDefaulted();//if it is full add new
	FRegisteredPlaceableSpawnLocationData& RegisteredData = RegisteredSpawnLocationData[Index];
	RegisteredData.Reset(RegisteredData.Generation); // Do not change generation.
	RegisteredData.SpawnLocation = InLocationData;
	RegisteredData.bInUse = true;
	check(Index < int32(MAX_uint16));
	const FPlaceableSpawnLocationDataHandle ResultHandle = FPlaceableSpawnLocationDataHandle(uint16(Index), uint16(RegisteredData.Generation));
	
	// InLocationData.OnRegistered(ResultHandle);//todo =
	// void APlaceableSpawnLocationData::OnRegistered(const FZoneGraphDataHandle DataHandle)
	// {
	// 	ZoneStorage.DataHandle = DataHandle;
	// 	bRegistered = true;
	// }
	//
	// UE::ZoneGraphDelegates::OnPostZoneGraphDataAdded.Broadcast(RegisteredData.ZoneGraphData);
	
	return ResultHandle;
}

void UPlaceableMassSpawnLocationSubsystem::UnregisterSpawnLocationData(FVector& LocationData)//todo same here must be actor APlaceableSpawner that has all the spawning data inside!!! remember about mutable FCriticalSection ZoneStorageLock; from ZoneGraphData
{
	FScopeLock Lock(&DataRegistrationSection);

	const int32 Index = RegisteredSpawnLocationData.IndexOfByPredicate([&LocationData](const FRegisteredPlaceableSpawnLocationData& Item) { return Item.bInUse && Item.SpawnLocation == LocationData; });
	if (Index == INDEX_NONE)
	{
		//UE_LOG(LogPlaceableMassSpawn, Error, TEXT("Trying to remove ZoneGraphData \'%s\' that does not exists in FRegisteredPlaceableSpawnLocationData."));
		return;
	}

	RemoveRegisteredDataItem(Index);

	//LocationData.OnUnregistered();//todo=
	// void APlaceableSpawnLocationData::OnUnregistered()
	// {
	// 	bRegistered = false;
	// }
}

void UPlaceableMassSpawnLocationSubsystem::RemoveRegisteredDataItem(const int32 Index)
{
	auto RegisteredData = RegisteredSpawnLocationData[Index];
	check(int32(RegisteredData.Generation + 1) < int32(MAX_uint16));

	//UE::ZoneGraphDelegates::OnPreZoneGraphDataRemoved.Broadcast(RegisteredData.ZoneGraphData);

	RegisteredData.Reset(RegisteredData.Generation + 1);	// Bump generation, so that uses of stale handles can be detected. <--------------------

	// Mark index to be reused.
	ZoneGraphDataFreeList.Add(Index);//make place, mark that list have free spaces
}

void UPlaceableMassSpawnLocationSubsystem::AddSpawnLocation(FVector InLocation)
{
	CurrentSpawnLocation = InLocation;
	OnSpawnLocationReceived.Broadcast(1);
}

void UPlaceableMassSpawnLocationSubsystem::MoveTargetLocation(FVector Location)
{
	//todo:
	// send signal to processor with data to move to  or:
	// just set the variable and system will use it when it runs
	// HOW TO FIND THE ENTITY WE WANT TO MOVE? Select location, and destination location if first more less match (like on the grid) then deploy movement to entity
}
