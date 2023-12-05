// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaceableTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassTestFragments.h"

void UPlaceableTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FPlaceableFragment>();
	BuildContext.AddFragment<FAgentRadiusFragment>();
	BuildContext.AddFragment<FTransformFragment>();
	//BuildContext.AddTag<FNoLocationTag>();
}
