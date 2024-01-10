// Fill out your copyright notice in the Description page of Project Settings.
#include "MassNavigationDataStorageSubsystem.h"

#include "EngineUtils.h"
#include "NavigationPathsData.h"


DEFINE_LOG_CATEGORY(LogMassNavigationExt);
DEFINE_LOG_CATEGORY_STATIC(ALogTestCategory, Log, All);

UMassNavigationDataStorageSubsystem::UMassNavigationDataStorageSubsystem()
	: bInitialized(false)
{
}

FMassNavigationPathDataHandle UMassNavigationDataStorageSubsystem::RegisterMassNavigationData(
	ANavigationPathsData& InNavigationData)
{
	if (!IsValid(&InNavigationData))
	{
		return FMassNavigationPathDataHandle();
	}

	if (InNavigationData.IsRegistered())
	{
		UE_LOG(LogMassNavigationExt, Error, TEXT("Trying to register already registered ZoneGraphData \'%s\'"), *InNavigationData.GetName());
		return FMassNavigationPathDataHandle();
	}

	FScopeLock Lock(&DataRegistrationSection);

	if (RegisteredNavigationData.FindByPredicate([&InNavigationData](const FRegisteredMassNavigationData& Item) { return Item.bInUse && Item.NavigationPathsData == &InNavigationData; }) != nullptr)
	{
		UE_LOG(LogMassNavigationExt, Error, TEXT("ZoneGraphData \'%s\' already exists in RegisteredZoneGraphData."), *InNavigationData.GetName());
		return FMassNavigationPathDataHandle();
	}

	const int32 Index = (NavigationDataFreeList.Num() > 0) ? NavigationDataFreeList.Pop(/*bAllowShrinking=*/ false) : RegisteredNavigationData.AddDefaulted();
	FRegisteredMassNavigationData& RegisteredData = RegisteredNavigationData[Index];
	RegisteredData.Reset(RegisteredData.Generation); // Do not change generation, it's recycled from free list
	RegisteredData.NavigationPathsData = &InNavigationData;
	RegisteredData.bInUse = true;
	check(Index < int32(MAX_uint16));
	const FMassNavigationPathDataHandle ResultHandle = FMassNavigationPathDataHandle(uint16(Index), uint16(RegisteredData.Generation));

	InNavigationData.OnRegistered(ResultHandle);

	//UE::ZoneGraphDelegates::OnPostZoneGraphDataAdded.Broadcast(RegisteredData.ZoneGraphData);TODO

	return ResultHandle;
}

void UMassNavigationDataStorageSubsystem::UnregisterMassNavigationData(ANavigationPathsData& InNavigationData)
{
	FScopeLock Lock(&DataRegistrationSection);

	const int32 Index = RegisteredNavigationData.IndexOfByPredicate([&InNavigationData](const FRegisteredMassNavigationData& Item) { return Item.bInUse && Item.NavigationPathsData== &InNavigationData; });
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogMassNavigationExt, Error, TEXT("Trying to remove ZoneGraphData \'%s\' that does not exists in RegisteredZoneGraphData."), *InNavigationData.GetName());
		return;
	}

	RemoveRegisteredDataItem(Index);

	InNavigationData.OnUnregistered();
}

void UMassNavigationDataStorageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bInitialized = true;
}

void UMassNavigationDataStorageSubsystem::PostInitialize()
{
	Super::PostInitialize();
	//todo consider removing all actor instances of data holders and keep a single storage?
	RegisterNavigationDataInstances();
}

void UMassNavigationDataStorageSubsystem::Deinitialize()
{
	bInitialized = false;
	Super::Deinitialize();
}

void UMassNavigationDataStorageSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TStatId UMassNavigationDataStorageSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMassNavigationDataStorageSubsystem, STATGROUP_Tickables);
}

void UMassNavigationDataStorageSubsystem::RemoveRegisteredDataItem(const int32 Index)
{
	FRegisteredMassNavigationData& RegisteredData = RegisteredNavigationData[Index];
	check(int32(RegisteredData.Generation + 1) < int32(MAX_uint16));

	//UE::ZoneGraphDelegates::OnPreZoneGraphDataRemoved.Broadcast(RegisteredData.ZoneGraphData);TODO

	RegisteredData.Reset(RegisteredData.Generation + 1);	// Bump generation, so that uses of stale handles can be detected.

	// Mark index to be reused.
	NavigationDataFreeList.Add(Index);
}

void UMassNavigationDataStorageSubsystem::UnregisterStaleZoneGraphDataInstances()
{
	// Unregister data that have gone stale.
	for (int32 Index = 0; Index < RegisteredNavigationData.Num(); Index++)
	{
		if (RegisteredNavigationData[Index].bInUse)
		{
			const ANavigationPathsData* ZoneGraphData = RegisteredNavigationData[Index].NavigationPathsData;
			if (!IsValid(ZoneGraphData))
			{
				UE_LOG(LogMassNavigationExt, Error, TEXT("Removing stale ZoneGraphData \'%s\' that does not exists in RegisteredZoneGraphData."), *GetNameSafe(ZoneGraphData));
				RemoveRegisteredDataItem(Index);
				Index--;
			}
			else if (!ZoneGraphData->IsRegistered())
			{
				UE_LOG(LogMassNavigationExt, Error, TEXT("Removing unregistered ZoneGraphData \'%s\' that does not exists in RegisteredZoneGraphData."), *GetNameSafe(ZoneGraphData));
				RemoveRegisteredDataItem(Index);
				Index--;
			}
		}
	}
}

void UMassNavigationDataStorageSubsystem::RegisterNavigationDataInstances()
{
	const UWorld* World = GetWorld();

	// Make sure all data is registered.
	for (TActorIterator<ANavigationPathsData> It(World); It; ++It)
	{
		ANavigationPathsData* NavigationPathsData = (*It);
		if (NavigationPathsData != nullptr && IsValidChecked(NavigationPathsData) == true && NavigationPathsData->IsRegistered() == false)
		{
			RegisterMassNavigationData(*NavigationPathsData);
		}
	}
}
