#pragma once

#include "GameFramework/Actor.h"
#include "ThrowableProjectile.generated.h"

UCLASS()
class FPSTEMPLATE_API AThrowableProjectile : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
		UProjectileMovementComponent * ProjectileMovement;

	// Sphere component for collision
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
		USphereComponent * SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
		UStaticMeshComponent * Mesh;

	void PostInitializeComponents() override;

	void Tick(float DeltaTime) override;

	UFUNCTION()
		virtual void OnProjectileBounce(const FHitResult & HitResult, const FVector & ImpactVelocity);

	UFUNCTION()
		virtual void OnProjectileStop(const FHitResult & HitResult);

};
