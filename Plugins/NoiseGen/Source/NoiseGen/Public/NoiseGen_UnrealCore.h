#pragma once

#include "CoreMinimal.h"

#include "../sdk/include/Noise.h"

#include "NoiseGen_UnrealCore.generated.h"

// conversion helper for NoiseMath's internal data types
NoiseMath::VecN FVectorToVecN(FVector V);
FVector VecNToFVector(NoiseMath::VecN V);
NoiseMath::Quat FQuatToQuat(FQuat Q);
FQuat QuatToFQuat(NoiseMath::Quat Q);

USTRUCT(BlueprintInternalUseOnly)
struct NOISEGEN_API FNoiseGen_UnrealCore
{
	GENERATED_BODY()

	FNoiseGen_UnrealCore();
	~FNoiseGen_UnrealCore();

	void Reset();
	
	NoiseGen::Noise* Noise = nullptr;
	bool IsInitialized = false;

	//add other functions like reset etc.
	//todo put interface implementation here...
};
