// Fill out your copyright notice in the Description page of Project Settings.


#include "MassRaycastHitSubsystem.h"
#include "ClickedActorSignalProcessor.h"
#include "MassActorSubsystem.h"
#include "MassExecutor.h"
#include "MassSignalSubsystem.h"
#include "PlaceablesObserver.h"
#include "EntitySystem/MovieSceneEntityManager.h"
#include "VisualLogger/VisualLogger.h"

void UMassRaycastHitSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	MassSelectedProcessor = NewObject<USelectedEntitiesProcessor>();
	//FMassEntityManager& Manager = UE::Mass::Utils::GetEntityManagerChecked(InWorld);
}

void UMassRaycastHitSubsystem::SingleLineTraceActorToEntity(APlayerController* PlayerController, float TraceLength, bool bDebugQuery)
{
	SingleTraceForSignal(UE::Mass::Signals::Selected, PlayerController, TraceLength, bDebugQuery);
}
//TODO: return or cache click coords
void UMassRaycastHitSubsystem::SingleLineTraceMoveTarget(APlayerController* PlayerController, float TraceLength,
                                                         bool bDebugQuery)
{
	SingleTraceForSignal(UE::Mass::Signals::MoveTarget, PlayerController, TraceLength, bDebugQuery);//todo split impl, signal should only respond to signals
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

void UMassRaycastHitSubsystem::SingleTraceForSignal(FName SignalName, APlayerController* PlayerController,
                                                    float TraceLength, bool bDebugQuery) const
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
		ProcessSingleLineTraceResult(HitResult, SignalName);
	}
}

bool UMassRaycastHitSubsystem::TrySendSelectSignal(AActor* HitActor) const
{
	if(HitActor)
	{
		//use subsystem actor to entity
		const FMassEntityHandle Entity = ActorSubsystem->GetEntityHandleFromActor(HitActor);

		if (Entity == FMassEntityManager::InvalidEntity || Entity.IsSet() == false)
		{
			UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("%s: actor is not a valid entity! Entity is set: %d"), *HitActor->GetName(), Entity.IsSet());
			return false;
		}
			
		UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("%s: entity extracted form actor: %s"), *Entity.DebugGetDescription(), *HitActor->GetName());
		//use signal subsystem (or make this one a signal subsystem)
		//we also need to run the system that had previously selected entity
		SignalSubsystem->SignalEntity(UE::Mass::Signals::Selected, Entity);//processor that subscribed to this
		return true;
	}
	return false;
}

void UMassRaycastHitSubsystem::SetMoveTarget(const FHitResult& HitResult) const
{
	//Signal or update processor that will attach move target with specific data for selected entities
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*((this->GetWorld())));
	FMassProcessingContext ProcessingContext(EntityManager, 0.0f);
	//add aux data
	UE_VLOG_UELOG(this, LogPlaceableSpawner, Log, TEXT("Setting move target location as aux data: %s"), *HitResult.Location.ToString());
	
	const FInstancedStruct MoveTarget = FInstancedStruct::Make(HitResult.Location);
	ProcessingContext.AuxData = MoveTarget;
	UE::Mass::Executor::Run(*MassSelectedProcessor, ProcessingContext);
}

void UMassRaycastHitSubsystem::ProcessSingleLineTraceResult(const FHitResult& HitResult, FName SignalName) const
{
	if (SignalName == UE::Mass::Signals::MoveTarget)//todo or chose this if game/player is in "has selected state", or try select will not have effect if there is state to choose move location
	{
		SetMoveTarget(HitResult);
	}
	else if (SignalName == UE::Mass::Signals::Selected)
	{
		TrySendSelectSignal(HitResult.GetActor());
	}
}
