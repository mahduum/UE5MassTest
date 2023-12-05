// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleRandomMovementTrait.h"
#include "MassEntityTemplateRegistry.h"

void USimpleRandomMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	//Super::BuildTemplate(BuildContext, World);
	BuildContext.AddFragment<FSimpleMovementFragment>();
}
