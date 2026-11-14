// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityElementTypes.h"
#include "MassEntityQuery.h"
#include "MassEntityTraitBase.h"
#include "MassProcessor.h"
#include "MassLocomotionAnimation.generated.h"

/**
 * Locomotion animation for entities rendered as Instanced Skinned Meshes (EMassRepresentationType::SkinnedMeshInstance).
 *
 * The sequence indices refer to the order of the Sequences array in the UAnimSequenceTransformProviderData asset
 * assigned to the visualization trait's SkinnedMeshInstanceDesc (e.g. 0 = Idle, 1 = Walk, 2 = Run).
 * The engine's UMassConsumeInstancedSkinnedMeshAnimationProcessor forwards the result to the GPU-skinned instances.
 */
USTRUCT()
struct FMassLocomotionAnimationParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Sequences")
	int32 IdleSequenceIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Sequences")
	int32 WalkSequenceIndex = 1;

	/** Set to -1 to never use a run sequence. */
	UPROPERTY(EditAnywhere, Category = "Sequences")
	int32 RunSequenceIndex = 2;

	/** Speed (cm/s) at which the agent starts walking. */
	UPROPERTY(EditAnywhere, Category = "Thresholds", meta = (ClampMin = "0"))
	float WalkStartSpeed = 30.f;

	/** Speed (cm/s) at which the agent starts running. */
	UPROPERTY(EditAnywhere, Category = "Thresholds", meta = (ClampMin = "0"))
	float RunStartSpeed = 300.f;

	/** A state is left only once the speed drops this much below its start speed (prevents flickering). */
	UPROPERTY(EditAnywhere, Category = "Thresholds", meta = (ClampMin = "0"))
	float Hysteresis = 20.f;

	/** Ground speed (cm/s) the walk clip matches at play rate 1. 0 = always play at rate 1. */
	UPROPERTY(EditAnywhere, Category = "Play Rate", meta = (ClampMin = "0"))
	float WalkAnimSpeed = 150.f;

	/** Ground speed (cm/s) the run clip matches at play rate 1. 0 = always play at rate 1. */
	UPROPERTY(EditAnywhere, Category = "Play Rate", meta = (ClampMin = "0"))
	float RunAnimSpeed = 375.f;

	UPROPERTY(EditAnywhere, Category = "Play Rate", meta = (ClampMin = "0.01"))
	float MinPlayRate = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Play Rate", meta = (ClampMin = "0.01"))
	float MaxPlayRate = 2.f;

	/** Cross-fade duration (seconds) when switching sequences. */
	UPROPERTY(EditAnywhere, Category = "Blending", meta = (ClampMin = "0"))
	float BlendTime = 0.2f;
};

/**
 * Adds the animation fragment consumed by the engine's instanced skinned mesh animation processor, plus the
 * locomotion parameters. Pair with a visualization trait whose LODs use SkinnedMeshInstance.
 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "Locomotion Animation (Instanced Skinned Mesh)"))
class UE5MASSTEST_API UMassLocomotionAnimationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ShowOnlyInnerProperties))
	FMassLocomotionAnimationParams Params;
};

/** Picks idle / walk / run (and play rate) from FMassVelocityFragment and writes FMassRepresentationAnimationFragment. */
UCLASS()
class UE5MASSTEST_API UMassLocomotionAnimationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassLocomotionAnimationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
