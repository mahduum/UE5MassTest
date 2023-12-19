// Fill out your copyright notice in the Description page of Project Settings.


#include "UE5MassTest/MassActorsExtensions/Public/TranslatorsExtensions/MassPawnMovementComponentToActorTranslator.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "UE5MassTest/PlaceablesObserver.h"

UMassPawnMovementComponentToActorTranslator::UMassPawnMovementComponentToActorTranslator() : EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	RequiredTags.Add<FMassPawnMovementCopyToActorTag>();
}

void UMassPawnMovementComponentToActorTranslator::ConfigureQueries()
{
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FPawnMovementWrapperFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	//to use when there is no move to target:
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassPawnMovementComponentToActorTranslator::Execute(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		const TArrayView<FPawnMovementWrapperFragment> ComponentList = Context.GetMutableFragmentView<FPawnMovementWrapperFragment>();//todo get this wrapper to be set the right component
		const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (UPawnMovementComponent* AsMovementComponent = ComponentList[i].Component.Get())
			{
				if(VelocityList[i].Value.Size() > 0.001)
				{
					UE_LOG(LogPlaceableSpawner, Display, TEXT("Pawn Movement component request direct move with value: %f, location: %s, for entity index: %d, actor name: %s"), VelocityList[i].Value.Size(), *TransformList[i].GetTransform().GetLocation().ToString(), i, *AsMovementComponent->GetOwner()->GetName());
				}
				AsMovementComponent->RequestDirectMove(VelocityList[i].Value, /*bForceMaxSpeed=*/false);
				//AsMovementComponent->RequestDirectMove(FVector(500.0f, 0, 0), /*bForceMaxSpeed=*/false);
				//AsMovementComponent->GetOwner()->SetActorLocation(TransformList[i].GetTransform().GetLocation());
			}
		}
	});
}

UMassPawnMovementComponentToMassTranslator::UMassPawnMovementComponentToMassTranslator() : EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorldToMass;
	RequiredTags.Add<FMassPawnMovementCopyToMassTag>();
}

void UMassPawnMovementComponentToMassTranslator::ConfigureQueries()
{
	EntityQuery.AddRequirement<FPawnMovementWrapperFragment>(EMassFragmentAccess::ReadOnly);//todo temp
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassPawnMovementComponentToMassTranslator::Execute(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		//wrapper remains the same
		const TConstArrayView<FPawnMovementWrapperFragment> ComponentList = Context.GetFragmentView<FPawnMovementWrapperFragment>();//todo get this wrapper to be set the right component
		const TArrayView<FTransformFragment> LocationList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

		const int32 NumEntities = Context.GetNumEntities();
		
		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (const UPawnMovementComponent* AsPawnMovement = ComponentList[i].Component.Get())
			{
				LocationList[i].GetMutableTransform().SetLocation(AsPawnMovement->GetActorNavLocation());
				VelocityList[i].Value = AsPawnMovement->Velocity;
			}
		}
	});
}

