// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "UObject/Object.h"
#include "PlaceableTrait.generated.h"


USTRUCT()
struct FNoLocationTag : public FMassTag
{
	GENERATED_BODY()
};
/**
 * 
 */
UCLASS()
class UE5MASSTEST_API UPlaceableTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
