// Fill out your copyright notice in the Description page of Project Settings.


#include "MassPathQuery.h"


namespace MassNavigationExt::MassNavigationPath::Query
{
	static bool EnsureLaneHandle(const FMassNavigationDataStorage& Storage, const FMassNavigationPathLaneHandle PathLaneHandle, const char* Function)
	{
		if (PathLaneHandle.DataHandle != Storage.DataHandle || PathLaneHandle.Index >= Storage.PathLanes.Num())
		{
			ensureMsgf(false, TEXT("%s: Bad lane handle: index = %d data = %d/%d), expected: index < %d data = %d/%d)"), ANSI_TO_TCHAR(Function),
				PathLaneHandle.Index, PathLaneHandle.DataHandle.Index, PathLaneHandle.DataHandle.Generation, Storage.PathLanes.Num(), Storage.DataHandle.Index, Storage.DataHandle.Generation);
			return false;
		}
		return true;
	}
	
	bool CalculateLaneSegmentIndexAtDistance(const FMassNavigationDataStorage& Storage, const FMassNavigationPathLaneHandle PathLaneHandle, const float Distance, int32& OutSegmentIndex)
	{
		if (!EnsureLaneHandle(Storage, PathLaneHandle, __FUNCTION__))
		{
			return false;
		}
		return CalculateLaneSegmentIndexAtDistance(Storage, PathLaneHandle.Index, Distance, OutSegmentIndex);
	}

	bool CalculateLaneSegmentIndexAtDistance(const FMassNavigationDataStorage& Storage, const uint32 PathLaneIndex, const float Distance, int32& OutSegmentIndex)
	{
		const FPathLaneData& Lane = Storage.PathLanes[PathLaneIndex];
		const int32 NumLanePoints = Lane.PointsEnd - Lane.PointsBegin;
		check(NumLanePoints >= 2);

		// Two or more points from here on.
		if (Distance <= Storage.PathLanePointsProgressions[Lane.PointsBegin])
		{
			// Handle out of range, before.
			OutSegmentIndex = Lane.PointsBegin;
		}
		else if (Distance >= Storage.PathLanePointsProgressions[Lane.PointsEnd - 1])
		{
			// Handle out of range, after.
			OutSegmentIndex = Lane.PointsEnd - 2;
		}
		else
		{
			// Binary search correct segment.
			TArrayView<const float> LanePointProgressionsView(&Storage.PathLanePointsProgressions[Lane.PointsBegin], NumLanePoints);
			// We want SegStart/End to be consecutive indices withing the range of the lane points.
			// Upper bound finds the point past the distance, so we subtract 1, and then clamp it to fit valid range.
			OutSegmentIndex = FMath::Clamp(Algo::UpperBound(LanePointProgressionsView, Distance) - 1, 0, NumLanePoints - 2) + Lane.PointsBegin;
		}

		return true;
	}

	bool GetLaneLength(const FMassNavigationDataStorage& Storage, const uint32 LaneIndex, float& OutLength)
	{
		const FPathLaneData& Lane = Storage.PathLanes[LaneIndex];
		OutLength = Storage.PathLanePointsProgressions[Lane.PointsEnd - 1];
		return true;
	}

	bool GetLaneLength(const FMassNavigationDataStorage& Storage, const FMassNavigationPathLaneHandle LaneHandle, float& OutLength)
	{
		if (!EnsureLaneHandle(Storage, LaneHandle, __FUNCTION__))
		{
			return false;
		}
		return GetLaneLength(Storage, LaneHandle.Index, OutLength);
	}
}
