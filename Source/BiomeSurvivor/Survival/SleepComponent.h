// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SleepComponent.generated.h"

/**
 * USleepComponent
 * Manages fatigue, sleep cycle, and rest bonuses.
 * Extended wakefulness causes penalties. Sleeping restores fatigue and gives comfort.
 * Values from plan.md Section 5.6.
 */

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API USleepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USleepComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Fatigue level (0 = fully rested, 100 = exhausted) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Sleep")
	float Fatigue = 0.0f;

	/** Fatigue accumulation rate per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sleep")
	float FatigueRate = 0.07f; // ~100 in 24 min game-day

	/** Fatigue threshold where penalties start */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sleep")
	float FatiguePenaltyThreshold = 70.0f;

	/** Is player currently sleeping? */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Sleep")
	bool bIsSleeping = false;

	/** Sleep recovery rate per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sleep")
	float SleepRecoveryRate = 5.0f;

	/** Indoor/bed quality bonus multiplier */
	UPROPERTY(BlueprintReadWrite, Category="Sleep")
	float SleepQualityMultiplier = 1.0f;

	// ---- API ----

	/** Start sleeping */
	UFUNCTION(BlueprintCallable, Category="Sleep")
	void StartSleep();

	/** Stop sleeping (wake up) */
	UFUNCTION(BlueprintCallable, Category="Sleep")
	void StopSleep();

	/** Get fatigue as percentage */
	UFUNCTION(BlueprintPure, Category="Sleep")
	float GetFatiguePercent() const { return Fatigue / 100.0f; }

	/** Is the player exhausted? */
	UFUNCTION(BlueprintPure, Category="Sleep")
	bool IsExhausted() const { return Fatigue >= 95.0f; }

	/** Is the player tired (past penalty threshold)? */
	UFUNCTION(BlueprintPure, Category="Sleep")
	bool IsTired() const { return Fatigue >= FatiguePenaltyThreshold; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFellAsleep);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWokeUp);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExhausted);

	UPROPERTY(BlueprintAssignable, Category="Sleep")
	FOnFellAsleep OnFellAsleep;

	UPROPERTY(BlueprintAssignable, Category="Sleep")
	FOnWokeUp OnWokeUp;

	UPROPERTY(BlueprintAssignable, Category="Sleep")
	FOnExhausted OnExhausted;

private:
	bool bWasExhausted = false;
};
