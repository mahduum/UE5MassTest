// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Floatable.h"
#include "WindNoiseGrid.h"

#include "NoiseFloatableActor.generated.h"

UCLASS()
//todo separate actors, to simpler grid adctor and floatable actor
class NOISEGEN_API ANoiseFloatableActor : public AActor, public IFloatable//floatable should be part of grid, and it should incase the class it is carrying, and this class would need to have necessary data
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ANoiseFloatableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	TWeakObjectPtr<AWindNoiseGrid> WindNoiseGrid;

	virtual void AddToGrid() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//UFUNCTION(BlueprintCallable, Category="Nosie Grid")
	virtual void Initialize(TWeakObjectPtr<AWindNoiseGrid> NoiseGrid) override;
	
	virtual void UpdateTranslation(FVector (FWindNoiseCalculator::*NextPosition)(FVector&&, const UObject*), FWindNoiseCalculator& Calculator) override;
};
