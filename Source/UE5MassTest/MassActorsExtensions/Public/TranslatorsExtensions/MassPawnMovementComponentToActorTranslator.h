// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <concepts>
#include "CoreMinimal.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassTranslator.h"
#include "UObject/Object.h"
#include "MassPawnMovementComponentToActorTranslator.generated.h"

template <typename PawnMovementComponentDerived>
concept PawnMovementComponentConcept =
	requires (PawnMovementComponentDerived ComponentDerived, const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	{ComponentDerived.GetActorNavLocation()} -> std::same_as<FVector>;
	std::is_same_v<decltype(ComponentDerived.Velocity), FVector&>;
	//std::is_same_v<decltype(std::declval<PawnMovementComponentDerived>().Velocity), FVector&>;//alternative if no concepts
	ComponentDerived.RequestDirectMove(MoveVelocity, bForceMaxSpeed);
};

USTRUCT()
struct FPawnMovementComponentDerivedWrapperFragment : public FObjectWrapperFragment//todo set sync processor and trait to assign the internal component like: UMassAgentMovementSyncTrait::BuildTemplate
{
	GENERATED_BODY()
	TWeakObjectPtr<UPawnMovementComponent> ComponentDerived;//make templated methods with various signatures corresponding to actual type of ComponentDerived to be called differently (like for vehicle vs person)
};

template <PawnMovementComponentConcept ComponentDerived>
void TranslateMovementToActor (FMassExecutionContext& Context)
{
	const TArrayView<FPawnMovementComponentDerivedWrapperFragment> ComponentList = Context.GetMutableFragmentView<FPawnMovementComponentDerivedWrapperFragment>();//todo get this wrapper to be set the right component
	const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
	const int32 NumEntities = Context.GetNumEntities();
	for (int32 i = 0; i < NumEntities; ++i)
	{
		if (ComponentDerived* AsMovementComponent = Cast<ComponentDerived>(ComponentList[i].ComponentDerived.Get()))
		{
			AsMovementComponent->RequestDirectMove(VelocityList[i].Value, /*bForceMaxSpeed=*/false);
		}
	}
};

template <PawnMovementComponentConcept ComponentDerived>
void TranslateMovementToMass (FMassExecutionContext& Context)
{
	const TArrayView<FPawnMovementComponentDerivedWrapperFragment> ComponentList = Context.GetMutableFragmentView<FPawnMovementComponentDerivedWrapperFragment>();//todo get this wrapper to be set the right component
	const TArrayView<FTransformFragment> LocationList = Context.GetMutableFragmentView<FTransformFragment>();
	const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
	const int32 NumEntities = Context.GetNumEntities();
	for (int32 i = 0; i < NumEntities; ++i)
	{
		if (const ComponentDerived* AsPawnMovement = Cast<ComponentDerived>(ComponentList[i].ComponentDerived.Get()))
		{
			LocationList[i].GetMutableTransform().SetLocation(AsPawnMovement->GetActorNavLocation());
			VelocityList[i].Value = AsPawnMovement->Velocity;
		}
	}
};

USTRUCT()
struct FPawnMovementWrapperFragment : public FObjectWrapperFragment
{
	GENERATED_BODY()
	TWeakObjectPtr<UPawnMovementComponent> Component;//use this simply by using virtual methods
};

USTRUCT()
struct FMassPawnMovementCopyToActorTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class UE5MASSTEST_API UMassPawnMovementComponentToActorTranslator : public UMassTranslator
{
	GENERATED_BODY()
public:
	UMassPawnMovementComponentToActorTranslator();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

USTRUCT()
struct FMassPawnMovementCopyToMassTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class UE5MASSTEST_API UMassPawnMovementComponentToMassTranslator : public UMassTranslator
{
	GENERATED_BODY()
public:
	UMassPawnMovementComponentToMassTranslator();
	
protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
