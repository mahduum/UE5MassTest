// Fill out your copyright notice in the Description page of Project Settings.

#include "MassLocomotionAnimation.h"

#include "MassCommonTypes.h"
#include "MassEntityManager.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"

//----------------------------------------------------------------------//
//  UMassLocomotionAnimationTrait
//----------------------------------------------------------------------//
void UMassLocomotionAnimationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FMassVelocityFragment>();

	FMassRepresentationAnimationFragment& AnimFragment = BuildContext.AddFragment_GetRef<FMassRepresentationAnimationFragment>();
	AnimFragment.AnimData.SequenceIndex = Params.IdleSequenceIndex;
	AnimFragment.AnimData.PlayRate = 1.f;
	AnimFragment.AnimData.BlendTime = 0.f;

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(Params));
}

//----------------------------------------------------------------------//
//  UMassLocomotionAnimationProcessor
//----------------------------------------------------------------------//
namespace UE::Mass::LocomotionAnimation
{
	enum class EState : uint8 { Idle, Walk, Run };

	EState GetCurrentState(const FMassLocomotionAnimationParams& Params, const int32 SequenceIndex)
	{
		if (Params.RunSequenceIndex != INDEX_NONE && SequenceIndex == Params.RunSequenceIndex)
		{
			return EState::Run;
		}
		return SequenceIndex == Params.WalkSequenceIndex ? EState::Walk : EState::Idle;
	}

	EState GetDesiredState(const FMassLocomotionAnimationParams& Params, const EState Current, const float Speed)
	{
		const bool bCanRun = Params.RunSequenceIndex != INDEX_NONE;
		if (bCanRun && (Speed >= Params.RunStartSpeed || (Current == EState::Run && Speed >= Params.RunStartSpeed - Params.Hysteresis)))
		{
			return EState::Run;
		}
		const bool bWasMoving = Current != EState::Idle;
		if (Speed >= Params.WalkStartSpeed || (bWasMoving && Speed >= FMath::Max(0.f, Params.WalkStartSpeed - Params.Hysteresis)))
		{
			return EState::Walk;
		}
		return EState::Idle;
	}

	float GetPlayRate(const FMassLocomotionAnimationParams& Params, const EState State, const float Speed)
	{
		const float ReferenceSpeed = State == EState::Run ? Params.RunAnimSpeed : (State == EState::Walk ? Params.WalkAnimSpeed : 0.f);
		if (ReferenceSpeed <= UE_KINDA_SMALL_NUMBER)
		{
			return 1.f;
		}
		return FMath::Clamp(Speed / ReferenceSpeed, Params.MinPlayRate, FMath::Max(Params.MinPlayRate, Params.MaxPlayRate));
	}

	int32 GetSequenceIndex(const FMassLocomotionAnimationParams& Params, const EState State)
	{
		switch (State)
		{
		case EState::Run:  return Params.RunSequenceIndex;
		case EState::Walk: return Params.WalkSequenceIndex;
		default:           return Params.IdleSequenceIndex;
		}
	}
}

UMassLocomotionAnimationProcessor::UMassLocomotionAnimationProcessor()
	: EntityQuery(*this)
{
	// Purely visual - no need to run on dedicated servers.
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Editor);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Representation);
	bRequiresGameThreadExecution = false;
}

void UMassLocomotionAnimationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationAnimationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMassLocomotionAnimationParams>();
}

void UMassLocomotionAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::Mass::LocomotionAnimation;

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const FMassLocomotionAnimationParams& Params = Context.GetConstSharedFragment<FMassLocomotionAnimationParams>();
		const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FMassRepresentationAnimationFragment> AnimList = Context.GetMutableFragmentView<FMassRepresentationAnimationFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FAnimSequenceTrackAutoPlayData& AnimData = AnimList[EntityIt].AnimData;
			const float Speed = static_cast<float>(VelocityList[EntityIt].Value.Size2D());

			const EState Current = GetCurrentState(Params, AnimData.SequenceIndex);
			const EState Desired = GetDesiredState(Params, Current, Speed);
			if (Desired != Current)
			{
				// The instanced skinned mesh only picks up new auto-play data when the sequence index changes,
				// so play rate is decided at the moment of the switch.
				AnimData.SequenceIndex = GetSequenceIndex(Params, Desired);
				AnimData.PlayRate = GetPlayRate(Params, Desired, Speed);
				AnimData.BlendTime = Params.BlendTime;
			}
		}
	});
}
