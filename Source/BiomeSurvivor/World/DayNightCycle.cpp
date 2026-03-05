// Copyright Biome Survivor. All Rights Reserved.

#include "World/DayNightCycle.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"
#include "Components/DirectionalLightComponent.h"

ADayNightCycle::ADayNightCycle()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;

	// Sun light
	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetIntensity(SunPeakIntensity);
	SunLight->SetAtmosphereSunLight(true);
	SunLight->SetCastShadows(true);
	SunLight->SetDynamicShadowCascades(4);
	RootComponent = SunLight;

	// Moon light
	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->SetIntensity(0.0f);
	MoonLight->SetCastShadows(false);
	MoonLight->SetLightColor(MoonColor);
	MoonLight->SetupAttachment(RootComponent);
}

void ADayNightCycle::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GameTimeHours = StartingHour;
		LastBroadcastHour = FMath::FloorToInt(StartingHour);
		bWasNight = IsNight();
	}
}

void ADayNightCycle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		AdvanceTime(DeltaTime);
	}

	UpdateSunPosition();
	UpdateLighting();
}

void ADayNightCycle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADayNightCycle, GameTimeHours);
	DOREPLIFETIME(ADayNightCycle, CurrentDay);
}

bool ADayNightCycle::IsNight() const
{
	return GameTimeHours >= NightStartHour || GameTimeHours < DayStartHour;
}

bool ADayNightCycle::IsDawn() const
{
	return GameTimeHours >= (DayStartHour - 1.0f) && GameTimeHours < (DayStartHour + 1.0f);
}

bool ADayNightCycle::IsDusk() const
{
	return GameTimeHours >= (NightStartHour - 1.0f) && GameTimeHours < (NightStartHour + 1.0f);
}

float ADayNightCycle::GetAmbientLightLevel() const
{
	// Full daylight: 8:00 - 18:00
	// Dawn/dusk transition: 6:00-8:00 and 18:00-20:00
	// Night: 20:00 - 6:00

	if (GameTimeHours >= 8.0f && GameTimeHours < 18.0f)
	{
		return 1.0f;
	}
	else if (GameTimeHours >= DayStartHour && GameTimeHours < 8.0f)
	{
		// Dawn ramp up
		return (GameTimeHours - DayStartHour) / 2.0f;
	}
	else if (GameTimeHours >= 18.0f && GameTimeHours < NightStartHour)
	{
		// Dusk ramp down
		return 1.0f - ((GameTimeHours - 18.0f) / 2.0f);
	}
	else
	{
		// Night - moonlight provides small ambient
		return 0.05f;
	}
}

void ADayNightCycle::SetTimeOfDay(float Hour)
{
	if (!HasAuthority()) return;

	GameTimeHours = FMath::Fmod(FMath::Max(Hour, 0.0f), 24.0f);
	LastBroadcastHour = FMath::FloorToInt(GameTimeHours);
}

void ADayNightCycle::OnRep_GameTime()
{
	UpdateSunPosition();
	UpdateLighting();
}

void ADayNightCycle::AdvanceTime(float DeltaTime)
{
	if (bTimePaused) return;

	// Convert real-time seconds to game-time hours
	// DayCycleDurationSeconds real seconds = 24 game hours
	const float HoursPerSecond = 24.0f / DayCycleDurationSeconds;
	GameTimeHours += HoursPerSecond * DeltaTime;

	// Check for new day
	if (GameTimeHours >= 24.0f)
	{
		GameTimeHours -= 24.0f;
		CurrentDay++;
		OnDayChanged.Broadcast(CurrentDay);
		UE_LOG(LogBiomeSurvivor, Log, TEXT("New day: Day %d"), CurrentDay);
	}

	// Broadcast hour changes
	int32 CurrentHour = FMath::FloorToInt(GameTimeHours);
	if (CurrentHour != LastBroadcastHour)
	{
		LastBroadcastHour = CurrentHour;
		OnHourChanged.Broadcast(CurrentHour);
	}

	// Broadcast day/night transitions
	bool bCurrentlyNight = IsNight();
	if (bCurrentlyNight != bWasNight)
	{
		bWasNight = bCurrentlyNight;
		OnTimeOfDayChanged.Broadcast(bCurrentlyNight);
	}
}

void ADayNightCycle::UpdateSunPosition()
{
	// Calculate sun elevation based on time
	// Noon (12:00) = highest point, Midnight (0:00) = lowest
	const float SunAngle = ((GameTimeHours - 6.0f) / 24.0f) * 360.0f;

	// Latitude affects how high the sun gets
	const float Elevation = -SunAngle;
	const float Azimuth = 180.0f; // Sun moves east to west

	FRotator SunRotation(Elevation, Azimuth, 0.0f);
	SunLight->SetWorldRotation(SunRotation);

	// Moon is opposite the sun
	FRotator MoonRotation(Elevation + 180.0f, Azimuth, 0.0f);
	MoonLight->SetWorldRotation(MoonRotation);
}

void ADayNightCycle::UpdateLighting()
{
	float LightLevel = GetAmbientLightLevel();

	// Sun intensity
	float SunIntensity = SunPeakIntensity * LightLevel;
	SunLight->SetIntensity(SunIntensity);

	// Sun color: lerp from horizon color at dawn/dusk to noon color at peak
	float ColorAlpha = FMath::Clamp((LightLevel - 0.3f) / 0.7f, 0.0f, 1.0f);
	FLinearColor CurrentSunColor = FMath::Lerp(SunHorizonColor, SunNoonColor, ColorAlpha);
	SunLight->SetLightColor(CurrentSunColor);

	// Moon intensity (inverse of sun, with smooth transition)
	float MoonIntensity = MoonPeakIntensity * (1.0f - LightLevel);
	MoonLight->SetIntensity(MoonIntensity);
	MoonLight->SetLightColor(MoonColor);

	// Enable/disable shadows based on light source
	SunLight->SetCastShadows(LightLevel > 0.1f);
	MoonLight->SetCastShadows(LightLevel < 0.1f && MoonIntensity > 0.1f);
}
