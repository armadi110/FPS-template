#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "SoldierMovementComponent.generated.h"

UCLASS()
class FPSTEMPLATE_API USoldierMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = Movement)
		float MaxSprintSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
		float FastSprintSpeed;

	USoldierMovementComponent();

	float GetMaxSpeed() const override;

	bool IsSprinting(float & OutSprintSpeed) const;
	
	
};
