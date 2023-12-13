// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MassEntityTypes.h"
#include "MassComponentHitTypes.h"
#include "MassAgentSubsystem.h"
#include "MassSignalSubsystem.h"
#include "MassActorSubsystem.h"
#include "SelectedEntitiesProcessor.h"
#include "MassRaycastHitSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UE5MASSTEST_API UMassRaycastHitSubsystem : public UWorldSubsystem//rename to signal
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Mass Selection")
	void SingleLineTraceActorToEntity(APlayerController* PlayerController, float TraceLength, bool bDebugQuery);

	UFUNCTION(BlueprintCallable, Category="Mass Selection")
	void SingleLineTraceMoveTarget(APlayerController* PlayerController, float TraceLength, bool bDebugQuery);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void SingleTraceForSignal(FName SignalName, APlayerController* PlayerController, float TraceLength, bool bDebugQuery) const;
	bool TrySendSelectSignal(AActor* HitActor) const;
	void SetMoveTarget(const FHitResult& HitResult) const;

protected:
	UPROPERTY()
	TObjectPtr<UMassSignalSubsystem> SignalSubsystem;

	UPROPERTY()
	TObjectPtr<UMassAgentSubsystem> AgentSubsystem;

	UPROPERTY()
	TObjectPtr<UMassActorSubsystem> ActorSubsystem;

	UPROPERTY()
	TObjectPtr<USelectedEntitiesProcessor> MassSelectedProcessor;

	UPROPERTY()
	TMap<FMassEntityHandle, FMassHitResult> HitResults;//todo if we need to accumulate them later

	UFUNCTION()
	void ProcessSingleLineTraceResult(const FHitResult& HitResult, FName SignalName) const;
};
