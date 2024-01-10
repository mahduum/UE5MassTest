// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavigationPathsData.h"
#include "MassNavigationDataStorageSubsystem.generated.h"

UE5MASSTEST_API DECLARE_LOG_CATEGORY_EXTERN(LogMassNavigationExt, Log, All);

/**
 * 
 */

// Struct representing registered ZoneGraph data (one per level) in the subsystem.
USTRUCT()
struct FRegisteredMassNavigationData//FRegisteredZoneGraphData
{
	GENERATED_BODY()

	void Reset(int32 InGeneration = 1)
	{
		NavigationPathsData = nullptr;
		bInUse = false;
		Generation = InGeneration;
	}

	UPROPERTY()
	TObjectPtr<ANavigationPathsData> NavigationPathsData = nullptr;//actor that holds info about all the lanes/paths in the level, has direct access to Storage, It is assigned a DataHandle on actors self registration
	//subsystem registers each such actor with generation

	int32 Generation = 1;	// Starting at generation 1 so that 0 can be invalid.
	bool bInUse = false;	// Extra bit indicating that the data is in meant to be in use. This tried to capture the case where NavigationPathsData might get nullified without notifying.
};

UCLASS()
class UE5MASSTEST_API UMassNavigationDataStorageSubsystem : public UTickableWorldSubsystem//UZoneGraphSubsystem 
{
	GENERATED_BODY()

public:
	UMassNavigationDataStorageSubsystem();

	//todo make new handle for the actor of zone graph (now: mass nav data) can be used to get actor or zone storage from that actor?
	FMassNavigationPathDataHandle RegisterMassNavigationData(ANavigationPathsData& InNavigationData);//todo zonegraph data TODO change handle
	void UnregisterMassNavigationData(ANavigationPathsData& InNavigationData);
	TConstArrayView</*FRegisteredZoneGraphData*/FRegisteredMassNavigationData> GetRegisteredMassNavigationData() const { return RegisteredNavigationData; }//todo

	const FMassNavigationDataStorage* GetMassNavigationDataStorage(const FMassNavigationPathDataHandle DataHandle) const
	{
		if (int32(DataHandle.Generation) == RegisteredNavigationData[DataHandle.Index].Generation && int32(DataHandle.Index) < RegisteredNavigationData.Num())
		{
			//todo a given zone actor holds specific data just for this zone and registers this data
			if (const ANavigationPathsData* Data = RegisteredNavigationData[DataHandle.Index].NavigationPathsData)
			{
				return &Data->GetStorage();
			}
		}
		return nullptr;
	}

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void PostInitialize() override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	
	void RemoveRegisteredDataItem(const int32 Index);
	void UnregisterStaleZoneGraphDataInstances();
	void RegisterNavigationDataInstances();
	
	FCriticalSection DataRegistrationSection;

	UPROPERTY()
	TArray<FRegisteredMassNavigationData> RegisteredNavigationData;//RegisteredZoneGraphData;//todo implement registration
	TArray<int32> NavigationDataFreeList;

	bool bInitialized;
};
