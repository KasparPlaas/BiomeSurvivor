// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BiomeDefinition.generated.h"

/**
 * Biome types available in the game
 */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
	Forest			UMETA(DisplayName = "Temperate Forest"),
	Tundra			UMETA(DisplayName = "Frozen Tundra"),
	Desert			UMETA(DisplayName = "Arid Desert"),
	Swamp			UMETA(DisplayName = "Swamp Marshland"),
	Mountain		UMETA(DisplayName = "Alpine Mountain"),
	Coast			UMETA(DisplayName = "Coastal Shore"),
	Cave			UMETA(DisplayName = "Underground Cave")
};

/**
 * Weather pattern for a biome
 */
USTRUCT(BlueprintType)
struct FBiomeWeatherPattern
{
	GENERATED_BODY()

	/** Probability of rain (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float RainChance = 0.3f;

	/** Probability of snow (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float SnowChance = 0.0f;

	/** Probability of fog (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float FogChance = 0.2f;

	/** Probability of storm/blizzard (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float StormChance = 0.05f;

	/** Average wind speed in the biome (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageWindSpeed = 5.0f;
};

/**
 * Resource availability in a biome
 */
USTRUCT(BlueprintType)
struct FBiomeResourceInfo
{
	GENERATED_BODY()

	/** Density of trees (0-1, used by PCG) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float TreeDensity = 0.5f;

	/** Density of rocks/ore nodes (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float RockDensity = 0.3f;

	/** Density of berry/herb bushes (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float PlantDensity = 0.4f;

	/** Chance to find water source nearby (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float WaterAvailability = 0.5f;
};

/**
 * Data asset defining a biome's properties
 */
UCLASS(BlueprintType)
class BIOMESURVIVOR_API UBiomeDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique biome ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName BiomeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Biome type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EBiomeType BiomeType;

	/** Ambient temperature at noon (Celsius) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate")
	float DayTemperature = 20.0f;

	/** Ambient temperature at midnight (Celsius) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate")
	float NightTemperature = 10.0f;

	/** Humidity level 0-1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate", meta = (ClampMin = "0", ClampMax = "1"))
	float Humidity = 0.5f;

	/** Weather patterns for this biome */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate")
	FBiomeWeatherPattern WeatherPattern;

	/** Resource availability */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	FBiomeResourceInfo Resources;

	/** Difficulty multiplier: affects hostile spawn rates, weather severity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float DifficultyMultiplier = 1.0f;

	/** Description for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	/** Get temperature at a given game hour (lerp between day and night) */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	float GetTemperatureAtHour(float Hour) const;

	// UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
