// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSignalProcessorBase.h"
#include "UObject/Object.h"
#include "ClickedActorSignalProcessor.generated.h"


namespace UE::Mass::Signals
{
	const FName Selected = FName(TEXT("SelectEntity"));
	const FName Deselected = FName(TEXT("DeselectEntity"));
}
/**
 * 
 */
UCLASS()
class UE5MASSTEST_API UClickedActorSignalProcessor : public UMassSignalProcessorBase
{
	GENERATED_BODY()
public:
	UClickedActorSignalProcessor();

protected:
	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	TArray<FMassEntityHandle> TransientEntitiesToSignal;
	FMassEntityQuery DeselectQuery;
};
