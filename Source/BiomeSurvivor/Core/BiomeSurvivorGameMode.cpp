// Copyright Biome Survivor. All Rights Reserved.

#include "Core/BiomeSurvivorGameMode.h"
#include "Core/BiomeSurvivorGameState.h"
#include "Core/BiomeSurvivorPlayerController.h"
#include "Core/BiomeSurvivorPlayerState.h"
#include "Player/SurvivorCharacter.h"
#include "UI/SurvivorHUD.h"
#include "World/DayNightCycle.h"
#include "World/WeatherSystem.h"
#include "GameFramework/GameSession.h"
#include "BiomeSurvivor.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ABiomeSurvivorGameMode::ABiomeSurvivorGameMode()
{
	// Set default classes
	DefaultPawnClass = ASurvivorCharacter::StaticClass();
	PlayerControllerClass = ABiomeSurvivorPlayerController::StaticClass();
	GameStateClass = ABiomeSurvivorGameState::StaticClass();
	PlayerStateClass = ABiomeSurvivorPlayerState::StaticClass();
	HUDClass = ASurvivorHUD::StaticClass();

	// Allow seamless travel for multiplayer
	bUseSeamlessTravel = true;
}

void ABiomeSurvivorGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Spawn essential world actors if not already placed in the level
	SpawnEssentialActors();

	// Parse server mode from URL options (e.g., ?ServerMode=PvPvE)
	FString ModeOption = UGameplayStatics::ParseOption(Options, TEXT("ServerMode"));
	if (ModeOption.Equals(TEXT("PvPvE"), ESearchCase::IgnoreCase))
	{
		ServerMode = EServerMode::PvPvE;
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Game initialized. Map: %s | Mode: %s | MaxPlayers: %d"),
		*MapName,
		ServerMode == EServerMode::PvE ? TEXT("PvE") : TEXT("PvPvE"),
		MaxPlayers);
}

void ABiomeSurvivorGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Player logged in: %s"), *NewPlayer->GetName());
	}
}

void ABiomeSurvivorGameMode::Logout(AController* Exiting)
{
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		// Clear any pending respawn timers
		if (FTimerHandle* Handle = PendingRespawns.Find(PC))
		{
			GetWorldTimerManager().ClearTimer(*Handle);
			PendingRespawns.Remove(PC);
		}
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Player logged out: %s"), *PC->GetName());
	}

	Super::Logout(Exiting);
}

AActor* ABiomeSurvivorGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// TODO: Prioritize bed/sleeping bag spawn points per player
	// For now, use default player start selection
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ABiomeSurvivorGameMode::RespawnPlayer(APlayerController* PlayerController, float DelaySeconds)
{
	if (!PlayerController || !HasAuthority()) return;

	if (DelaySeconds <= 0.0f)
	{
		HandleRespawn(PlayerController);
		return;
	}

	FTimerHandle& TimerHandle = PendingRespawns.FindOrAdd(PlayerController);

	GetWorldTimerManager().SetTimer(TimerHandle, [this, PlayerController]()
	{
		HandleRespawn(PlayerController);
	}, DelaySeconds, false);
}

void ABiomeSurvivorGameMode::HandleRespawn(APlayerController* PlayerController)
{
	if (!PlayerController) return;

	PendingRespawns.Remove(PlayerController);
	RestartPlayer(PlayerController);

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Player respawned: %s"), *PlayerController->GetName());
}

void ABiomeSurvivorGameMode::KickPlayer(APlayerController* PlayerController, const FString& Reason)
{
	if (!PlayerController || !HasAuthority()) return;

	UE_LOG(LogBiomeSurvivor, Warning, TEXT("Kicking player: %s | Reason: %s"), *PlayerController->GetName(), *Reason);

	if (APlayerState* PS = PlayerController->GetPlayerState<APlayerState>())
	{
		// TODO: Save player data before kicking
	}

	if (GameSession)
	{
		GameSession->KickPlayer(PlayerController, FText::FromString(Reason));
	}
}

void ABiomeSurvivorGameMode::SpawnEssentialActors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Spawn DayNightCycle if none exists
	TArray<AActor*> DayNightActors;
	UGameplayStatics::GetAllActorsOfClass(World, ADayNightCycle::StaticClass(), DayNightActors);
	if (DayNightActors.Num() == 0)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<ADayNightCycle>(ADayNightCycle::StaticClass(), FTransform::Identity, Params);
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Auto-spawned DayNightCycle"));
	}

	// Spawn WeatherSystem if none exists
	TArray<AActor*> WeatherActors;
	UGameplayStatics::GetAllActorsOfClass(World, AWeatherSystem::StaticClass(), WeatherActors);
	if (WeatherActors.Num() == 0)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AWeatherSystem>(AWeatherSystem::StaticClass(), FTransform::Identity, Params);
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Auto-spawned WeatherSystem"));
	}
}
