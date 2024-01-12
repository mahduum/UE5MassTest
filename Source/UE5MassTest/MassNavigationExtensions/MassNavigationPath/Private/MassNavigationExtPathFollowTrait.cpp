// Fill out your copyright notice in the Description page of Project Settings.


#include "MassNavigationExtPathFollowTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "MassNavigationExtFragments.h"
#include "MassNavigationFragments.h"

void UMassNavigationExtPathFollowTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,
                                                      const UWorld& World) const
{
	//FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	BuildContext.RequireFragment<FAgentRadiusFragment>();
	BuildContext.RequireFragment<FTransformFragment>();
	BuildContext.RequireFragment<FMassVelocityFragment>();
	BuildContext.RequireFragment<FMassMoveTargetFragment>();

	BuildContext.AddFragment<FMassNavigationPathLaneLocationFragment>();
	BuildContext.AddFragment<FMassNavigationShortPathRequestFragment>();
	BuildContext.AddFragment<FMassNavigationShortPathFragment>();
	BuildContext.AddFragment<FMassNavigationCachedPathLaneFragment>();

	//const FConstSharedStruct ZGMovementParamsFragment = EntityManager.GetOrCreateConstSharedFragment(NavigationParameters);
	//BuildContext.AddConstSharedFragment(ZGMovementParamsFragment);
}
