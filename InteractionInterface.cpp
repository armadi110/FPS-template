#include "FPSTemplate.h"
#include "InteractionInterface.h"
#include "Soldier/Soldier.h"

bool IInteractionInterface::CanSoldierInteract(ASoldier * Soldier) const
{
	return true;
}

float IInteractionInterface::GetInteractionLength() const
{
	return 0.f;
}

FVector IInteractionInterface::GetInteractionLocation() const
{
	return FVector::ZeroVector;
}

UAnimMontage * IInteractionInterface::GetInteractionMontage(ASoldier * Soldier) const
{
	return NULL;
}

void IInteractionInterface::OnInteract(ASoldier * Soldier)
{
	
}