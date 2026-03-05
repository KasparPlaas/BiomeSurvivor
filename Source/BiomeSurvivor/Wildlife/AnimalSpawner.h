// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AnimalSpawner.generated.h"

class AAnimalBase;

/**
 * AAnimalSpawner
 * Manages wildlife population in a region. Spawns/despawns animals based on
 * biome, time of day, player proximity, and population caps.
 */

USTRUCT(BlueprintType)
struct FAnimalSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AAnimalBase> AnimalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNocturnalOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDiurnalOnly = false;
};

UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API AAnimalSpawner : public AActor
{
	GENERATED_BODY()

public:
	AAnimalSpawner();

	/** Spawn entries (which animals and how many) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
	TArray<FAnimalSpawnEntry> SpawnTable;

	/** Spawn radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	float SpawnRadius = 5000.0f;

	/** Min distance from any player to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	float MinPlayerDistance = 3000.0f;

	/** Max distance from any player before despawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	float MaxPlayerDistance = 15000.0f;

	/** Spawn check interval in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	float SpawnInterval = 30.0f;

	/** Biome ID this spawner belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
	FName BiomeID;

	/** Force a spawn check now */
	UFUNCTION(BlueprintCallable, Category="Spawner")
	void ForceSpawnCheck();

	/** Get current alive animal count */
	UFUNCTION(BlueprintPure, Category="Spawner")
	int32 GetAliveAnimalCount() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TArray<AAnimalBase*> SpawnedAnimals;

	float SpawnTimer = 0.0f;

	void TrySpawnAnimal();
	void DespawnDistantAnimals();
	bool IsPlayerNearby(float Distance) const;
	FVector GetRandomSpawnPoint() const;
};
