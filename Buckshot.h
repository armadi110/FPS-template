#pragma once

#include "Projectiles/Bullet.h"
#include "Buckshot.generated.h"

UCLASS()
class FPSTEMPLATE_API ABuckshot : public ABullet
{
	GENERATED_BODY()
	
public:
	// Rate at which this projectile grows, 0 means no change in size
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		float SpreadRate;

	ABuckshot(const FObjectInitializer & ObjectInitializer);

	virtual void Tick(float DeltaTime) override;
	
	virtual float CalcDamage(const FHitResult & HitResult, const FVector & ImpactVelocity) const;
	
};
