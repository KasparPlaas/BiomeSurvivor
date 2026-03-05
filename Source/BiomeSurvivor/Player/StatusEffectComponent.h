// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusEffectComponent.generated.h"

/**
 * UStatusEffectComponent
 * Manages active status effects: Hypothermia, Hyperthermia, Bleeding,
 * Fracture, Poisoning, Infection, Starvation, Dehydration.
 * Each effect has severity (0-100) and applies penalties/damage over time.
 * Values from plan.md Section 5.1.3.
 */

UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
	None			UMETA(DisplayName = "None"),
	Hypothermia		UMETA(DisplayName = "Hypothermia"),
	Hyperthermia	UMETA(DisplayName = "Hyperthermia"),
	Bleeding		UMETA(DisplayName = "Bleeding"),
	Fracture		UMETA(DisplayName = "Fracture"),
	Poisoning		UMETA(DisplayName = "Poisoning"),
	Infection		UMETA(DisplayName = "Infection"),
	Starvation		UMETA(DisplayName = "Starvation"),
	Dehydration		UMETA(DisplayName = "Dehydration")
};

USTRUCT(BlueprintType)
struct FStatusEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStatusEffectType Type = EStatusEffectType::None;

	/** Severity 0-100. Higher = worse effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="100"))
	float Severity = 0.0f;

	/** How fast severity increases/decreases per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProgressionRate = 1.0f;

	/** Damage per second at severity 100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDamagePerSecond = 2.0f;

	/** Movement speed multiplier at max severity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedPenaltyMultiplier = 0.5f;

	/** Duration remaining (-1 = until cured) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = -1.0f;

	/** Whether this effect is currently active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusEffectChanged, EStatusEffectType, Type, float, Severity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectRemoved, EStatusEffectType, Type);

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Apply a new status effect or increase severity of existing one */
	UFUNCTION(BlueprintCallable, Category="StatusEffects")
	void ApplyStatusEffect(EStatusEffectType Type, float InitialSeverity = 10.0f, float Duration = -1.0f);

	/** Remove a status effect completely */
	UFUNCTION(BlueprintCallable, Category="StatusEffects")
	void RemoveStatusEffect(EStatusEffectType Type);

	/** Reduce severity of a specific effect (e.g., from medicine) */
	UFUNCTION(BlueprintCallable, Category="StatusEffects")
	void ReduceEffectSeverity(EStatusEffectType Type, float Amount);

	/** Check if a specific effect is active */
	UFUNCTION(BlueprintPure, Category="StatusEffects")
	bool HasStatusEffect(EStatusEffectType Type) const;

	/** Get severity of a specific effect */
	UFUNCTION(BlueprintPure, Category="StatusEffects")
	float GetEffectSeverity(EStatusEffectType Type) const;

	/** Get the total movement speed penalty from all effects (0.0 to 1.0 multiplier) */
	UFUNCTION(BlueprintPure, Category="StatusEffects")
	float GetCombinedSpeedMultiplier() const;

	/** Get all active effects */
	UFUNCTION(BlueprintPure, Category="StatusEffects")
	TArray<FStatusEffect> GetActiveEffects() const;

	/** Remove all status effects */
	UFUNCTION(BlueprintCallable, Category="StatusEffects")
	void ClearAllEffects();

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category="StatusEffects")
	FOnStatusEffectChanged OnStatusEffectChanged;

	UPROPERTY(BlueprintAssignable, Category="StatusEffects")
	FOnStatusEffectRemoved OnStatusEffectRemoved;

private:
	UPROPERTY()
	TMap<EStatusEffectType, FStatusEffect> ActiveEffects;

	/** Default configurations for each effect type */
	FStatusEffect GetDefaultEffect(EStatusEffectType Type) const;

	/** Apply tick damage from a status effect */
	void ProcessEffectTick(FStatusEffect& Effect, float DeltaTime);
};
