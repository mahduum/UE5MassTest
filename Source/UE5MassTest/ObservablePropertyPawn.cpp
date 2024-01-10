// Fill out your copyright notice in the Description page of Project Settings.


#include "ObservablePropertyPawn.h"

#include "PlaceablesObserver.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "MassActorsExtensions/Public/TranslatorsExtensions/MassFloatingPawnMovementToActorTranslator.h"


// Sets default values
AObservablePropertyPawn::AObservablePropertyPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AObservablePropertyPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AObservablePropertyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AObservablePropertyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AObservablePropertyPawn::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;


	UE_LOG(LogPlaceableSpawner, Display, TEXT("Property change with name: %s"), *PropertyName.ToString());

	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UFloatingPawnMovement, Velocity))
	{
		const UObject* Object = PropertyChangedEvent.GetObjectBeingEdited(0);
		// Notify listeners that MyIntProperty has changed
		FVector Vel = FVector::Zero();
		if(auto Comp = Cast<UFloatingPawnMovement>(Object))
		{
			//UE_LOG(LogPlaceableSpawner, Display, TEXT("Floating Pawn Movement Velocity mag changed to: %f"), Vel.Size());
			Vel = Comp->Velocity;
		}
		//UE_LOG(LogPlaceableSpawner, Display, TEXT("Floating Pawn Movement Velocity mag changed to: %f"), Vel.Size());
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

