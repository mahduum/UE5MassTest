// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCommonFragments.h"
#include "MassTranslator.h"
#include "UObject/Object.h"
#include "MassFloatingPawnMovementToActorTranslator.generated.h"

class UFloatingPawnMovement;

/*
 * Must transform movement translation to add input vector
 */
USTRUCT()
struct FFloatingPawnMovementWrapperFragment : public FObjectWrapperFragment
{
	GENERATED_BODY()
	TWeakObjectPtr<UFloatingPawnMovement> Component;
};

USTRUCT()
struct FMassFloatingPawnMovementCopyToActorTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class UE5MASSTEST_API UMassFloatingPawnMovementToActorTranslator : public UMassTranslator
{
	GENERATED_BODY()

public:
	UMassFloatingPawnMovementToActorTranslator();
	
protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

USTRUCT()
struct FMassFloatingPawnMovementCopyToMassTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class UE5MASSTEST_API UMassFloatingPawnMovementToMassTranslator: public UMassTranslator
{
	GENERATED_BODY()

public:
	UMassFloatingPawnMovementToMassTranslator();
	
protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
