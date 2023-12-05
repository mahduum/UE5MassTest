// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "MassSpawnerSubsystem.h"
#include "GameFramework/Actor.h"
#include "SEntitySpawnerActor.generated.h"

UCLASS()
class UE5MASSTEST_API ASEntitySpawnerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASEntitySpawnerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category= "Entity Spawn")
	void SpawnEntityFromSubsystem();

	UPROPERTY()
	UMassSpawnerSubsystem* MassSpawnerSubsystem;

	UPROPERTY(EditAnywhere, Category = "Entity Spawn")
	FMassEntityConfig EntityConfig;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
