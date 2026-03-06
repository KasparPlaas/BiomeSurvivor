// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BiomeSurvivorGameMode.generated.h"

/**
 * ABiomeSurvivorGameMode
 *
 * The core game mode controlling match rules, player spawning, and server-side game logic.
 * Runs on the server only. Handles:
 * - Player spawning and respawning
 * - Day/night cycle ownership
 * - Weather system ownership
 * - PvE vs PvPvE mode switching
 * - Server admin commands
 */
UENUM(BlueprintType)
enum class EServerMode : uint8
{
	PvE       UMETA(DisplayName = "PvE - Cooperative"),
	PvPvE     UMETA(DisplayName = "PvPvE - Full PvP")
};

UCLASS(Blueprintable)
class BIOMESURVIVOR_API ABiomeSurvivorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABiomeSurvivorGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** Respawn a player after death with a configurable delay. */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void RespawnPlayer(APlayerController* PlayerController, float DelaySeconds = 5.0f);

	/** Kick a player by controller reference. Server-only. */
	UFUNCTION(BlueprintCallable, Category = "Admin")
	void KickPlayer(APlayerController* PlayerController, const FString& Reason);

	/** Current server PvP mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Settings")
	EServerMode ServerMode = EServerMode::PvE;

	/** Maximum players allowed on this server. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Settings", meta = (ClampMin = "1", ClampMax = "32"))
	int32 MaxPlayers = 16;

	/** Time in real seconds for one full in-game day (default: 24 min = 1440 seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Settings", meta = (ClampMin = "60"))
	float DayCycleDurationSeconds = 1440.0f;

protected:
	/** Handle delayed respawn via timer. */
	void HandleRespawn(APlayerController* PlayerController);

	/** Spawn essential world actors (DayNightCycle, WeatherSystem) if not already placed. */
	void SpawnEssentialActors();

	/** Spawn resource nodes (trees, rocks, bushes) throughout the world. */
	void SpawnWorldResources();

	/** Spawn wildlife (deer, wolves, rabbits). */
	void SpawnWildlife();

	/** Timer handles for pending respawns. */
	TMap<APlayerController*, FTimerHandle> PendingRespawns;
};
