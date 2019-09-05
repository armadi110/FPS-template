#include "FPSTemplate.h"
#include "ThrowableProjectile.h"


AThrowableProjectile::AThrowableProjectile(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	SphereComponent = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->SetSphereRadius(3.5f);
	SphereComponent->SetCollisionProfileName(FName(TEXT("Projectile")));

	Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	Mesh->AttachParent = SphereComponent;
	Mesh->SetCollisionProfileName(FName(TEXT("NoCollision")));

	ProjectileMovement = PCIP.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.2f;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 100.f;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1750.f;
	ProjectileMovement->ProjectileGravityScale = 0.7f;

	InitialLifeSpan = 2.f;
	// Throwable projectiles are only spawned on server and replicated (in contrast to bullets)
	bReplicates = true;
	bReplicateMovement = true;

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AThrowableProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &AThrowableProjectile::OnProjectileBounce);
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AThrowableProjectile::OnProjectileStop);
}

void AThrowableProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Mesh->AddRelativeRotation(FRotator(50.f, 50.f, -50.f) * DeltaTime);
}

void AThrowableProjectile::OnProjectileBounce(const FHitResult & HitResult, const FVector & ImpactVelocity)
{
	if (HitResult.GetActor())
	{
		// Apply impact damage when projectile hits something
		FPointDamageEvent DamageEvent;
		DamageEvent.HitInfo = HitResult;
		DamageEvent.Damage = 5.f;
		DamageEvent.ShotDirection = ImpactVelocity;
		HitResult.Actor->TakeDamage(5.f, DamageEvent, GetInstigatorController(), this);
	}
}

void AThrowableProjectile::OnProjectileStop(const FHitResult & HitResult)
{
	SetActorTickEnabled(false);
}