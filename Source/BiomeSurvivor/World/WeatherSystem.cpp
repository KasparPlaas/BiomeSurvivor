// Copyright Biome Survivor. All Rights Reserved.

#include "World/WeatherSystem.h"
#include "World/BiomeManager.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

AWeatherSystem::AWeatherSystem()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AWeatherSystem::BeginPlay()
{
	Super::BeginPlay();

	// Find the biome manager
	BiomeManagerRef = Cast<ABiomeManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ABiomeManager::StaticClass()));

	if (HasAuthority())
	{
		NextWeatherChangeTime = FMath::FRandRange(MinWeatherDuration, MaxWeatherDuration);

		// Randomize initial wind
		WindDirection = FMath::FRandRange(0.0f, 360.0f);
		WindSpeed = FMath::FRandRange(1.0f, 8.0f);
	}
}

void AWeatherSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		UpdateWeatherTimer(DeltaTime);
	}

	UpdateWeatherEffects(DeltaTime);
	UpdateVFX(DeltaTime);
}

void AWeatherSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeatherSystem, CurrentWeather);
	DOREPLIFETIME(AWeatherSystem, WindSpeed);
	DOREPLIFETIME(AWeatherSystem, WindDirection);
	DOREPLIFETIME(AWeatherSystem, WeatherIntensity);
}

void AWeatherSystem::SetWeather(EWeatherType NewWeather, float Intensity)
{
	if (!HasAuthority()) return;

	EWeatherType OldWeather = CurrentWeather;
	CurrentWeather = NewWeather;
	TargetIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	OnWeatherChanged.Broadcast(OldWeather, NewWeather);

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Weather changed: %d -> %d (Intensity: %.1f)"),
		(int32)OldWeather, (int32)NewWeather, TargetIntensity);
}

float AWeatherSystem::GetWindChillFactor() const
{
	// Wind chill increases with wind speed
	// Returns temperature reduction in Celsius
	if (WindSpeed < 2.0f) return 0.0f;
	return FMath::Clamp(WindSpeed * 0.8f, 0.0f, 15.0f);
}

bool AWeatherSystem::IsPrecipitating() const
{
	return CurrentWeather == EWeatherType::Rain ||
		   CurrentWeather == EWeatherType::HeavyRain ||
		   CurrentWeather == EWeatherType::Snow ||
		   CurrentWeather == EWeatherType::Blizzard ||
		   CurrentWeather == EWeatherType::Thunderstorm;
}

void AWeatherSystem::OnRep_CurrentWeather()
{
	// Clients respond to weather changes
	// VFX updates handled in UpdateVFX
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Weather replicated: %d"), (int32)CurrentWeather);
}

void AWeatherSystem::UpdateWeatherTimer(float DeltaTime)
{
	WeatherTimer += DeltaTime;

	// Slowly drift wind direction
	WindDirection = FMath::Fmod(WindDirection + FMath::FRandRange(-2.0f, 2.0f) * DeltaTime, 360.0f);

	if (WeatherTimer >= NextWeatherChangeTime)
	{
		WeatherTimer = 0.0f;
		NextWeatherChangeTime = FMath::FRandRange(MinWeatherDuration, MaxWeatherDuration);
		ChooseNextWeather();
	}
}

void AWeatherSystem::ChooseNextWeather()
{
	// Weight chances based on current biome (use camera location as reference)
	float RainChance = 0.25f;
	float SnowChance = 0.05f;
	float FogChance = 0.15f;
	float StormChance = 0.05f;
	float ClearChance = 0.5f;

	if (BiomeManagerRef)
	{
		// Get biome at world origin or first player - simplified
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC && PC->GetPawn())
		{
			FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
			UBiomeDefinition* Biome = BiomeManagerRef->GetBiomeAtLocation(PlayerLoc);
			if (Biome)
			{
				RainChance = Biome->WeatherPattern.RainChance;
				SnowChance = Biome->WeatherPattern.SnowChance;
				FogChance = Biome->WeatherPattern.FogChance;
				StormChance = Biome->WeatherPattern.StormChance;
				ClearChance = 1.0f - RainChance - SnowChance - FogChance - StormChance;
				ClearChance = FMath::Max(ClearChance, 0.1f);

				WindSpeed = Biome->WeatherPattern.AverageWindSpeed + FMath::FRandRange(-3.0f, 5.0f);
			}
		}
	}

	// Weighted random selection
	float Total = ClearChance + RainChance + SnowChance + FogChance + StormChance;
	float Roll = FMath::FRandRange(0.0f, Total);

	EWeatherType NewWeather;
	float Intensity = FMath::FRandRange(0.3f, 1.0f);

	if (Roll < ClearChance)
	{
		NewWeather = (FMath::FRand() < 0.4f) ? EWeatherType::Cloudy : EWeatherType::Clear;
		Intensity = FMath::FRandRange(0.0f, 0.3f);
	}
	else if (Roll < ClearChance + RainChance)
	{
		NewWeather = (FMath::FRand() < 0.3f) ? EWeatherType::HeavyRain : EWeatherType::Rain;
	}
	else if (Roll < ClearChance + RainChance + SnowChance)
	{
		NewWeather = (FMath::FRand() < 0.2f) ? EWeatherType::Blizzard : EWeatherType::Snow;
	}
	else if (Roll < ClearChance + RainChance + SnowChance + FogChance)
	{
		NewWeather = EWeatherType::Fog;
	}
	else
	{
		NewWeather = EWeatherType::Thunderstorm;
		Intensity = FMath::FRandRange(0.6f, 1.0f);
	}

	SetWeather(NewWeather, Intensity);
}

void AWeatherSystem::UpdateWeatherEffects(float DeltaTime)
{
	// Smoothly transition intensity
	WeatherIntensity = FMath::FInterpTo(WeatherIntensity, TargetIntensity, DeltaTime, TransitionSpeed);

	// Update temperature modifier based on weather
	switch (CurrentWeather)
	{
	case EWeatherType::Clear:
		WeatherTemperatureModifier = FMath::FInterpTo(WeatherTemperatureModifier, 0.0f, DeltaTime, 1.0f);
		break;
	case EWeatherType::Cloudy:
		WeatherTemperatureModifier = FMath::FInterpTo(WeatherTemperatureModifier, -2.0f, DeltaTime, 1.0f);
		break;
	case EWeatherType::Rain:
	case EWeatherType::HeavyRain:
		WeatherTemperatureModifier = FMath::FInterpTo(WeatherTemperatureModifier, -5.0f * WeatherIntensity, DeltaTime, 1.0f);
		break;
	case EWeatherType::Snow:
		WeatherTemperatureModifier = FMath::FInterpTo(WeatherTemperatureModifier, -8.0f * WeatherIntensity, DeltaTime, 1.0f);
		break;
	case EWeatherType::Blizzard:
		WeatherTemperatureModifier = FMath::FInterpTo(WeatherTemperatureModifier, -15.0f * WeatherIntensity, DeltaTime, 1.0f);
		break;
	case EWeatherType::Fog:
		WeatherTemperatureModifier = FMath::FInterpTo(WeatherTemperatureModifier, -3.0f, DeltaTime, 1.0f);
		break;
	case EWeatherType::Sandstorm:
		WeatherTemperatureModifier = FMath::FInterpTo(WeatherTemperatureModifier, 5.0f * WeatherIntensity, DeltaTime, 1.0f);
		break;
	case EWeatherType::Thunderstorm:
		WeatherTemperatureModifier = FMath::FInterpTo(WeatherTemperatureModifier, -7.0f * WeatherIntensity, DeltaTime, 1.0f);
		break;
	}

	// Update wetness
	if (IsPrecipitating())
	{
		CurrentWetness = FMath::Clamp(CurrentWetness + WeatherIntensity * 0.05f * DeltaTime, 0.0f, 1.0f);
	}
	else
	{
		CurrentWetness = FMath::Clamp(CurrentWetness - 0.02f * DeltaTime, 0.0f, 1.0f);
	}
}

void AWeatherSystem::UpdateVFX(float DeltaTime)
{
	// VFX updates - spawn/despawn Niagara systems based on weather
	// Full implementation would attach particle systems to player camera
	// and adjust fog density, sky sphere, etc.

	// This is handled in Blueprints/Materials for visual quality,
	// but the C++ system drives the state and parameters.
}
