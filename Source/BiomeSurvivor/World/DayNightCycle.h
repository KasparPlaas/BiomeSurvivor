// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DayNightCycle.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;
class USkyAtmosphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHourChanged, int32, NewHour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32, NewDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, bool, bIsNight);

/**
 * Manages the day/night cycle including sun positioning,
 * lighting transitions, and time progression.
 * 
 * Full cycle: 24 minutes real time = 24 hours game time (default)
 * Replicates time to all clients.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API ADayNightCycle : public AActor
{
	GENERATED_BODY()

public:
	ADayNightCycle();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Components ---

	/** Directional light acting as the sun */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	/** Directional light acting as the moon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> MoonLight;

	// --- Time State ---

	/** Current game time in hours (0.0 - 24.0) */
	UPROPERTY(ReplicatedUsing = OnRep_GameTime, BlueprintReadOnly, Category = "Time")
	float GameTimeHours = 8.0f;

	/** Current game day count */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	int32 CurrentDay = 1;

	// --- Config ---

	/** Real-time seconds for one full day cycle. Default: 1440s = 24 min */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float DayCycleDurationSeconds = 1440.0f;

	/** Starting hour of the game (0-24) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float StartingHour = 8.0f;

	/** Hour when night begins (for gameplay purposes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float NightStartHour = 20.0f;

	/** Hour when day begins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float DayStartHour = 6.0f;

	/** Sun intensity during peak day */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float SunPeakIntensity = 10.0f;

	/** Moon intensity at night */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float MoonPeakIntensity = 0.5f;

	/** Sun light color at noon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor SunNoonColor = FLinearColor(1.0f, 0.95f, 0.85f);

	/** Sun light color at sunrise/sunset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor SunHorizonColor = FLinearColor(1.0f, 0.5f, 0.2f);

	/** Moon light color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor MoonColor = FLinearColor(0.4f, 0.5f, 0.7f);

	/** Latitude for sun angle calculation. 0=equator, 90=north pole */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "-90", ClampMax = "90"))
	float WorldLatitude = 50.0f;

	// --- Interface ---

	UFUNCTION(BlueprintCallable, Category = "Time")
	bool IsNight() const;

	UFUNCTION(BlueprintCallable, Category = "Time")
	bool IsDawn() const;

	UFUNCTION(BlueprintCallable, Category = "Time")
	bool IsDusk() const;

	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetNormalizedTimeOfDay() const { return GameTimeHours / 24.0f; }

	/** Get ambient light level (0=pitch dark, 1=full daylight) */
	UFUNCTION(BlueprintCallable, Category = "Lighting")
	float GetAmbientLightLevel() const;

	/** Set time of day (authority only) */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeOfDay(float Hour);

	/** Pause/resume time progression */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimePaused(bool bPaused) { bTimePaused = bPaused; }

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnHourChanged OnHourChanged;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnDayChanged OnDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

private:
	UFUNCTION()
	void OnRep_GameTime();

	void AdvanceTime(float DeltaTime);
	void UpdateSunPosition();
	void UpdateLighting();

	bool bTimePaused = false;
	int32 LastBroadcastHour = -1;
	bool bWasNight = false;
};
