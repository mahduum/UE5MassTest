#pragma once
#include "NoiseGen_UnrealCore.h"

FNoiseGen_UnrealCore::FNoiseGen_UnrealCore()
{
	Noise = new NoiseGen::Noise();
}

FNoiseGen_UnrealCore::~FNoiseGen_UnrealCore()
{
	//delete Noise;
}

void FNoiseGen_UnrealCore::Reset()
{
	// instantiate a Noise Generator
	if (!Noise)
	{
		Noise = new NoiseGen::Noise();
	}

	// reset data structures
	Noise->Reset();
	// also reset other data if necessary:
}

using NoiseMath::VecN;
using NoiseMath::Quat;

VecN FVectorToVecN(FVector V)
{
	return VecN(V.X, V.Y, V.Z);
}

FVector VecNToFVector(VecN V)
{
	return FVector(V.X, V.Y, V.Z);
}

Quat FQuatToQuat(FQuat Q)
{
	return Quat(Q.X, Q.Y, Q.Z, Q.W);
}

FQuat QuatToFQuat(Quat Q)
{
	return FQuat(Q.X, Q.Y, Q.Z, Q.W);
}
