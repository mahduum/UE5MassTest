// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Gridable.generated.h"

class AWindNoiseGrid;
// This class does not need to be modified.
UINTERFACE()
class UGridable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NOISEGEN_API IGridable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Initialize(TWeakObjectPtr<AWindNoiseGrid> WindNoiseGrid);
};
