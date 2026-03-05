// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TemperatureComponent.generated.h"

/**
 * UTemperatureComponent
 * Manages body temperature. External temp from biome/weather + clothing warmth.
 * Hypothermia below 35C, Hyperthermia above 39C.
 * Values from plan.md Section 5.6.
 */

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API UTemperatureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTemperatureComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Current body temperature in Celsius */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Temperature")
	float BodyTemperature = 37.0f;

	/** Normal body temperature */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Temperature")
	float NormalBodyTemp = 37.0f;

	/** Hypothermia threshold */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Temperature")
	float HypothermiaThreshold = 35.0f;

	/** Hyperthermia threshold */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Temperature")
	float HyperthermiaThreshold = 39.0f;

	/** Rate body temp changes toward ambient (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Temperature")
	float TempChangeRate = 0.05f;

	/** External ambient temperature (set by biome/weather) */
	UPROPERTY(BlueprintReadWrite, Category="Temperature")
	float AmbientTemperature = 20.0f;

	/** Wind chill factor (0.0 = no wind, 1.0 = strong wind) */
	UPROPERTY(BlueprintReadWrite, Category="Temperature")
	float WindChill = 0.0f;

	/** Wetness factor (0.0 = dry, 1.0 = soaked) - increases cold effect */
	UPROPERTY(BlueprintReadWrite, Category="Temperature")
	float Wetness = 0.0f;

	/** Is player near a heat source? (campfire, building) */
	UPROPERTY(BlueprintReadWrite, Category="Temperature")
	bool bNearHeatSource = false;

	/** Heat source warmth bonus */
	UPROPERTY(BlueprintReadWrite, Category="Temperature")
	float HeatSourceWarmth = 10.0f;

	// ---- API ----

	UFUNCTION(BlueprintPure, Category="Temperature")
	bool IsHypothermic() const { return BodyTemperature < HypothermiaThreshold; }

	UFUNCTION(BlueprintPure, Category="Temperature")
	bool IsHyperthermic() const { return BodyTemperature > HyperthermiaThreshold; }

	UFUNCTION(BlueprintPure, Category="Temperature")
	float GetEffectiveAmbientTemp() const;

	UFUNCTION(BlueprintPure, Category="Temperature")
	FText GetTemperatureStatusText() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTemperatureChanged, float, NewTemp);
	UPROPERTY(BlueprintAssignable, Category="Temperature")
	FOnTemperatureChanged OnTemperatureChanged;

private:
	void ApplyTemperatureEffects(float DeltaTime);
};
