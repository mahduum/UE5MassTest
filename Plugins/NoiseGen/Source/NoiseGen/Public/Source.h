// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Gridable.h"
#include "Source.generated.h"

class AWindNoiseGrid;
struct FWindNoiseCalculator;
// This class does not need to be modified.
UINTERFACE()
class USource : public UGridable
{
	GENERATED_BODY()
};

/**
 * 
 */
class NOISEGEN_API ISource : public IGridable 
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// this function will take parameters from the grid, lambda for calculating values, and noise derivative for source
	// and upon tick from the grid will call it and update the values/properties
	virtual void UpdateProperties(FVector (FWindNoiseCalculator::*NextPosition)(FVector&&, const UObject*), FWindNoiseCalculator& Calculator);
};
