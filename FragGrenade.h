#pragma once

#include "Projectiles/ThrowableProjectile.h"
#include "FragGrenade.generated.h"

UCLASS()
class FPSTEMPLATE_API AFragGrenade : public AThrowableProjectile
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = FragGrenade)
		UParticleSystem * ExplosionEmitter;

	UPROPERTY(EditDefaultsOnly, Category = FragGrenade)
		USoundCue * ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = FragGrenade)
		float DamageRadius;

	// Damage applied in the center of the explosion, fades to 0 within DamageRadius
	UPROPERTY(EditDefaultsOnly, Category = FragGrenade)
		float MaxDamage;

	AFragGrenade(const FObjectInitializer & ObjectInitializer);

	void Destroyed() override;
	
	
};
