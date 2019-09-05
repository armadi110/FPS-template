#include "FPSTemplate.h"
#include "SoldierMovementComponent.h"
#include "Soldier/Soldier.h"

USoldierMovementComponent::USoldierMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanJump = true;
	MaxWalkSpeed = 400.f;
	MaxWalkSpeedCrouched = 200.f;
	MaxSprintSpeed = 600.f;
	FastSprintSpeed = 800.f;
	MaxAcceleration = 2048;
	BrakingDecelerationWalking = 128.f;
	JumpZVelocity = 450.f;
	CrouchedHalfHeight = 70.f;
	bCanWalkOffLedgesWhenCrouching = true;
}

float USoldierMovementComponent::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		// Overriden to be able to consider sprinting
		float SprintSpeed;
		return IsCrouching() ? MaxWalkSpeedCrouched : IsSprinting(SprintSpeed) ? SprintSpeed : MaxWalkSpeed;
	case MOVE_Falling:
		return MaxWalkSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	case MOVE_None:
	default:
		return 0.f;
	}
}

bool USoldierMovementComponent::IsSprinting(float & OutSprintSpeed) const
{
	ASoldier * Soldier = Cast<ASoldier>(CharacterOwner);
	if (Soldier)
	{
		if (Soldier->bSprinting && !Soldier->bAiming && !Soldier->bFiring)
		{
			OutSprintSpeed = Soldier->HasPerk(EPerkEnum::FastSprint) ? FastSprintSpeed : MaxSprintSpeed;
			return true;
		}
	}

	return false;
}