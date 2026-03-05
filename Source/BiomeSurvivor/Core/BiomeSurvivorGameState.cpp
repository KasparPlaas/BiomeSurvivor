// Copyright Biome Survivor. All Rights Reserved.

#include "Core/BiomeSurvivorGameState.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"

ABiomeSurvivorGameState::ABiomeSurvivorGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // Tick every 100ms for time updates
}

void ABiomeSurvivorGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABiomeSurvivorGameState, GameTimeHours);
	DOREPLIFETIME(ABiomeSurvivorGameState, CurrentDay);
	DOREPLIFETIME(ABiomeSurvivorGameState, DayCycleDuration);
}

void ABiomeSurvivorGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		AdvanceGameClock(DeltaTime);
	}
}

void ABiomeSurvivorGameState::AdvanceGameClock(float DeltaTime)
{
	// Convert real-time delta to game-time hours
	// DayCycleDuration real seconds = 24 game hours
	const float HoursPerRealSecond = 24.0f / DayCycleDuration;
	GameTimeHours += DeltaTime * HoursPerRealSecond;

	// Roll over to next day
	if (GameTimeHours >= 24.0f)
	{
		GameTimeHours -= 24.0f;
		CurrentDay++;
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Day %d has begun."), CurrentDay);
	}
}

bool ABiomeSurvivorGameState::IsNight() const
{
	return (GameTimeHours >= 20.0f || GameTimeHours < 6.0f);
}

float ABiomeSurvivorGameState::GetSunProgress() const
{
	// Sunrise at 6:00, sunset at 18:00
	if (GameTimeHours < 6.0f || GameTimeHours > 18.0f)
	{
		return -1.0f; // Sun is below horizon
	}
	return (GameTimeHours - 6.0f) / 12.0f; // 0.0 = sunrise, 0.5 = noon, 1.0 = sunset
}

void ABiomeSurvivorGameState::MulticastServerMessage_Implementation(const FString& Message)
{
	UE_LOG(LogBiomeSurvivor, Log, TEXT("[Server Message] %s"), *Message);
	// TODO: Display message on all clients' HUD
}
