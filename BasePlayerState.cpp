#include "FPSTemplate.h"
#include "BasePlayerState.h"
#include "UnrealNetwork.h"

void ABasePlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerState, Kills);
	DOREPLIFETIME(ABasePlayerState, Deaths);
	DOREPLIFETIME(ABasePlayerState, Team);
}

bool ABasePlayerState::IsEnemyFor(ABasePlayerState * OtherPlayer) const
{
	if (OtherPlayer && OtherPlayer != this)
	{
		return Team != OtherPlayer->GetTeam() || Team == ETeamEnum::None;
	}

	return true;
}

void ABasePlayerState::SetTeam(ETeamEnum NewTeam)
{
	Team = NewTeam;
}