#pragma once

#include "Projectiles/BaseProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

UCLASS()
class FPSTEMPLATE_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		UBaseProjectileMovementComponent * ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		USphereComponent * SphereCollision;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		UStaticMeshComponent * Mesh;

	// Cartidge compatibility
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		FName CompatibilityTag;

	ABaseProjectile(const FObjectInitializer & ObjectInitializer);

	void PostInitializeComponents() override;

	UFUNCTION()
		virtual void OnProjectileStop(const FHitResult & HitResult);

	UFUNCTION()
		virtual void OnProjectileHit(const FHitResult & HitResult, const FVector & ImpactVelocity);

	virtual void SpawnImpactEffect(const FHitResult & HitResult);
	
};
