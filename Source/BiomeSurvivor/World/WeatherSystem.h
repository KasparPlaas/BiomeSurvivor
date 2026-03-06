// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeatherSystem.generated.h"

class ABiomeManager;
class UNiagaraComponent;
class UNiagaraSystem;
class UAudioComponent;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;

/**
 * Current weather type
 */
UENUM(BlueprintType)
enum class EWeatherType : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Cloudy		UMETA(DisplayName = "Cloudy"),
	Rain		UMETA(DisplayName = "Rain"),
	HeavyRain	UMETA(DisplayName = "Heavy Rain"),
	Snow		UMETA(DisplayName = "Snow"),
	Blizzard	UMETA(DisplayName = "Blizzard"),
	Fog			UMETA(DisplayName = "Dense Fog"),
	Sandstorm	UMETA(DisplayName = "Sandstorm"),
	Thunderstorm UMETA(DisplayName = "Thunderstorm")
};

/**
 * Delegate for weather changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EWeatherType, OldWeather, EWeatherType, NewWeather);

/**
 * Global weather system that responds to biome data
 * and creates dynamic weather transitions.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIVOR_API AWeatherSystem : public AActor
{
	GENERATED_BODY()

public:
	AWeatherSystem();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- State ---

	/** Current active weather type */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeather, BlueprintReadOnly, Category = "Weather")
	EWeatherType CurrentWeather = EWeatherType::Clear;

	/** Current wind speed (m/s) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weather")
	float WindSpeed = 0.0f;

	/** Current wind direction (world space yaw degrees) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weather")
	float WindDirection = 0.0f;

	/** Weather intensity 0-1 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weather")
	float WeatherIntensity = 0.0f;

	/** Current wetness level 0-1 (increases during rain/snow) */
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float CurrentWetness = 0.0f;

	// --- Config ---

	/** Minimum time between weather changes (seconds of game time) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MinWeatherDuration = 120.0f;

	/** Maximum time between weather changes (seconds of game time) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxWeatherDuration = 600.0f;

	/** Speed of weather transition (0-1 per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float TransitionSpeed = 0.2f;

	/** Temperature modifier from current weather */
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float WeatherTemperatureModifier = 0.0f;

	// --- VFX References ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> RainParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SnowParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SandstormParticleSystem;

	// --- Audio References ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> RainAmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> WindAmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> ThunderSound;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Weather")
	FOnWeatherChanged OnWeatherChanged;

	// --- Interface ---

	/** Force weather change (authority only) */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(EWeatherType NewWeather, float Intensity = 1.0f);

	/** Get effective temperature modifier from weather */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	float GetWeatherTemperatureModifier() const { return WeatherTemperatureModifier; }

	/** Get effective wind chill factor */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	float GetWindChillFactor() const;

	/** Is it currently precipitating? */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	bool IsPrecipitating() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_CurrentWeather();

	void UpdateWeatherTimer(float DeltaTime);
	void ChooseNextWeather();
	void UpdateVFX(float DeltaTime);
	void UpdateWeatherEffects(float DeltaTime);

	/** Reference to the biome manager */
	UPROPERTY()
	TObjectPtr<ABiomeManager> BiomeManagerRef;

	/** Active particle component */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveParticleComp;

	/** Wind audio component */
	UPROPERTY()
	TObjectPtr<UAudioComponent> WindAudioComp;

	/** Rain audio component */
	UPROPERTY()
	TObjectPtr<UAudioComponent> RainAudioComp;

	float WeatherTimer = 0.0f;
	float NextWeatherChangeTime = 300.0f;
	float TargetIntensity = 0.0f;
	EWeatherType PendingWeather = EWeatherType::Clear;
};
