// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassNavigationExtProcessors.generated.h"

class UMassSignalSubsystem;
/**
 * 
 */
UCLASS()
class UE5MASSTEST_API UMassNavPathFollowProcessor : public UMassProcessor
{
	GENERATED_BODY()

protected:
	UMassNavPathFollowProcessor();
	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery_Conditional;

	/*This is needed for state tree signaling when path changes or is done. */
	UPROPERTY(Transient)
	TObjectPtr<UMassSignalSubsystem> SignalSubsystem = nullptr;
};
