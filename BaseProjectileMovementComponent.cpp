#include "FPSTemplate.h"
#include "BaseProjectileMovementComponent.h"

void UBaseProjectileMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// This function is slightly different than Epic's implementation, because it will also broadcast OnProjectileBounce when the projectile stops

	bool bStopSimulating = false;
	const FVector OldVelocity = Velocity;

	if (bShouldBounce)
	{
		Velocity = ComputeBounceResult(Hit, TimeSlice, MoveDelta);

		Velocity = LimitVelocity(Velocity);
		if (Velocity.SizeSquared() < FMath::Square(BounceVelocityStopSimulatingThreshold))
		{
			bStopSimulating = true;
		}
	}
	else
	{
		bStopSimulating = true;
	}

	OnProjectileBounce.Broadcast(Hit, OldVelocity);

	if (bStopSimulating)
	{
		StopSimulating(Hit);
	}
}