// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassGridSubsystem.generated.h"

class FMassGridCellContent;

UCLASS()
class UE5MASSTEST_API UMassGridSubsystem : public UWorldSubsystem//make TObjectPtr to this in UMassProcessor and grabbed in its Initialize virtual method
{
	GENERATED_BODY()
public:

	UMassGridSubsystem();
	UPROPERTY(BlueprintReadWrite, Category="Test")
	int TestInt;
	//typedef FMassGridCellContent MassGrid [GridDimensions][GridDimensions];//this is a MassGrid alias for a 2d array of FMassGridCellContent
private:
	//MassGrid Grid;
	
};

/**
 * 
 */
typedef FMassGridCellContent MassSubGrid [3][3];

USTRUCT()
struct FMassMovementTargetFragment : public FMassFragment //each entity will keep with it small piece of grid it is on
{
	GENERATED_BODY()
	FVector TargetLocation;
	uint32 CachedGridX;
	uint32 CachedGridY;
	//UMassGridSubsystem::MassSubGrid CachedGrid;
	bool bStartGoing;
};

//FMassMoveTargetFragment
