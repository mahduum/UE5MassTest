#pragma once
#include "MassEntityTypes.h"
#include "CoreMinimal.h"
#include "MassTestFragments.generated.h"

USTRUCT()
struct FPlaceableFragment : public FMassFragment
{
	GENERATED_BODY()
	int32 Index = 0;
};
