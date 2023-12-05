// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AboveSpawnerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLUGINTEST_0206_2001_API UAboveSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAboveSpawnerComponent();

	UFUNCTION(BlueprintCallable)
	void SpawnActorAbove();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ActorClassToSpawn;
	
	// Called when the game starts
	virtual void BeginPlay() override;
};
