// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BiomeSurvivorGameState.generated.h"

/**
 * ABiomeSurvivorGameState
 *
 * Replicated game state visible to all clients. Manages:
 * - Current time of day (game-time clock)
 * - Current weather state per biome
 * - Server-wide events and announcements
 * - Global resource respawn timers
 */
UCLASS(Blueprintable)
class BIOMESURVIVOR_API ABiomeSurvivorGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ABiomeSurvivorGameState();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Time of Day ----

	/** Current game time in hours (0.0 = midnight, 12.0 = noon, 23.99 = end of day). */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	float GameTimeHours = 8.0f;

	/** Current day number (starts at 1). */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	int32 CurrentDay = 1;

	/** Real-time seconds for a full in-game day. Set by GameMode. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	float DayCycleDuration = 1440.0f;

	/** Is it currently night time? (between 20:00 and 06:00 game-time) */
	UFUNCTION(BlueprintPure, Category = "Time")
	bool IsNight() const;

	/** Get normalized sun position (0.0 = sunrise, 0.5 = noon, 1.0 = sunset). */
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetSunProgress() const;

	// ---- Weather ----

	/** Server-broadcast message to all players. */
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Server")
	void MulticastServerMessage(const FString& Message);

protected:
	/** Advance game clock each tick. Server only. */
	void AdvanceGameClock(float DeltaTime);
};
