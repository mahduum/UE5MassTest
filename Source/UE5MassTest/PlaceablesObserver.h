// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "PlaceableMassSpawnLocationSubsystem.h"
#include "PlaceablesObserver.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlaceableSpawner, Log, All);

/**
 * 
 */
UCLASS()
class UE5MASSTEST_API UPlaceablesObserver : public UMassObserverProcessor
{
	GENERATED_BODY()
public:
	UPlaceablesObserver();

protected:
	virtual void ConfigureQueries() override;
	//virtual void PostInitProperties() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;

private:
	TObjectPtr<UPlaceableMassSpawnLocationSubsystem> PlaceableMassSpawnLocationSubsystem;
};
