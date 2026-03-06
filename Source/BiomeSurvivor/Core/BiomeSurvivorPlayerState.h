// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BiomeSurvivorPlayerState.generated.h"

/**
 * ABiomeSurvivorPlayerState
 *
 * Replicated per-player data visible to all clients:
 * - Player name / display name
 * - Team/clan affiliation
 * - Skill levels
 * - Kill/death stats
 */
UCLASS(Blueprintable)
class BIOMESURVIVOR_API ABiomeSurvivorPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABiomeSurvivorPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Team System ----

	/** Team ID (-1 = no team). */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	int32 TeamID = -1;

	/** Set this player's team. Server only. */
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(int32 NewTeamID);

	/** Check if another player is on the same team. */
	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsSameTeam(const ABiomeSurvivorPlayerState* Other) const;

	// ---- Skill Levels (replicated for display) ----

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skills")
	int32 GatheringLevel = 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skills")
	int32 CraftingLevel = 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skills")
	int32 CombatLevel = 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skills")
	int32 SurvivalLevel = 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skills")
	int32 StealthLevel = 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skills")
	int32 BuildingLevel = 1;

	// ---- Stats ----

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 AnimalsKilled = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Deaths = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 DaysSurvived = 0;
};
