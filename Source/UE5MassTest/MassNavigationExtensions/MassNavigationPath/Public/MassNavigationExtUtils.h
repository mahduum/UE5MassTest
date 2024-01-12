// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTypes.h"

struct FMassNavigationShortPathRequest;
struct FMassNavigationPathLaneLocationFragment;
struct FMassNavigationShortPathFragment;
struct FMassNavigationCachedPathLaneFragment;
struct FMassMoveTargetFragment;
struct FMassNavigationShortPathRequestFragment;
struct FAgentRadiusFragment;

class UMassSignalSubsystem;
class UMassNavigationDataStorageSubsystem;

namespace MassNavigationExt::Actions
{
	UE5MASSTEST_API bool ActivateActionMove(const UWorld& World,
		const UObject* Requester,
		const FMassEntityHandle EntityHandle,
		const UMassNavigationDataStorageSubsystem& NavStorageSubsystem,
		const FMassNavigationPathLaneLocationFragment& PathLaneLocationFragment,
		const FMassNavigationShortPathRequest& PathRequest,
		const float AgentRadius,
		const float DesiredSpeed,
		FMassMoveTargetFragment& MoveTarget,
		FMassNavigationShortPathFragment& ShortPathFragment,
		FMassNavigationCachedPathLaneFragment& CachedPathLaneFragment
		);
}
