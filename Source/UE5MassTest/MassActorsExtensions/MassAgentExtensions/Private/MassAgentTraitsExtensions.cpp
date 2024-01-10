// Fill out your copyright notice in the Description page of Project Settings.


#include "UE5MassTest/MassActorsExtensions/MassAgentExtensions/Public/MassAgentTraitsExtensions.h"

#include "MassEntityTemplateRegistry.h"
#include "MassEntityView.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "UE5MassTest/PlaceablesObserver.h"
#include "UE5MassTest/MassActorsExtensions/Public/TranslatorsExtensions/MassFloatingPawnMovementToActorTranslator.h"
#include "UE5MassTest/MassActorsExtensions/Public/TranslatorsExtensions/MassPawnMovementComponentToActorTranslator.h"

void UMassAgentAllMovementSyncTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,
                                                   const UWorld& World) const
{
	BuildContext.AddFragment<FFloatingPawnMovementWrapperFragment>();
	BuildContext.AddFragment<FPawnMovementWrapperFragment>();
	BuildContext.RequireFragment<FMassVelocityFragment>();

	BuildContext.GetMutableObjectFragmentInitializers().Add([=](UObject& Owner, FMassEntityView& EntityView, const EMassTranslationDirection TranslationDirection)
	{
		//FPawnMovementComponentDerivedWrapperFragment& WrapperFragment = EntityView.GetFragmentData<FPawnMovementComponentDerivedWrapperFragment>();
		//FPawnMovementWrapperFragment& WrapperFragment = EntityView.GetFragmentData<FPawnMovementWrapperFragment>();
		FFloatingPawnMovementWrapperFragment& WrapperFragment = EntityView.GetFragmentData<FFloatingPawnMovementWrapperFragment>();
		// if (UCharacterMovementComponent* MovementComp = FMassAgentComponentsExtractor::AsComponent<UCharacterMovementComponent>(Owner))
		// {
		// 	WrapperFragment.Component = MovementComp;
		//
		// 	FMassVelocityFragment& VelocityFragment = EntityView.GetFragmentData<FMassVelocityFragment>();
		// 	if (TranslationDirection == EMassTranslationDirection::MassToActor)
		// 	{
		// 		MovementComp->bRunPhysicsWithNoController = true;
		// 		MovementComp->SetMovementMode(EMovementMode::MOVE_Walking);
		// 		MovementComp->Velocity = VelocityFragment.Value;
		// 	}
		// 	// actor is the authority
		// 	else
		// 	{
		// 		VelocityFragment.Value = MovementComp->GetLastUpdateVelocity();
		// 	}
		// }

		if (UFloatingPawnMovement* MovementComp = FMassAgentComponentsExtractor::AsComponent<UFloatingPawnMovement>(Owner))
		{
			//UE_LOG(LogPlaceableSpawner, Display, TEXT("Floating Pawn Movement component found and assigned in All Movement Sync Trait."))

			WrapperFragment.Component = MovementComp;

			FMassVelocityFragment& VelocityFragment = EntityView.GetFragmentData<FMassVelocityFragment>();
			if (TranslationDirection == EMassTranslationDirection::MassToActor)
			{
				//UE_LOG(LogPlaceableSpawner, Display, TEXT("Floating Pawn Movement MassToActor init set in All Movement Sync Trait, Pawn Velocity set to: %s, magnitude: %f"), *VelocityFragment.Value.ToString(), VelocityFragment.Value.Size())
				MovementComp->Velocity = VelocityFragment.Value;
				MovementComp->UpdateComponentVelocity();
				MovementComp->bAutoUpdateTickRegistration = true;
				MovementComp->bAutoRegisterPhysicsVolumeUpdates = true;
				MovementComp->MovementState.bCanWalk = true;
			}
			// actor is the authority
			else
			{
				//UE_LOG(LogPlaceableSpawner, Display, TEXT("Floating Pawn Movement ActorToMass init set in All Movement Sync Trait, Mass Velocity set to: %s"), *MovementComp->Velocity.ToString())
				VelocityFragment.Value = MovementComp->Velocity;
			}
		}
	});

	if (EnumHasAnyFlags(SyncDirection, EMassTranslationDirection::ActorToMass))
	{
		UE_LOG(LogPlaceableSpawner, Display, TEXT("Mass Agent Ext adding ActorToMass translator."))
		//BuildContext.AddTranslator<UMassPawnMovementComponentToMassTranslator>();
		BuildContext.AddTranslator<UMassFloatingPawnMovementToMassTranslator>();
	}

	if (EnumHasAnyFlags(SyncDirection, EMassTranslationDirection::MassToActor))
	{
		UE_LOG(LogPlaceableSpawner, Display, TEXT("Mass Agent Ext adding MassToActor translator."))
		//BuildContext.AddTranslator<UMassPawnMovementComponentToActorTranslator>();
		BuildContext.AddTranslator<UMassFloatingPawnMovementToActorTranslator>();
	}
}
