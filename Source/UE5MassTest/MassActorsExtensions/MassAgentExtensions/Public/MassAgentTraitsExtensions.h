// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassAgentTraits.h"
#include "MassAgentTraitsExtensions.generated.h"


namespace FMassAgentComponentsExtractor
{
	template<typename T>
	T* AsComponent(UObject& Owner)
	{
		T* Component = nullptr;
		if (const AActor* AsActor = Cast<AActor>(&Owner))
		{
			Component = AsActor->FindComponentByClass<T>();
		}
		else
		{
			Component = Cast<T>(&Owner);
		}

		UE_CVLOG_UELOG(Component == nullptr, &Owner, LogMass, Error, TEXT("Trying to extract %s from %s failed with FMassAgentComponentExtractor.")
			, *T::StaticClass()->GetName(), *Owner.GetName());

		return Component;
	}
}
/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "Agent All Movement Sync"))
class UE5MASSTEST_API UMassAgentAllMovementSyncTrait : public UMassAgentSyncTrait
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
