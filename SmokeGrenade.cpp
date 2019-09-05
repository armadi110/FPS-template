#include "FPSTemplate.h"
#include "SmokeGrenade.h"

ASmokeGrenade::ASmokeGrenade(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitialLifeSpan = 60.f;
}

void ASmokeGrenade::BeginPlay()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_SpawnSmoke, this, &ASmokeGrenade::SpawnSmoke, 4.f);
}

void ASmokeGrenade::SpawnSmoke()
{
	UGameplayStatics::SpawnEmitterAtLocation(this, SmokeEmitter, GetActorLocation(), FRotator(0.f, 0.f, GetActorRotation().Yaw));
	UGameplayStatics::PlaySoundAtLocation(this, SmokeSound, GetActorLocation());
}