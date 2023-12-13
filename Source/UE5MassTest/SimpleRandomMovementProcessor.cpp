// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleRandomMovementProcessor.h"

#include <string>

#include "SimpleRandomMovementTrait.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassExecutionContext.h"
#include "PlaceablesObserver.h"
#include "VisualLogger/VisualLogger.h"

USimpleRandomMovementProcessor::USimpleRandomMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void USimpleRandomMovementProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FSimpleMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void USimpleRandomMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, ([this](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> TransformsList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FSimpleMovementFragment> SimpleMovementList = Context.GetMutableFragmentView<FSimpleMovementFragment>();
		const float WorldDeltaTime = Context.GetDeltaTimeSeconds();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FTransform& Transform = TransformsList[EntityIndex].GetMutableTransform();
			FVector& MoveTarget = SimpleMovementList[EntityIndex].Target;

			FVector CurrentLocation = Transform.GetLocation();
			FVector TargetVector = MoveTarget - CurrentLocation;

			if (TargetVector.Size() <= 20.f)
			{
				MoveTarget = FVector(FMath::RandRange(-1.f, 1.f) * 1000.f, FMath::RandRange(-1.f, 1.f) * 1000.f, CurrentLocation.Z);
			}
			else
			{
				Transform.SetLocation(CurrentLocation + (TargetVector.GetSafeNormal() * 400.F * WorldDeltaTime));
			}
		}
	}));
}
/*
 * UMassObserverProcessor will help to initialize fragments that depend on other fragments information or other systems that may not be initialised at construction time
 * for example modify a radius of some units for example for builders it would configure queries by adding SomeRadiusFragment and SomeBuilderTag (:FMassTag)
 * AddTagRequirement<SomeTag>(EMassFragmentPresence::All)
 * it would be also different in constructor instead of order of initialization ObservedType = SomeRadiusFragment::StaticStruct(); and Operation = EMassObserverOperation::Add;
 * it all means that it will be executed just one time at the beginning
 * todo use this for setting initial target for cones
 */
