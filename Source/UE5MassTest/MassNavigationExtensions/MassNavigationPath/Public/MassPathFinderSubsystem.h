// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassNavigationDataStorageSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassPathFinderSubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAsyncPathFound, ENavigationQueryResult::Type, Result, const TArray<FVector>&, PathPointLocations);

UCLASS()
class UE5MASSTEST_API UMassPathFinderSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Mass Navigation Ext")
	void AsyncFindMassPath(const FVector& StartLocation, const FVector& EndLocation, const APawn* Agent);//accept delegate todo

	UPROPERTY(BlueprintAssignable, Category="Mass Navigation Ext")
	FOnAsyncPathFound OnPathFoundDebug;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	UPROPERTY()
	TObjectPtr<UMassNavigationDataStorageSubsystem> NavStorageSubsystem;
};
