// Fill out your copyright notice in the Description page of Project Settings.


#include "NavigationPathsData.h"
#include "MassNavigationDataStorageSubsystem.h"


// Sets default values
ANavigationPathsData::ANavigationPathsData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetCanBeDamaged(false);
}

void ANavigationPathsData::PostActorCreated()
{
	Super::PostActorCreated();
	const bool bSucceeded = RegisterWithSubsystem();
	UE_LOG(LogMassNavigationExt, Verbose, TEXT("\'%s\' (0x%llx) PostActorCreated - %s"), *GetName(), UPTRINT(this), bSucceeded ? TEXT("Succeeded") : TEXT("Failed"));
}

void ANavigationPathsData::PostLoad()
{
	Super::PostLoad();
	const bool bSucceeded = RegisterWithSubsystem();
	UE_LOG(LogMassNavigationExt, Verbose, TEXT("\'%s\' (0x%llx) PostLoad - %s"), *GetName(), UPTRINT(this), bSucceeded ? TEXT("Succeeded") : TEXT("Failed"));
}

void ANavigationPathsData::Destroyed()
{
	const bool bSucceeded = UnregisterWithSubsystem();
	Super::Destroyed();
	UE_LOG(LogMassNavigationExt, Verbose, TEXT("\'%s\' (0x%llx) Destroyed - %s"), *GetName(), UPTRINT(this), bSucceeded ? TEXT("Succeeded") : TEXT("Failed"));
}

void ANavigationPathsData::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const bool bSucceeded = UnregisterWithSubsystem();
	Super::EndPlay(EndPlayReason);
	UE_LOG(LogMassNavigationExt, Verbose, TEXT("\'%s\' (0x%llx) EndPlay - %s"), *GetName(), UPTRINT(this), bSucceeded ? TEXT("Succeeded") : TEXT("Failed"));
}

void ANavigationPathsData::PreRegisterAllComponents()
{
	Super::PreRegisterAllComponents();

	// Handle UWorld::AddToWorld(), i.e. turning on level visibility
	if (const ULevel* Level = GetLevel())
	{
		// This function gets called in editor all the time, we're only interested the case where level is being added to world.
		if (Level->bIsAssociatingLevel)
		{
			const bool bSucceeded = RegisterWithSubsystem();
			UE_LOG(LogMassNavigationExt, Verbose, TEXT("\'%s\' (0x%llx) PreRegisterAllComponents - %s"), *GetName(), UPTRINT(this), bSucceeded ? TEXT("Succeeded") : TEXT("Failed"));
		}
	}
}

void ANavigationPathsData::PostUnregisterAllComponents()
{
	// Handle UWorld::RemoveFromWorld(), i.e. turning off level visibility
	if (const ULevel* Level = GetLevel())
	{
		// This function gets called in editor all the time, we're only interested the case where level is being removed from world.
		if (Level->bIsDisassociatingLevel)
		{
			const bool bSucceeded = UnregisterWithSubsystem();
			UE_LOG(LogMassNavigationExt, Verbose, TEXT("\'%s\' (0x%llx) PostUnregisterAllComponents - %s"), *GetName(), UPTRINT(this), bSucceeded ? TEXT("Succeeded") : TEXT("Failed"));
		}
	}

	Super::PostUnregisterAllComponents();
}

void ANavigationPathsData::OnRegistered(const FMassNavigationPathDataHandle DataHandle)
{
	NavigationDataStorage.DataHandle = DataHandle;
}

void ANavigationPathsData::OnUnregistered()
{
	bRegistered = false;
}

bool ANavigationPathsData::RegisterWithSubsystem()
{
	if (!bRegistered && HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		if (UMassNavigationDataStorageSubsystem* MassNav = UWorld::GetSubsystem<UMassNavigationDataStorageSubsystem>(GetWorld()))
		{
			MassNav->RegisterMassNavigationData(*this);
			//UpdateDrawing();
			return true;
		}
	}
	return false;
}

bool ANavigationPathsData::UnregisterWithSubsystem()
{
	if (bRegistered)
	{
		if (UMassNavigationDataStorageSubsystem* NavigationDataStorageSubsystem = UWorld::GetSubsystem<UMassNavigationDataStorageSubsystem>(GetWorld()))
		{
			NavigationDataStorageSubsystem->UnregisterMassNavigationData(*this);
			NavigationDataStorage.DataHandle.Reset();
			//UpdateDrawing();
			return true;
		}
	}
	return false;
}
