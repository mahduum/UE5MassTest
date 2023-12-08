// Fill out your copyright notice in the Description page of Project Settings.


#include "MassRaycastHitSubsystem.h"
#include "ClickedActorSignalProcessor.h"
#include "MassActorSubsystem.h"
#include "MassSignalSubsystem.h"
#include "PlaceablesObserver.h"
#include "VisualLogger/VisualLogger.h"

void UMassRaycastHitSubsystem::SingleLineTraceActorToEntity(APlayerController* PlayerController, float TraceLength, bool bDebugQuery)
{
	FVector WorldLocation;
	FVector WorldDirection;
	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
	FVector End = WorldDirection * TraceLength;
	FCollisionQueryParams Params;
	Params.bDebugQuery = bDebugQuery;
	Params.AddIgnoredActor(PlayerController->GetOwner());
	//Todo trace channel set to "selectable" and make it!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (FHitResult HitResult; GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, End, ECollisionChannel::ECC_Visibility, Params, FCollisionResponseParams::DefaultResponseParam))
	{
		if(AActor* HitActor = HitResult.GetActor())
		{
			//use subsystem actor to entity
			const FMassEntityHandle Entity = ActorSubsystem->GetEntityHandleFromActor(HitActor);

			if (Entity == FMassEntityManager::InvalidEntity || Entity.IsSet() == false)
			{
				UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("%s: actor is not a valid entity!"), *HitActor->GetName());
				//UE_LOG(LogPlaceableSpawner, Log, TEXT("%s: actor is not a valid entity!"), *HitActor->GetName());
				return;
			}
			
			UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("%s: entity extracted form actor: %s"), *Entity.DebugGetDescription(), *HitActor->GetName());
			//UE_LOG(LogPlaceableSpawner, Log, TEXT("%s: entity extracted form actor: %s"), *Entity.DebugGetDescription(), *HitActor->GetName());
			//use signal subsystem (or make this one a signal subsystem)
			//we also need to run the system that had previously selected entity
			SignalSubsystem->SignalEntity(UE::Mass::Signals::Selected, Entity);//processor that subscribed to this 
		}
	}
}

void UMassRaycastHitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	SignalSubsystem = Collection.InitializeDependency<UMassSignalSubsystem>();
	checkfSlow(SignalSubsystem != nullptr, TEXT("MassSignalSubsystem is required"));

	AgentSubsystem = Collection.InitializeDependency<UMassAgentSubsystem>();
	checkfSlow(AgentSubsystem != nullptr, TEXT("MassAgentSubsystem is required"));

	ActorSubsystem = Collection.InitializeDependency<UMassActorSubsystem>();
	checkfSlow(AgentSubsystem != nullptr, TEXT("MassAgentSubsystem is required"));
}
