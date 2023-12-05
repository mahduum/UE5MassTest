// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "NoiseFloatableActor.h"

// Sets default values
ANoiseFloatableActor::ANoiseFloatableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANoiseFloatableActor::BeginPlay()
{
	Super::BeginPlay();
}

void ANoiseFloatableActor::Initialize(TWeakObjectPtr<AWindNoiseGrid> NoiseGrid)
{
	WindNoiseGrid = NoiseGrid;
	//AddToGrid();
}

void ANoiseFloatableActor::AddToGrid()
{

}

void ANoiseFloatableActor::UpdateTranslation(FVector (FWindNoiseCalculator::*NextPosition)(FVector&&, const UObject*), FWindNoiseCalculator& Calculator)//or std::function<FVector(FVector&, UObject*)>
{
	SetActorLocation((Calculator.*NextPosition)(GetActorLocation(), this));
}

// Called every frame
void ANoiseFloatableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

