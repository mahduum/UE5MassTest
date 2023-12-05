// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once
#include "WindNoiseCalculator.h"

#include "NoiseFloatableActor.h"

void FWindNoiseCalculator::Refresh()//TODO!!! must move tdhe logic to skd!!! this should just pass the data there!!!
//TODO obstacle to overcome: sdk delegated function should receive all BP interface defined variables:
//Resolution, Offset, Dimensions, point00-11 should be calculated there and lerp also done there but will need quaternion and lerp functions
//all this variables can be passed as a struct
//the main problem is with Vertices and Normals because they are local Unreal containers, so maybe the function could return it's own array/vector of
//vertices and normals and those could be copied back to Unreal Vertices and Normals
{
	FQuat q = Rotation.ToOrientationQuat();
	const FQuat qInv = q.Inverse();
	//todo we are in a center of a 1 unit grid cell, need to rescale to fit other sizes?
	const FVector point00 = q * FVector{-0.5f, -0.5f, 0} + Offset;
	const FVector point10 = q * FVector{0.5f, -0.5f, 0} + Offset;
	const FVector point01 = q * FVector{-0.5f, 0.5f, 0} + Offset;
	const FVector point11 = q * FVector{0.5f, 0.5f, 0} + Offset;

	const float stepSize = 1.f / Resolution;
	//float amplitude = Damping ? Strength / Frequency : Strength;//why is it unused?
	for (uint32 v = 0, y = 0; y <= Resolution; y++)
	{
		//TODO must receive resolution to calculate!!!
		FVector point0 = FMath::Lerp(point00, point01, y * stepSize);
		FVector point1 = FMath::Lerp(point10, point11, y * stepSize);
		for (uint32 x = 0; x <= Resolution; x++, v++)
		{
			const FVector point = FMath::Lerp(point0, point1, x * stepSize);
			NoiseGen::NoiseSample sample = Core.Noise->Sum(static_cast<uint8>(NoiseType), Dimensions - 1, FVectorToVecN(point), Frequency,
			                                     Octaves, Lacunarity, Persistence);
			sample = sample * 0.5f;
			//sample = NoiseType == NoiseMethodType::Value ? sample - 0.5f : sample * 0.5f;
			Vertices[v].Y = sample.value;
			sample.derivative = FQuatToQuat(qInv).Rotate(sample.derivative);
			Normals[v] = FVector(-sample.derivative.X, -sample.derivative.Y, 1.f).GetSafeNormal();//it is a shortcut for Vector3.Cross(FVector(0f, 1f, GetZDerivative(x, y)),FVector(1f, 0f, GetZDerivative(x, y)));
		}
	}
}

FVector FWindNoiseCalculator::GetWindVectorForLocation(FVector&& position, const UObject* context)
{
	auto deltaTime = context->GetWorld()->DeltaTimeSeconds;
	const FQuat q = Rotation.ToOrientationQuat(); //Rotation is grid base plane rotation
	const FQuat qInv = q.Inverse();
	const float amplitude = Damping ? DampingStrength / Frequency : DampingStrength;

	const FVector point = /* q */ FVector(position.X, position.Y, position.Z) + Offset;
	NoiseGen::NoiseSample sample = Core.Noise->Sum(static_cast<uint8>(NoiseType), Dimensions, FVectorToVecN(point), Frequency, Octaves,
										Lacunarity, Persistence);
	sample = sample * 0.5f;
	//sample = NoiseType == NoiseMethodType::Value ? sample - 0.5f : sample * 0.5f;
	sample = sample * amplitude;
	sample.derivative = FQuatToQuat(qInv).Rotate(sample.derivative);

	const FVector curl = FVector(sample.derivative.Y, -sample.derivative.X, 0.f);
	return curl /* DeltaTime */* FlowStrength;
}
