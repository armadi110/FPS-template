#pragma once

#include "GameFramework/ProjectileMovementComponent.h"
#include "BaseProjectileMovementComponent.generated.h"

UCLASS()
class FPSTEMPLATE_API UBaseProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
	
public:
	void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
	
};
