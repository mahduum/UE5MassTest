// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Floatable.h"

#include "WindNoiseCalculator.h"
#include "../sdk/include/Grid.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "WindNoiseGrid.generated.h"

class AWindNoiseGrid;
class ANoiseFloatableActor;
class UGridable;

USTRUCT()
struct FGridCellData
{
	GENERATED_BODY()
	//stores or calculates wind direction for a single cell when necessary
	//TODO remember that scent bubble gets bigger with distance but also less intense
	float CellSize;//
    FVector Position;
    FVector WindDirection;
};

DECLARE_DELEGATE_TwoParams(FUpdateFloatable, IFloatable&, AWindNoiseGrid*)//unused

UCLASS()
class NOISEGEN_API AWindNoiseGrid : public AActor//todo make just grid system that is configurable from blue pring
{
	GENERATED_BODY()

	//typedef void (IFloatable::* UpdateFloatable)(AWindNoiseGrid*);
	friend class SharedPointerInternals::TIntrusiveReferenceController<AWindNoiseGrid, ESPMode::ThreadSafe>;
	
	public:
	// Sets default values for this actor's properties
	AWindNoiseGrid();

	protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void DebugMeshFollowNoise(float DeltaTime);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GameMode")//TODO try assigning in bp and delete if not necessary
	// TSubclassOf<class ASurvivalGameGameModeBase> BP_SurvivalGameMode;
	
	UPROPERTY(EditDefaultsOnly, Category="Grid Static Properties")
	int GridWidth;

	UPROPERTY(EditDefaultsOnly, Category="Grid Static Properties")
	int GridHeight;

	UPROPERTY(EditDefaultsOnly, Category="Grid Static Properties")
	float CellSize;

	UPROPERTY(EditAnywhere, Category="Grid Dynamic Properties")
	float GridOffsetSpeed;

	UPROPERTY(EditAnywhere, Category="Grid Dynamic Properties")
	FVector GridOffsetDirection;

	UPROPERTY(EditAnywhere, Category="Debug Properties")
	UStaticMeshComponent* DebugMesh;

	UPROPERTY()
	TArray<UStaticMeshComponent*> SpawnedMeshes;

	UPROPERTY(EditAnywhere, Category="Debug Properties")
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent;

	UPROPERTY(EditAnywhere, Category="Debug Properties")
	TArray<UMaterialInterface*> InstancedMaterials;

	UPROPERTY(EditAnywhere, Category="Debug Properties")
	bool SpawnDebugMesh;

	UPROPERTY(EditAnywhere, Category="Debug Properties")
	bool MoveDebugMesh;

	UPROPERTY(EditAnywhere, Category="Debug Properties")
	TEnumAsByte<ECollisionChannel> TraceDebugChannel;

	UPROPERTY(EditAnywhere, Category="Grid Static Properties")
	bool UseRelativeCoordinates;
	
	UPROPERTY(EditDefaultsOnly, Category="Noise Properties")
	FWindNoiseCalculator NoiseDerivative;

	UPROPERTY()
	USceneComponent* Root;
	
	TArray<FGridCellData*> GridCells;

	/** Produces world position of grid cell origin based on grid indices. */
	UFUNCTION(BlueprintCallable)
	FVector GetGridCellOriginWorldPosition(int x, int y);

	/** Produces world position of grid cell center based on grid indices. */
	UFUNCTION(BlueprintCallable)
	FVector GetGridCellCenterWorldPosition(int x, int y);

	/** Produces relative position of grid cell origin based on grid indices. */
	UFUNCTION(BlueprintCallable)
	FVector GetGridCellOriginRelativePosition(int x, int y);

	/** Produces relative position of grid cell center based on grid indices. */
	UFUNCTION(BlueprintCallable)
	FVector GetGridCellCenterRelativePosition(int x, int y);

	/** Produces coordinates in grid units based on world position. */
	UFUNCTION(BlueprintCallable) //maybe should be not exposed in BP
	FVector GetGridUnitsPositionFromWorldPosition(const FVector& worldPosition); //refactor to accept struct??

	UFUNCTION(BlueprintCallable)
	void DebugMeshSpawnOnGrid();

	UFUNCTION(BlueprintCallable)
	void CalculateWindDirectionInsideCell2D(FVector2D cellPosition);

	UFUNCTION(BlueprintCallable)
	FVector GetWindVectorAtLocation(FVector Location);

private:
	NoiseGen::Grid* grid;
	FCollisionQueryParams CollisionQueryParams;
	//NoiseGen::Noise* noise;
};
