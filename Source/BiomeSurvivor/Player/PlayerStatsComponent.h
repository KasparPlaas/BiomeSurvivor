// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatsComponent.generated.h"

/**
 * UPlayerStatsComponent
 * Manages all core survival stats: Health, Hunger, Thirst, Stamina, Comfort.
 * Stat drain rates are tuned from plan.md Section 5.1.2.
 * Replicated for multiplayer. Tick-driven stat decay.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatDepleted);

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStatsComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Health ----
	UPROPERTY(ReplicatedUsing=OnRep_Health, EditAnywhere, BlueprintReadWrite, Category="Stats|Health")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Health")
	float HealthRegenRate = 0.5f; // HP/sec when well-fed and warm

	// ---- Hunger ----
	UPROPERTY(ReplicatedUsing=OnRep_Hunger, EditAnywhere, BlueprintReadWrite, Category="Stats|Hunger")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Hunger")
	float MaxHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Hunger")
	float HungerDrainRate = 1.0f / 60.0f; // 1.0 per minute => ~0.0167/sec

	// ---- Thirst ----
	UPROPERTY(ReplicatedUsing=OnRep_Thirst, EditAnywhere, BlueprintReadWrite, Category="Stats|Thirst")
	float Thirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Thirst")
	float MaxThirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Thirst")
	float ThirstDrainRate = 1.5f / 60.0f; // 1.5 per minute => ~0.025/sec

	// ---- Stamina ----
	UPROPERTY(ReplicatedUsing=OnRep_Stamina, EditAnywhere, BlueprintReadWrite, Category="Stats|Stamina")
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Stamina")
	float StaminaRegenRate = 8.0f; // per second when not sprinting

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Stamina")
	float SprintStaminaDrain = 12.0f; // per second while sprinting

	// ---- Comfort ----
	UPROPERTY(ReplicatedUsing=OnRep_Comfort, EditAnywhere, BlueprintReadWrite, Category="Stats|Comfort")
	float Comfort = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Comfort")
	float MaxComfort = 100.0f;

	// ---- Weight ----
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Stats|Weight")
	float CurrentWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Weight")
	float MaxWeight = 40.0f; // kg base capacity

	// ---- API ----
	UFUNCTION(BlueprintCallable, Category="Stats")
	void ApplyDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category="Stats")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category="Stats")
	void ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category="Stats")
	void RestoreHunger(float Amount);

	UFUNCTION(BlueprintCallable, Category="Stats")
	void RestoreThirst(float Amount);

	UFUNCTION(BlueprintCallable, Category="Stats")
	void RestoreComfort(float Amount);

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetHunger() const { return Hunger; }

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetThirst() const { return Thirst; }

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetComfort() const { return Comfort; }

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetHungerPercent() const { return MaxHunger > 0.f ? Hunger / MaxHunger : 0.f; }

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetThirstPercent() const { return MaxThirst > 0.f ? Thirst / MaxThirst : 0.f; }

	UFUNCTION(BlueprintPure, Category="Stats")
	float GetStaminaPercent() const { return MaxStamina > 0.f ? Stamina / MaxStamina : 0.f; }

	UFUNCTION(BlueprintPure, Category="Stats")
	bool IsOverEncumbered() const { return CurrentWeight > MaxWeight; }

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category="Stats")
	FOnStatChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="Stats")
	FOnStatChanged OnHungerChanged;

	UPROPERTY(BlueprintAssignable, Category="Stats")
	FOnStatChanged OnThirstChanged;

	UPROPERTY(BlueprintAssignable, Category="Stats")
	FOnStatChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category="Stats")
	FOnStatDepleted OnHealthDepleted;

	UPROPERTY(BlueprintAssignable, Category="Stats")
	FOnStatDepleted OnHungerDepleted;

	UPROPERTY(BlueprintAssignable, Category="Stats")
	FOnStatDepleted OnThirstDepleted;

private:
	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_Hunger();

	UFUNCTION()
	void OnRep_Thirst();

	UFUNCTION()
	void OnRep_Stamina();

	UFUNCTION()
	void OnRep_Comfort();

	/** Whether the player is currently sprinting (set by character) */
	bool bSprinting = false;
	friend class ASurvivorCharacter;
};
