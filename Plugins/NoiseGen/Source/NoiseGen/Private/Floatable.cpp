#include "Floatable.h"

// Add default functionality here for any IFloatable functions that are not pure virtual.

FVector IFloatable::GetLocation()
{
	if(const AActor* floatable = Cast<AActor>(this))
	{
		return floatable->GetActorLocation();
	}
	return {};
}

void IFloatable::SetLocation(const FVector& position)
{
	if(AActor* floatable = Cast<AActor>(this))
	{
		floatable->SetActorLocation(position);
	}
}

void IFloatable::LifeSpanExpired()
{
	if(AActor* floatable = Cast<AActor>(this))
	{
		floatable->LifeSpanExpired();
	}
}

bool IFloatable::IsPendingKillOrDestroyed()
{
	if(const UObject* floatable = Cast<UObject>(this))
	{
		return floatable->IsPendingKillOrUnreachable();
	}
	return true;
} 

void IFloatable::AddToGrid()
{
	unimplemented();
}

void IFloatable::RemoveFromGrid()
{
	unimplemented();
}

void IFloatable::UpdateTranslation(FVector (FWindNoiseCalculator::*NextPosition)(FVector&&, const UObject*), FWindNoiseCalculator& Calculator)
{
	unimplemented();
}
