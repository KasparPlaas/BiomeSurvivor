// Copyright Biome Survivor. All Rights Reserved.

#include "Core/BiomeSurvivorPlayerState.h"
#include "Net/UnrealNetwork.h"

ABiomeSurvivorPlayerState::ABiomeSurvivorPlayerState()
{
}

void ABiomeSurvivorPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABiomeSurvivorPlayerState, TeamID);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, GatheringLevel);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, CraftingLevel);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, CombatLevel);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, SurvivalLevel);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, StealthLevel);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, BuildingLevel);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, AnimalsKilled);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, Deaths);
	DOREPLIFETIME(ABiomeSurvivorPlayerState, DaysSurvived);
}

void ABiomeSurvivorPlayerState::SetTeam(int32 NewTeamID)
{
	if (HasAuthority())
	{
		TeamID = NewTeamID;
	}
}

bool ABiomeSurvivorPlayerState::IsSameTeam(const ABiomeSurvivorPlayerState* Other) const
{
	if (!Other || TeamID < 0 || Other->TeamID < 0) return false;
	return TeamID == Other->TeamID;
}
