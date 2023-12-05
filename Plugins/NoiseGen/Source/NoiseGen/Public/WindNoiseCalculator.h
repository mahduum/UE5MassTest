// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "NoiseGen_UnrealCore.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "WindNoiseCalculator.generated.h"

UENUM()
enum class NoiseMethodType : uint8
{
	Value UMETA(DisplayName = "Value Noise"),
	Perlin UMETA(DisplayName = "Perlin Noise"),
	SimplexValue UMETA(DisplayName = "Simplex Value Noise"),
	Simplex UMETA(DisplayName = "Simplex Noise")
};

USTRUCT(BlueprintType)//TODO class must be created that will be blueprint and will have this struct as a property!!!
struct NOISEGEN_API FWindNoiseCalculator//todo rename to wind calculator 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NoiseGen")
	float Frequency = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NoiseGen", meta=(ClampMin = "1", ClampMax = "8"))
	uint8 Octaves = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NoiseGen", meta=(ClampMin = "1.", ClampMax = "4."))
	float Lacunarity = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NoiseGen", meta=(ClampMin = "0.", ClampMax = "1."))
	float Persistence = .5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NoiseGen", meta=(ClampMin = "1", ClampMax = "3"))
	uint8 Dimensions = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NoiseGen")
	float FlowStrength = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NoiseGen", meta=(ClampMin = "0", ClampMax = "1"))
	float DampingStrength = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NoiseGen")
	NoiseMethodType NoiseType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NoiseGen")
	FVector Offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NoiseGen")
	FVector Rotation;//todo consider quaternions?

	void Refresh();

	FVector GetWindVectorForLocation(FVector&& position, const UObject* context);
	
private:

	UPROPERTY(EditAnywhere, Category="NoiseGen")
	bool Damping = true;
	
	//UPROPERTY()
	TArray<FVector> Vertices;
	
	//UPROPERTY()
	TArray<FVector> Normals;

	//UPROPERTY()
	TArray<FColor> Colors;

	uint32 Resolution = 1;//todo get from grid

	uint32 CurrentResolution = 1;//to refresh checking?

	//todo initialize noise
	FNoiseGen_UnrealCore Core;
	
	//ref to grid manager or back

	//grid manager needs to query noise gen to acquire data for the cells

	//instead of creating grid by itself wind calculator will receive grid data from grid manager

	//noise type
	//
	//advanced:
	//two choices noise itself is being offset or grid itself with its data is moving (then it would have to have its own coord space)

	//noise shift rate
};
