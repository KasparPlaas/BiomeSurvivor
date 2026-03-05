// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NutritionComponent.generated.h"

/**
 * UNutritionComponent
 * Tracks calorie intake, food quality effects, and cooking bonuses.
 * Works alongside PlayerStatsComponent hunger/thirst for detailed nutrition.
 * Values from plan.md Section 5.6.
 */

UENUM(BlueprintType)
enum class EFoodQuality : uint8
{
	Raw			UMETA(DisplayName = "Raw"),
	Cooked		UMETA(DisplayName = "Cooked"),
	WellCooked	UMETA(DisplayName = "Well Cooked"),
	Burned		UMETA(DisplayName = "Burned"),
	Spoiled		UMETA(DisplayName = "Spoiled")
};

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API UNutritionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNutritionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Current calorie count */
	UPROPERTY(BlueprintReadOnly, Category="Nutrition")
	float CurrentCalories = 2000.0f;

	/** Daily calorie requirement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nutrition")
	float DailyCalorieNeed = 2000.0f;

	/** Calorie burn rate per second (base) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nutrition")
	float BaseCalorieBurnRate = 1.4f; // ~2000 calories per 24min game day

	/** Bonus multiplier for cooked food nutrition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nutrition")
	float CookedFoodBonus = 1.5f;

	/** Consume food item */
	UFUNCTION(BlueprintCallable, Category="Nutrition")
	void ConsumeFood(float Calories, float HungerRestore, float ThirstRestore, EFoodQuality Quality, float PoisonChance);

	/** Is the player well-nourished? */
	UFUNCTION(BlueprintPure, Category="Nutrition")
	bool IsWellFed() const { return CurrentCalories >= DailyCalorieNeed * 0.7f; }

	/** Is the player malnourished? */
	UFUNCTION(BlueprintPure, Category="Nutrition")
	bool IsMalnourished() const { return CurrentCalories < DailyCalorieNeed * 0.3f; }

	/** Get calorie percentage */
	UFUNCTION(BlueprintPure, Category="Nutrition")
	float GetCaloriePercent() const { return CurrentCalories / DailyCalorieNeed; }
};
