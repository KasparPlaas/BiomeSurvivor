// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/BiomeDefinition.h"
#include "BiomeManager.generated.h"

class UBoxComponent;

/**
 * Represents a biome zone in the world.
 * Place these in the level to define biome boundaries.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIVOR_API ABiomeZone : public AActor
{
	GENERATED_BODY()

public:
	ABiomeZone();

	/** Volume defining the zone boundary */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> ZoneVolume;

	/** Biome definition for this zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
	TObjectPtr<UBiomeDefinition> BiomeData;

	/** Priority for overlapping zones (higher wins) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
	int32 Priority = 0;

	/** Check if a world location is inside this zone */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	bool IsLocationInZone(const FVector& Location) const;
};

/**
 * Manages all biome zones in the world.
 * Provides queries for biome data at any location.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIVOR_API ABiomeManager : public AActor
{
	GENERATED_BODY()

public:
	ABiomeManager();

	virtual void BeginPlay() override;

	/** Get the biome definition at a world location */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	UBiomeDefinition* GetBiomeAtLocation(const FVector& Location) const;

	/** Get the current temperature at a world location, considering time of day */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	float GetTemperatureAtLocation(const FVector& Location, float GameHour) const;

	/** Get the biome type at a location */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	EBiomeType GetBiomeTypeAtLocation(const FVector& Location) const;

	/** Default biome definition used when outside all zones */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	TObjectPtr<UBiomeDefinition> DefaultBiome;

	/** Blend radius between biomes (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blending")
	float BiomeBlendRadius = 2000.0f;

private:
	/** Cached list of all biome zones */
	UPROPERTY()
	TArray<ABiomeZone*> BiomeZones;
};
