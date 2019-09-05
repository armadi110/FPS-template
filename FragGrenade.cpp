#include "FPSTemplate.h"
#include "FragGrenade.h"
#include "Damage/DamageType_Explosion.h"

AFragGrenade::AFragGrenade(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitialLifeSpan = 2.f;
	MaxDamage = 150.f;
	DamageRadius = 500.f;
}

void AFragGrenade::Destroyed()
{
	Super::Destroyed();

	UGameplayStatics::ApplyRadialDamageWithFalloff(this, MaxDamage, 0.f, GetActorLocation(), 0.f, DamageRadius, 1.f, UDamageType_Explosion::StaticClass(), TArray<AActor *>(), this, GetInstigatorController(), ECC_MAX);

	if (GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEmitter, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}
}