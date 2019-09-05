#include "FPSTemplate.h"
#include "BaseProjectile.h"


ABaseProjectile::ABaseProjectile(const FObjectInitializer & ObjectInitializer)
{
	ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UBaseProjectileMovementComponent>(this, TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereCollision;
	ProjectileMovement->bRotationFollowsVelocity = true;

	SphereCollision = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereCollision"));
	RootComponent = SphereCollision;
	SphereCollision->SetCollisionProfileName(FName("Projectile"));
	SphereCollision->bReturnMaterialOnMove = true;
	SphereCollision->CanCharacterStepUpOn = ECB_No;

	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	Mesh->AttachParent = SphereCollision;
	Mesh->SetCollisionProfileName(FName("NoCollision"));

	InitialLifeSpan = 2.f;
}

void ABaseProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &ABaseProjectile::OnProjectileHit);
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ABaseProjectile::OnProjectileStop);

	SphereCollision->IgnoreActorWhenMoving(Instigator, true);
}

void ABaseProjectile::OnProjectileStop(const FHitResult& HitResult)
{
	Destroy();
}

void ABaseProjectile::OnProjectileHit(const FHitResult & HitResult, const FVector & ImpactVelocity)
{
	
}

void ABaseProjectile::SpawnImpactEffect(const FHitResult & HitResult)
{

}