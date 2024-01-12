// Fill out your copyright notice in the Description page of Project Settings.


#include "MassNavigationExtUtils.h"

#include "MassNavigationDataStorageSubsystem.h"
#include "MassNavigationExtFragments.h"
#include "MassNavigationFragments.h"

namespace MassNavigationExt::Actions
{
	static constexpr float InflateDistancePathRequest = 200.0f;
	
	bool ActivateActionMove
		(
		const UWorld& World, const UObject* Requester,
		const FMassEntityHandle EntityHandle, const UMassNavigationDataStorageSubsystem& NavStorageSubsystem,
		const FMassNavigationPathLaneLocationFragment& PathLaneLocationFragment,
		const FMassNavigationShortPathRequest& PathRequest, const float AgentRadius, const float DesiredSpeed,
		FMassMoveTargetFragment& MoveTarget, FMassNavigationShortPathFragment& ShortPathFragment,
		FMassNavigationCachedPathLaneFragment& CachedPathLaneFragment
		)
	{
		ShortPathFragment.Reset();
		//CachedPathLaneFragment.Reset();
		MoveTarget.DistanceToGoal = 0.0f;
		MoveTarget.DesiredSpeed.Set(0.0f);

		if (!ensureMsgf(MoveTarget.GetCurrentAction() == EMassMovementAction::Move, TEXT("Expecting action 'Move': Invalid action %s"), MoveTarget.GetCurrentAction()))
		{
			return false;
		}

		const FMassNavigationDataStorage* NavStorage = NavStorageSubsystem.GetMassNavigationDataStorage(PathLaneLocationFragment.PathHandle.DataHandle);

		if (NavStorage == nullptr)
		{
			UE_VLOG(Requester, LogMassNavigationExt, Error, TEXT("Entity [%s] move request failed: missing ZoneGraph Storage for current lane %s."),
				*EntityHandle.DebugGetDescription(),
				*PathLaneLocationFragment.PathHandle.ToString());
			return false;
		}

		MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
		MoveTarget.DesiredSpeed.Set(DesiredSpeed);

		CachedPathLaneFragment.CachePathLaneData(*NavStorage, PathLaneLocationFragment.PathHandle, PathLaneLocationFragment.DistanceAlongPathLane, PathRequest.TargetDistance, InflateDistancePathRequest);
		if(ShortPathFragment.RequestPath(CachedPathLaneFragment, PathRequest, PathLaneLocationFragment.DistanceAlongPathLane, AgentRadius))
		{
			MoveTarget.IntentAtGoal = ShortPathFragment.EndOfPathIntent;
			MoveTarget.DistanceToGoal = (ShortPathFragment.NumPoints > 0) ? ShortPathFragment.Points[ShortPathFragment.NumPoints - 1].DistanceAlongLane.Get() : 0.0f;
		}
		else
		{
			UE_VLOG(Requester, LogMassNavigationExt, Error, TEXT("Entity [%s] move request failed: unable to request path on lane %s."),
				*EntityHandle.DebugGetDescription(),
				*PathLaneLocationFragment.PathHandle.ToString());
			return false;
		}

		UE_VLOG(Requester, LogMassNavigationExt, Log, TEXT("Entity [%s] successfully requested %s"), *EntityHandle.DebugGetDescription(), *MoveTarget.ToString());
		return true;
	}
}

