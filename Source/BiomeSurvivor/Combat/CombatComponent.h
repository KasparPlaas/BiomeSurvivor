// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

/**
 * UCombatComponent
 * Handles melee/ranged attacks, blocking, dodging, lock-on, and damage calculation.
 * Values from plan.md Section 5.5.
 */

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Blocking	UMETA(DisplayName = "Blocking"),
	Dodging		UMETA(DisplayName = "Dodging"),
	Staggered	UMETA(DisplayName = "Staggered"),
	Dead		UMETA(DisplayName = "Dead")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageDealt, AActor*, Target, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, ECombatState, NewState);

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIVOR_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ---- Combat State ----

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	ECombatState CurrentState = ECombatState::Idle;

	// ---- Attack ----

	/** Perform a light attack */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void TryLightAttack();

	/** Perform a heavy attack (hold) */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void TryHeavyAttack();

	/** Perform a ranged attack (shoot) */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void TryRangedAttack();

	// ---- Block & Dodge ----

	UFUNCTION(BlueprintCallable, Category="Combat")
	void StartBlock();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void StopBlock();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void PerformDodge();

	// ---- Lock-On ----

	UFUNCTION(BlueprintCallable, Category="Combat")
	void ToggleLockOn();

	UFUNCTION(BlueprintPure, Category="Combat")
	AActor* GetLockedTarget() const { return LockedTarget; }

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsLockedOn() const { return LockedTarget != nullptr; }

	// ---- Damage Calculation ----

	/** Calculate final damage considering weapon, armor, and effects */
	UFUNCTION(BlueprintPure, Category="Combat")
	float CalculateDamage(float BaseDamage, AActor* Target) const;

	/** Apply damage to a target actor */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void ApplyDamageToTarget(AActor* Target, float BaseDamage, TSubclassOf<UDamageType> DamageType = nullptr);

	// ---- Stats ----

	/** Light attack stamina cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float LightAttackStaminaCost = 8.0f;

	/** Heavy attack stamina cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float HeavyAttackStaminaCost = 20.0f;

	/** Block damage reduction (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float BlockDamageReduction = 0.7f;

	/** Dodge i-frame duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float DodgeIFrameDuration = 0.3f;

	/** Lock-on max distance in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float LockOnRange = 1500.0f;

	/** Attack cooldown (seconds between attacks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackCooldown = 0.5f;

	/** Melee attack range in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float MeleeRange = 200.0f;

	/** Melee sweep radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float MeleeSweepRadius = 60.0f;

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnDamageDealt OnDamageDealt;

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnCombatStateChanged OnCombatStateChanged;

	// ---- Server RPCs ----
	UFUNCTION(Server, Reliable)
	void ServerLightAttack();

	UFUNCTION(Server, Reliable)
	void ServerHeavyAttack();

private:
	UPROPERTY()
	AActor* LockedTarget = nullptr;

	float AttackCooldownTimer = 0.0f;
	float DodgeTimer = 0.0f;
	bool bIsInvincible = false; // i-frames during dodge

	FTimerHandle LightAttackTimerHandle;
	FTimerHandle HeavyAttackTimerHandle;

	void SetCombatState(ECombatState NewState);
	void PerformMeleeSweep(float DamageMultiplier);
	AActor* FindLockOnTarget() const;

	UFUNCTION()
	void OnAttackAnimFinished();
};
