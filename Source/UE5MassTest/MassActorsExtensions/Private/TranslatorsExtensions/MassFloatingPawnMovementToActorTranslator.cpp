// Fill out your copyright notice in the Description page of Project Settings.


#include "UE5MassTest/MassActorsExtensions/Public/TranslatorsExtensions/MassFloatingPawnMovementToActorTranslator.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "UE5MassTest/PlaceablesObserver.h"
#include "UE5MassTest/MassActorsExtensions/Public/TranslatorsExtensions/MassPawnMovementComponentToActorTranslator.h"

UMassFloatingPawnMovementToActorTranslator::UMassFloatingPawnMovementToActorTranslator() : EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	RequiredTags.Add<FMassFloatingPawnMovementCopyToActorTag>();
}

void UMassFloatingPawnMovementToActorTranslator::ConfigureQueries()
{
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FFloatingPawnMovementWrapperFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassFloatingPawnMovementToActorTranslator::Execute(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		const TArrayView<FFloatingPawnMovementWrapperFragment> ComponentList = Context.GetMutableFragmentView<FFloatingPawnMovementWrapperFragment>();
		const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		
		for (int32 i = 0; i < NumEntities; ++i)
		{
			if (UFloatingPawnMovement* AsMovementComponent = ComponentList[i].Component.Get())
			{
				UE_LOG(LogPlaceableSpawner, Display, TEXT("Floating Pawn Movement component request direct move with value: %f, location: %s, for entity index: %d, actor name: %s"), VelocityList[i].Value.Size(), *TransformList[i].GetTransform().GetLocation().ToString(), i, *AsMovementComponent->GetOwner()->GetName());
				AsMovementComponent->RequestDirectMove(/*VelocityList[i].Value*/FVector(500.0f, 0, 0 ), /*bForceMaxSpeed=*/false);
				AsMovementComponent->AddInputVector(FVector(0.2, 0, 0 ));
				AsMovementComponent->UpdateComponentVelocity();//or set placed in world or spawned
				//or: MoveUpdatedComponent
			}
		}
		
		//TranslateMovementToActor<UFloatingPawnMovement>(Context);
		//TranslateMovementToActor<UPawnMovementComponent>(Context);
	});
}

UMassFloatingPawnMovementToMassTranslator::UMassFloatingPawnMovementToMassTranslator() : EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorldToMass;
	RequiredTags.Add<FMassFloatingPawnMovementCopyToMassTag>();
}

void UMassFloatingPawnMovementToMassTranslator::ConfigureQueries()
{
	EntityQuery.AddRequirement<FFloatingPawnMovementWrapperFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
}

void UMassFloatingPawnMovementToMassTranslator::Execute(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		 const TConstArrayView<FFloatingPawnMovementWrapperFragment> ComponentList = Context.GetFragmentView<FFloatingPawnMovementWrapperFragment>();
		 const TArrayView<FTransformFragment> LocationList = Context.GetMutableFragmentView<FTransformFragment>();
		 const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
		
		 const int32 NumEntities = Context.GetNumEntities();
		
		 for (int32 i = 0; i < NumEntities; ++i)
		 {
		 	if (const UFloatingPawnMovement* AsFloatingPawnMovement = ComponentList[i].Component.Get())
		 	{
		 		LocationList[i].GetMutableTransform().SetLocation(AsFloatingPawnMovement->GetActorNavLocation());
		 		VelocityList[i].Value = AsFloatingPawnMovement->Velocity;
		 	}
		 }
		//TranslateMovementToMass<UFloatingPawnMovement>(Context);
		//TranslateMovementToMass<UPawnMovementComponent>(Context);
	});;
}

// Check if the type has the required methods and properties
// 	static_assert(std::is_member_function_pointer_v<decltype(&MovementComponentType::GetActorNavLocation)>, "Type must have GetActorNavLocation method");
// 	static_assert(std::is_same_v<decltype(std::declval<MovementComponentType>().Velocity), FVector>, "Type must have Velocity property");
//  */
