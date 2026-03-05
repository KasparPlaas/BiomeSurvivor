// Copyright Biome Survivor. All Rights Reserved.

#include "Wildlife/AnimalSpawner.h"
#include "Wildlife/AnimalBase.h"
#include "BiomeSurvivor.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"

AAnimalSpawner::AAnimalSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 5.0f; // Check every 5 seconds
}

void AAnimalSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnTimer = FMath::FRandRange(0.0f, SpawnInterval); // Stagger initial spawns
}

void AAnimalSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	// Clean up dead/destroyed references
	SpawnedAnimals.RemoveAll([](AAnimalBase* Animal)
	{
		return !Animal || Animal->IsPendingKillPending();
	});

	SpawnTimer += DeltaTime;
	if (SpawnTimer >= SpawnInterval)
	{
		SpawnTimer = 0.0f;
		DespawnDistantAnimals();

		if (IsPlayerNearby(MaxPlayerDistance))
		{
			TrySpawnAnimal();
		}
	}
}

void AAnimalSpawner::ForceSpawnCheck()
{
	SpawnTimer = SpawnInterval;
}

int32 AAnimalSpawner::GetAliveAnimalCount() const
{
	int32 Count = 0;
	for (AAnimalBase* Animal : SpawnedAnimals)
	{
		if (Animal && !Animal->IsDead())
		{
			Count++;
		}
	}
	return Count;
}

void AAnimalSpawner::TrySpawnAnimal()
{
	if (SpawnTable.Num() == 0) return;

	// Don't spawn if player is too close
	if (IsPlayerNearby(MinPlayerDistance)) return;

	// Pick a random entry based on weight
	float TotalWeight = 0.0f;
	for (const FAnimalSpawnEntry& Entry : SpawnTable)
	{
		// Count current spawns of this type
		int32 CurrentCount = 0;
		for (AAnimalBase* Animal : SpawnedAnimals)
		{
			if (Animal && Animal->IsA(Entry.AnimalClass))
			{
				CurrentCount++;
			}
		}

		if (CurrentCount < Entry.MaxCount)
		{
			TotalWeight += Entry.SpawnWeight;
		}
	}

	if (TotalWeight <= 0.0f) return;

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	float Accumulated = 0.0f;

	for (const FAnimalSpawnEntry& Entry : SpawnTable)
	{
		int32 CurrentCount = 0;
		for (AAnimalBase* Animal : SpawnedAnimals)
		{
			if (Animal && Animal->IsA(Entry.AnimalClass))
			{
				CurrentCount++;
			}
		}

		if (CurrentCount >= Entry.MaxCount) continue;

		Accumulated += Entry.SpawnWeight;
		if (Roll <= Accumulated)
		{
			// Spawn this animal
			FVector SpawnPoint = GetRandomSpawnPoint();
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AAnimalBase* Spawned = GetWorld()->SpawnActor<AAnimalBase>(Entry.AnimalClass, SpawnPoint, FRotator::ZeroRotator, Params);
			if (Spawned)
			{
				SpawnedAnimals.Add(Spawned);
				UE_LOG(LogBiomeSurvivor, Log, TEXT("Animal spawned: %s at %s"),
					*Spawned->AnimalName.ToString(), *SpawnPoint.ToString());
			}
			break;
		}
	}
}

void AAnimalSpawner::DespawnDistantAnimals()
{
	for (int32 i = SpawnedAnimals.Num() - 1; i >= 0; --i)
	{
		AAnimalBase* Animal = SpawnedAnimals[i];
		if (!Animal) continue;

		// Don't despawn dead animals (let them have their corpse timer)
		if (Animal->IsDead()) continue;

		if (!IsPlayerNearby(MaxPlayerDistance))
		{
			Animal->Destroy();
			SpawnedAnimals.RemoveAt(i);
		}
	}
}

bool AAnimalSpawner::IsPlayerNearby(float Distance) const
{
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Players);

	for (AActor* Player : Players)
	{
		if (FVector::Dist(GetActorLocation(), Player->GetActorLocation()) < Distance)
		{
			return true;
		}
	}

	return false;
}

FVector AAnimalSpawner::GetRandomSpawnPoint() const
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		if (NavSys->GetRandomReachablePointInRadius(GetActorLocation(), SpawnRadius, NavLoc))
		{
			return NavLoc.Location;
		}
	}

	// Fallback: random point in radius at spawn height
	FVector2D RandDir = FMath::RandPointInCircle(SpawnRadius);
	return GetActorLocation() + FVector(RandDir.X, RandDir.Y, 0.0f);
}
