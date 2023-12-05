// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "CoreMinimal.h"
#include "MassSpawnerTypes.h"
#include "GameFramework/SpectatorPawn.h"
#include "PlaceableSpawnerSpectatorPawn.generated.h"

UCLASS()
class UE5MASSTEST_API APlaceableSpawnerSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlaceableSpawnerSpectatorPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UMassProcessor> InitProcessor;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mass|Spawn")
	UMassEntityConfigAsset* EntityConfig;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mass|Spawn")
	int32 NumberToSpawn;

	UFUNCTION(BlueprintCallable, Category = "Mass|Spawn")
	void SpawnEntityFromEntityManager(FVector InLocation);

	UFUNCTION(BlueprintCallable, Category = "Mass|Spawn")
	void SpawnEntityFromEntitySpawnSubsystem (FVector InLocation);
};
