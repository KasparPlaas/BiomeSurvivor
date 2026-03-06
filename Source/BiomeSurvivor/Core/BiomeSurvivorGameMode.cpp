// Copyright Biome Survivor. All Rights Reserved.

#include "Core/BiomeSurvivorGameMode.h"
#include "Core/BiomeSurvivorGameState.h"
#include "Core/BiomeSurvivorPlayerController.h"
#include "Core/BiomeSurvivorPlayerState.h"
#include "Player/SurvivorCharacter.h"
#include "UI/SurvivorHUD.h"
#include "World/DayNightCycle.h"
#include "World/WeatherSystem.h"
#include "World/ResourceNode.h"
#include "World/LootContainer.h"
#include "Wildlife/AnimalBase.h"
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

	// Spawn world resources and wildlife
	SpawnWorldResources();
	SpawnWildlife();
	SpawnLootContainers();
}

void ABiomeSurvivorGameMode::SpawnWorldResources()
{
	UWorld* World = GetWorld();
	if (!World) return;

	auto SpawnResource = [&](FVector Location, EResourceNodeType Type, FText Name, float Health,
		FName YieldItem, int32 BaseQty, int32 BonusQty, float RespawnTime)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, Location);
		AResourceNode* Node = World->SpawnActorDeferred<AResourceNode>(
			AResourceNode::StaticClass(), SpawnTransform);
		if (Node)
		{
			Node->ResourceType = Type;
			Node->ResourceName = Name;
			Node->MaxHealth = Health;
			Node->CurrentHealth = Health;
			Node->RespawnTimeSeconds = RespawnTime;

			FResourceYield Yield;
			Yield.ItemID = YieldItem;
			Yield.BaseQuantity = BaseQty;
			Yield.BonusQuantity = BonusQty;
			Yield.DropChance = 1.0f;
			Node->HarvestYields.Add(Yield);

			Node->FinishSpawning(SpawnTransform);
		}
		return Node;
	};

	// ---- TREES (spread around the terrain) ----
	const float TerrainHalf = 15000.0f; // Terrain radius
	FRandomStream Rand(42); // Deterministic seed

	for (int32 i = 0; i < 20; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf, TerrainHalf),
			Rand.FRandRange(-TerrainHalf, TerrainHalf),
			100.0f
		);
		SpawnResource(Pos, EResourceNodeType::Tree, FText::FromString(TEXT("Pine Tree")),
			150.0f, FName("Wood"), 2, 2, 300.0f);
	}

	// ---- ROCKS ----
	for (int32 i = 0; i < 12; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf, TerrainHalf),
			Rand.FRandRange(-TerrainHalf, TerrainHalf),
			50.0f
		);
		SpawnResource(Pos, EResourceNodeType::Rock, FText::FromString(TEXT("Stone Boulder")),
			200.0f, FName("Stone"), 1, 2, 600.0f);
	}

	// ---- BERRY BUSHES ----
	for (int32 i = 0; i < 8; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf * 0.5f, TerrainHalf * 0.5f),
			Rand.FRandRange(-TerrainHalf * 0.5f, TerrainHalf * 0.5f),
			50.0f
		);
		SpawnResource(Pos, EResourceNodeType::BerryBush, FText::FromString(TEXT("Berry Bush")),
			30.0f, FName("Berries"), 3, 3, 120.0f);
	}

	// ---- FIBER PLANTS ----
	for (int32 i = 0; i < 6; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf * 0.7f, TerrainHalf * 0.7f),
			Rand.FRandRange(-TerrainHalf * 0.7f, TerrainHalf * 0.7f),
			50.0f
		);
		SpawnResource(Pos, EResourceNodeType::FiberPlant, FText::FromString(TEXT("Fiber Plant")),
			20.0f, FName("Fiber"), 2, 2, 90.0f);
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Spawned world resources: 20 trees, 12 rocks, 8 bushes, 6 fiber"));
}

void ABiomeSurvivorGameMode::SpawnWildlife()
{
	UWorld* World = GetWorld();
	if (!World) return;

	auto SpawnAnimal = [&](FVector Location, FName ID, FText Name, EAnimalBehavior Behavior,
		float HP, float WalkSpd, float RunSpd, float AtkDmg, float DetectRange,
		FName LootItem, int32 LootMin, int32 LootMax)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, Location);
		AAnimalBase* Animal = World->SpawnActorDeferred<AAnimalBase>(
			AAnimalBase::StaticClass(), SpawnTransform);
		if (Animal)
		{
			Animal->AnimalID = ID;
			Animal->AnimalName = Name;
			Animal->BehaviorType = Behavior;
			Animal->MaxHealth = HP;
			Animal->Health = HP;
			Animal->WalkSpeed = WalkSpd;
			Animal->RunSpeed = RunSpd;
			Animal->AttackDamage = AtkDmg;
			Animal->DetectionRadius = DetectRange;

			FAnimalLootDrop Loot;
			Loot.ItemID = LootItem;
			Loot.MinCount = LootMin;
			Loot.MaxCount = LootMax;
			Loot.DropChance = 1.0f;
			Animal->LootTable.Add(Loot);

			// Second loot drop (hide/leather)
			FAnimalLootDrop HideLoot;
			HideLoot.ItemID = FName("AnimalHide");
			HideLoot.MinCount = 1;
			HideLoot.MaxCount = 2;
			HideLoot.DropChance = 0.8f;
			Animal->LootTable.Add(HideLoot);

			Animal->FinishSpawning(SpawnTransform);
		}
		return Animal;
	};

	const float TerrainHalf = 15000.0f;
	FRandomStream Rand(123);

	// ---- DEER (passive, flee from player) ----
	for (int32 i = 0; i < 6; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf * 0.8f, TerrainHalf * 0.8f),
			Rand.FRandRange(-TerrainHalf * 0.8f, TerrainHalf * 0.8f),
			100.0f
		);
		SpawnAnimal(Pos, FName("Deer"), FText::FromString(TEXT("Deer")),
			EAnimalBehavior::Passive, 80.0f, 200.0f, 700.0f, 0.0f, 2500.0f,
			FName("RawMeat"), 2, 4);
	}

	// ---- WOLVES (aggressive, hunt player) ----
	for (int32 i = 0; i < 3; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf * 0.6f, TerrainHalf * 0.6f),
			Rand.FRandRange(-TerrainHalf * 0.6f, TerrainHalf * 0.6f),
			100.0f
		);
		SpawnAnimal(Pos, FName("Wolf"), FText::FromString(TEXT("Wolf")),
			EAnimalBehavior::Aggressive, 120.0f, 250.0f, 800.0f, 20.0f, 3000.0f,
			FName("RawMeat"), 1, 3);
	}

	// ---- RABBITS (passive, flee quickly) ----
	for (int32 i = 0; i < 5; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf * 0.7f, TerrainHalf * 0.7f),
			Rand.FRandRange(-TerrainHalf * 0.7f, TerrainHalf * 0.7f),
			100.0f
		);
		SpawnAnimal(Pos, FName("Rabbit"), FText::FromString(TEXT("Rabbit")),
			EAnimalBehavior::Passive, 20.0f, 180.0f, 900.0f, 0.0f, 1500.0f,
			FName("RawMeat"), 1, 1);
	}

	// ---- BOARS (territorial) ----
	for (int32 i = 0; i < 3; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf * 0.5f, TerrainHalf * 0.5f),
			Rand.FRandRange(-TerrainHalf * 0.5f, TerrainHalf * 0.5f),
			100.0f
		);
		SpawnAnimal(Pos, FName("Boar"), FText::FromString(TEXT("Wild Boar")),
			EAnimalBehavior::Territorial, 100.0f, 180.0f, 600.0f, 15.0f, 2000.0f,
			FName("RawMeat"), 2, 3);
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Spawned wildlife: 6 deer, 3 wolves, 5 rabbits, 3 boars"));
}

void ABiomeSurvivorGameMode::SpawnLootContainers()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const float TerrainHalf = 15000.0f;
	FRandomStream Rand(777);

	auto SpawnContainer = [&](FVector Location, EContainerType Type, FText Name, int32 Slots)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, Location);
		ALootContainer* Container = World->SpawnActorDeferred<ALootContainer>(
			ALootContainer::StaticClass(), SpawnTransform);
		if (Container)
		{
			Container->ContainerType = Type;
			Container->ContainerName = Name;
			Container->ContainerSlots = Slots;
			Container->bUseLootTable = false; // We'll manually add items

			Container->FinishSpawning(SpawnTransform);

			// Manually add items since loot table needs PrimaryAssetIds which don't exist
			if (UInventoryComponent* Inv = Container->GetContainerInventory())
			{
				// Add random survival supplies
				int32 NumItems = Rand.RandRange(2, 5);
				TArray<FName> PossibleItems = { FName("Wood"), FName("Stone"), FName("Berries"), FName("Fiber"), FName("Bandage") };
				for (int32 i = 0; i < NumItems; ++i)
				{
					FName ItemID = PossibleItems[Rand.RandRange(0, PossibleItems.Num() - 1)];
					int32 Count = Rand.RandRange(1, 5);
					Inv->AddItem(ItemID, Count);
				}
			}
		}
		return Container;
	};

	// Spawn 5 loot containers scattered around the world
	for (int32 i = 0; i < 5; ++i)
	{
		FVector Pos(
			Rand.FRandRange(-TerrainHalf * 0.6f, TerrainHalf * 0.6f),
			Rand.FRandRange(-TerrainHalf * 0.6f, TerrainHalf * 0.6f),
			50.0f
		);

		EContainerType Type = (i % 2 == 0) ? EContainerType::Chest : EContainerType::Crate;
		FText Name = (Type == EContainerType::Chest)
			? FText::FromString(TEXT("Wooden Chest"))
			: FText::FromString(TEXT("Supply Crate"));

		SpawnContainer(Pos, Type, Name, 8);
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Spawned 5 loot containers"));
}
