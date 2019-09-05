#pragma once

#include "Projectiles/BaseProjectile.h"
#include "Bullet.generated.h"

USTRUCT()
struct FImpactEffectStruct
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Bullet)
		UParticleSystem * ParticleEmitter;

	UPROPERTY(EditDefaultsOnly, Category = Bullet)
		USoundCue * SoundCue;

	UPROPERTY(EditDefaultsOnly, Category = Bullet)
		UMaterialInterface * DecalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = Bullet)
		float DecalSize;

	FImpactEffectStruct()
	{
		ParticleEmitter = NULL;
		SoundCue = NULL;
		DecalMaterial = NULL;
		DecalSize = 5.f;
	}
};

UCLASS()
class FPSTEMPLATE_API ABullet : public ABaseProjectile
{
	GENERATED_BODY()

public:
	// Impact effects
	UPROPERTY(EditDefaultsOnly, Category = ImpactEffects)
		FImpactEffectStruct MetalEffect;

	UPROPERTY(EditDefaultsOnly, Category = ImpactEffects)
		FImpactEffectStruct StoneEffect;

	UPROPERTY(EditDefaultsOnly, Category = ImpactEffects)
		FImpactEffectStruct WoodEffect;

	UPROPERTY(EditDefaultsOnly, Category = ImpactEffects)
		FImpactEffectStruct PlasticEffect;

	UPROPERTY(EditDefaultsOnly, Category = ImpactEffects)
		FImpactEffectStruct DirtEffect;

	UPROPERTY(EditDefaultsOnly, Category = ImpactEffects)
		FImpactEffectStruct WaterEffect;

	UPROPERTY(EditDefaultsOnly, Category = ImpactEffects)
		FImpactEffectStruct FleshEffect;

	// Maxi damage this bullet will cause
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		float MaxDamage;

	// Max damage this bullet will cause against an armor physics material (used for soldier body armor)
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		float MaxArmorDamage;

	ABullet(const FObjectInitializer & ObjectInitializer);
	
	void OnConstruction(const FTransform & Transform) override;

	void OnProjectileHit(const FHitResult & HitResult, const FVector & ImpactVelocity) override;

	void SpawnImpactEffect(const FHitResult & HitResult) override;

	virtual float CalcDamage(const FHitResult & HitResult, const FVector & ImpactVelocity) const;
	
};
