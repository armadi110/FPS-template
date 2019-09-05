#pragma once

#include "Projectiles/ThrowableProjectile.h"
#include "SmokeGrenade.generated.h"

UCLASS()
class FPSTEMPLATE_API ASmokeGrenade : public AThrowableProjectile
{
	GENERATED_BODY()

	FTimerHandle TimerHandle_SpawnSmoke;

	UPROPERTY(EditDefaultsOnly, Category = SmokeGrenade)
		UParticleSystem * SmokeEmitter;

	UPROPERTY(EditDefaultsOnly, Category = SmokeGrenade)
		USoundCue * SmokeSound;

	ASmokeGrenade(const FObjectInitializer & ObjectInitializer);
	
	void BeginPlay() override;

	void SpawnSmoke();
	
};
