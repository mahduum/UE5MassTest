// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassNavigationExtTypes.h"

/**
 * 
 */
namespace MassNavigationExt::MassNavigationPath::Query
{
	static bool EnsureLaneHandle(const FMassNavigationDataStorage& Storage, const FMassNavigationPathLaneHandle PathLaneHandle, const char* Function);
	bool CalculateLaneSegmentIndexAtDistance(const FMassNavigationDataStorage& Storage, const FMassNavigationPathLaneHandle PathLaneHandle, const float Distance, int32& OutSegmentIndex);
	bool CalculateLaneSegmentIndexAtDistance(const FMassNavigationDataStorage& Storage, const uint32 PathLaneIndex, const float Distance, int32& OutSegmentIndex);
	bool GetLaneLength(const FMassNavigationDataStorage& Storage, const uint32 LaneIndex, float& OutLength);
	bool GetLaneLength(const FMassNavigationDataStorage& Storage, const FMassNavigationPathLaneHandle LaneHandle, float& OutLength);
}
