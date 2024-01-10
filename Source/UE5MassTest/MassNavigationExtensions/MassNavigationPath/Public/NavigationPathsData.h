// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassNavigationExtTypes.h"
#include "GameFramework/Actor.h"
#include "NavigationPathsData.generated.h"

UCLASS(config = MassNavigationPaths, defaultconfig, NotBlueprintable)
class UE5MASSTEST_API ANavigationPathsData : public AActor//can be placed from place actors menu
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANavigationPathsData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject/AActor Interface TODO
	virtual void PostActorCreated() override;
	virtual void PostLoad() override;
	virtual void Destroyed() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PreRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;

	FORCEINLINE bool IsRegistered() const { return bRegistered; }
	void OnRegistered(const FMassNavigationPathDataHandle DataHandle);
	void OnUnregistered();

	FMassNavigationDataStorage& GetStorageMutable() { return NavigationDataStorage; }
	const FMassNavigationDataStorage& GetStorage() const { return NavigationDataStorage; }
	FCriticalSection& GetStorageLock() const { return NavigationStorageLock; }

protected:
	bool RegisterWithSubsystem();
	bool UnregisterWithSubsystem();

	bool bRegistered;

	/** if set to true then this zone graph data will be drawing itself when requested as part of "show navigation" */
	// UPROPERTY(Transient, EditAnywhere, Category = Display)//todo for debugging
	// bool bEnableDrawing;
	//
	// UPROPERTY(transient, duplicatetransient)
	// TObjectPtr<UZoneGraphRenderingComponent> RenderingComp;

	UPROPERTY()
	FMassNavigationDataStorage NavigationDataStorage;

	/** Critical section to prevent rendering of the zone graph storage data while it's getting rebuilt */
	mutable FCriticalSection NavigationStorageLock;

	/** Combined hash of all ZoneShapes that were used to build the data. */
	UPROPERTY()
	uint32 CombinedShapeHash = 0;
};
