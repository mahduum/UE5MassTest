// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "UObject/Object.h"
#include "SelectedEntitiesProcessor.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Mass Move Selected"))
class UE5MASSTEST_API USelectedEntitiesProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	USelectedEntitiesProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
