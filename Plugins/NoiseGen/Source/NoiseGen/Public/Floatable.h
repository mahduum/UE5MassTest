// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Gridable.h"
#include "Floatable.generated.h"

class AWindNoiseGrid;
struct FWindNoiseCalculator;
// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UFloatable : public UGridable
{
	GENERATED_BODY()
};

/**
 * 
 */
class NOISEGEN_API IFloatable : public IGridable
{
	GENERATED_BODY()
	 
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FVector GetLocation();//to remove
	virtual void SetLocation(const FVector& position);//to remove
	virtual void LifeSpanExpired();//to remove
	virtual void AddToGrid();//to remove
	virtual void RemoveFromGrid();//to remove
	virtual bool IsPendingKillOrDestroyed();//to remove
	//first param says: I return FVector from some member function pointer here named NextPosition of FWindNoiseCalculator, and I take in position, and UObject that is context the implementor
	//and I need also the owner of the member function, then I can take the Calculator and call it with NextPosition, passing in location of the implementor and the reference to it (for context)
	virtual void UpdateTranslation(FVector (FWindNoiseCalculator::*NextPosition)(FVector&&, const UObject*), FWindNoiseCalculator& Calculator);//TODO maybe make this one Update that will always take this args, what else can it generate apart from vector?
};
