// Fill out your copyright notice in the Description page of Project Settings
#pragma once

#include "../Public/WindNoiseGrid.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "NoiseFloatableActor.h"
#include "Source.h"


// Sets default values
AWindNoiseGrid::AWindNoiseGrid()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	AWindNoiseGrid::SetActorHiddenInGame(true);

	if(InstancedStaticMeshComponent == nullptr)
	{
		InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshComponentTest"));
	}
	
	//todo try setting as root component ad see if it works
	//SetRootComponent(InstancedStaticMeshComponent);
	SetRootComponent(Root);
}

// Called when the game starts or when spawned
void AWindNoiseGrid::BeginPlay()
{
	Super::BeginPlay();
	grid = new NoiseGen::Grid(GridWidth, GridHeight, CellSize);
	// class ASurvivalGameGameModeBase* SurvivalGameMode = Cast<ASurvivalGameGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	//
	// UE_LOG(LogTemp, Display, TEXT("Cast to game base class from bp result: %d"), (SurvivalGameMode != nullptr));
	//
	// if(SurvivalGameMode != nullptr && SurvivalGameMode->WindNoiseGrid == nullptr)
	// {
	// 	SurvivalGameMode->WindNoiseGrid = this;
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("Noise wind grid is already referenced in game mode."));
	// }

	//set a reference to self (if not already set) by GetGameMode()->CastToCustomGameMode->Set the right reference to self
	//alternatively game mode can be assigned in BP and accessed directly
	
	if(GEngine)
	{
		const std::string text = grid->GridInfo();
		GEngine->AddOnScreenDebugMessage(2, 5, FColor::Emerald, text.c_str());
	}
	//
	if(InstancedStaticMeshComponent)
	{
		UE_LOG(LogTemp, Display, TEXT("Instanced static mesh is not null"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Instanced static mesh was null and has been created."));
	}
	
	DebugMeshSpawnOnGrid();
}

// Called every frame

void AWindNoiseGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DebugMeshFollowNoise(DeltaTime);
}

FVector AWindNoiseGrid::GetGridCellOriginWorldPosition(int x, int y)
{
	return VecNToFVector(grid->GetGridCellOriginFromIndices(x, y)) + this->GetActorLocation();
}

FVector AWindNoiseGrid::GetGridCellCenterWorldPosition(int x, int y)
{
	return VecNToFVector(grid->GetGridCellCenterFromIndices(x, y)) + this->GetActorLocation();
}

FVector AWindNoiseGrid::GetGridCellOriginRelativePosition(int x, int y)//relative to the grid
{
	return VecNToFVector(grid->GetGridCellOriginFromIndices(x, y));
}

FVector AWindNoiseGrid::GetGridCellCenterRelativePosition(int x, int y)
{
	return VecNToFVector(grid->GetGridCellCenterFromIndices(x, y));
}

FVector AWindNoiseGrid::GetGridUnitsPositionFromWorldPosition(const FVector& worldPosition)
{
	return VecNToFVector(grid->GetGridUnitsPosition(worldPosition.X, worldPosition.Y));
}

void AWindNoiseGrid::CalculateWindDirectionInsideCell2D(FVector2D cellPosition)
{
	//AWindNoiseGrid::NoiseDerivative.Damping = 2.f;
}

FVector AWindNoiseGrid::GetWindVectorAtLocation(const FVector Location)
{
	auto GridPosition = GetGridUnitsPositionFromWorldPosition(Location);//to do hide this implementation details inside grid actor
	const auto WindVector = NoiseDerivative.GetWindVectorForLocation(MoveTemp(GridPosition), this);
	return WindVector;
}

//Debugging functionality:
void AWindNoiseGrid::DebugMeshSpawnOnGrid()
{
	if(InstancedStaticMeshComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Instanced static mesh component was null and it shouldn't have happened!"));
		return;
	}
	
	if(SpawnDebugMesh == false)
	{
		return;
	}
	
	CollisionQueryParams.AddIgnoredComponent(InstancedStaticMeshComponent);
	
	for(int x = 0; x < GridWidth; x++)
	{
		for(int y = 0; y < GridHeight; y++)
		{
			FVector position = UseRelativeCoordinates ? GetGridCellCenterRelativePosition(x, y) : GetGridCellCenterWorldPosition(x, y);
			//UE_LOG(LogTemp, Display, TEXT("Grid X: %d, grid Y: %d"), x, y);
			
			DrawDebugSphere(this->GetWorld(), position, 10.f, 8, FColor::Turquoise, false, 60);
			DrawDebugBox(
				this->GetWorld(),
				position,
				FVector(CellSize/2, CellSize/2, CellSize/2), FColor::Emerald,
				true,
				60);
			
			FVector EndPosition = position + FVector{0, 0, -1.f} * 100.f * CellSize;
			FHitResult OutHit;
			
			DrawDebugLine(this->GetWorld(), position, EndPosition, FColor::Purple, false, MoveDebugMesh ? this->GetWorld()->DeltaTimeSeconds : 60.f, 0, 5.f);
			if (GetWorld()->LineTraceSingleByChannel(OutHit, position, EndPosition, TraceDebugChannel, CollisionQueryParams))
			{
				position = FVector{position.X, position.Y, OutHit.Location.Z + 100.f};
				EndPosition = OutHit.ImpactPoint;
			}
			
			DrawDebugPoint(this->GetWorld(), OutHit.ImpactPoint, 5.f, FColor::Red, false,MoveDebugMesh ? this->GetWorld()->DeltaTimeSeconds : 5.f);
			
			FTransform newTransform {this->GetTransform().GetRotation(), position};
			
			const auto InstancedMeshIndex
			= InstancedStaticMeshComponent->AddInstance(newTransform, true);

			//auto index = (x+y) % InstancedMaterials.Num();
			//InstancedStaticMeshComponent->SetMaterial(InstancedMeshIndex, InstancedMaterials[0]);
		}
	}
}

void AWindNoiseGrid::DebugMeshFollowNoise(float DeltaTime)
{
	if(SpawnDebugMesh == false || MoveDebugMesh == false)
	{
		return;
	}
	
	if(InstancedStaticMeshComponent == nullptr)
	{
		return;
	}
	
	FTransform InstanceTransform;
	InstancedStaticMeshComponent->SetMobility(EComponentMobility::Movable);
	auto InstanceBodies = InstancedStaticMeshComponent->InstanceBodies;
	
	for(auto& body : InstanceBodies)
	{
		body->InstanceBodyIndex;

		InstancedStaticMeshComponent->SetMobility(EComponentMobility::Movable);
		InstancedStaticMeshComponent->GetInstanceTransform(body->InstanceBodyIndex, InstanceTransform, true);
		
		FVector position = InstanceTransform.GetLocation();

		FVector StartPosition = position;// + FVector{0, 0, 1} * 50.f * CellSize;
		FVector EndPosition = position + FVector{0, 0, -1} * 100.f * CellSize;
		FHitResult OutHit;
		float NewZ = 0;
		if (GetWorld()->LineTraceSingleByChannel(OutHit, StartPosition, EndPosition, TraceDebugChannel, CollisionQueryParams))
		{
			NewZ = (OutHit.ImpactPoint.Z + 100.f) - InstanceTransform.GetLocation().Z;
			DrawDebugPoint(this->GetWorld(), OutHit.ImpactPoint, 10.f, FColor::Red, false, 10);
			EndPosition = OutHit.ImpactPoint;
		}
		
		DrawDebugLine(this->GetWorld(), position, EndPosition, FColor::Purple, false, 0, 0, 0.1f);

		const FVector gridPosition = VecNToFVector(grid->GetGridUnitsPosition(InstanceTransform.GetLocation().X, InstanceTransform.GetLocation().Y));//todo necessary!!! call behind the scenes

		FVector offset = (DeltaTime * GridOffsetSpeed * GridOffsetDirection)/CellSize;
		NoiseDerivative.Offset += offset;
		FVector noiseTranslation = NoiseDerivative.GetWindVectorForLocation(static_cast<FVector>(std::move(gridPosition)), this);
		noiseTranslation.Z = NewZ;
		InstanceTransform.AddToTranslation(noiseTranslation);
		
		auto bMoved = InstancedStaticMeshComponent->UpdateInstanceTransform(body->InstanceBodyIndex, InstanceTransform, true, true);
	}
}
