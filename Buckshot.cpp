#include "FPSTemplate.h"
#include "Buckshot.h"

ABuckshot::ABuckshot(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABuckshot::Tick(float DeltaTime)
{
	SphereCollision->SetRelativeScale3D(SphereCollision->GetComponentScale() + DeltaTime * SpreadRate);
}

float ABuckshot::CalcDamage(const FHitResult & HitResult, const FVector & ImpactVelocity) const
{
	// Distance between impact point and motion line of bullet
	const float Dist = FMath::PointDistToLine(HitResult.ImpactPoint, ImpactVelocity, GetActorLocation());
	const float SphereRadius = SphereCollision->GetScaledSphereRadius();
	// Ratio from 0-1 to apply damage
	const float DamageRatio = ((SphereRadius - Dist) / SphereRadius) * GetDefault<ABuckshot>(GetClass())->SphereCollision->GetScaledSphereRadius() / SphereRadius;

	return Super::CalcDamage(HitResult, ImpactVelocity) * DamageRatio;
}