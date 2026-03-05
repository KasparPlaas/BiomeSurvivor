// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimalBase.generated.h"

/**
 * AAnimalBase
 * Base class for all wildlife. Supports passive/aggressive AI behaviors,
 * loot drops, biome-specific spawning, and behavior trees.
 * Values from plan.md Section 5.7.
 */

UENUM(BlueprintType)
enum class EAnimalBehavior : uint8
{
	Passive		UMETA(DisplayName = "Passive"),       // Deer, rabbit - flee from player
	Neutral		UMETA(DisplayName = "Neutral"),       // Bear - attacks only when provoked
	Aggressive	UMETA(DisplayName = "Aggressive"),    // Wolf, cougar - hunts player
	Territorial	UMETA(DisplayName = "Territorial")    // Moose - aggressive if too close
};

UENUM(BlueprintType)
enum class EAnimalState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Wandering	UMETA(DisplayName = "Wandering"),
	Feeding		UMETA(DisplayName = "Feeding"),
	Sleeping	UMETA(DisplayName = "Sleeping"),
	Fleeing		UMETA(DisplayName = "Fleeing"),
	Stalking	UMETA(DisplayName = "Stalking"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Dying		UMETA(DisplayName = "Dying")
};

USTRUCT(BlueprintType)
struct FAnimalLootDrop
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCount = 1;

	/** Drop chance (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="1"))
	float DropChance = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API AAnimalBase : public ACharacter
{
	GENERATED_BODY()

public:
	AAnimalBase();

	// ---- Identity ----

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal")
	FText AnimalName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal")
	FName AnimalID;

	// ---- Behavior ----

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|AI")
	EAnimalBehavior BehaviorType = EAnimalBehavior::Passive;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Animal|AI")
	EAnimalState CurrentState = EAnimalState::Idle;

	/** Detection radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animal|AI")
	float DetectionRadius = 2000.0f;

	/** Flee distance in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animal|AI")
	float FleeDistance = 3000.0f;

	/** Attack range in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animal|AI")
	float AttackRange = 200.0f;

	/** Wander radius from spawn point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animal|AI")
	float WanderRadius = 1500.0f;

	// ---- Stats ----

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Animal|Stats")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Stats")
	float AttackDamage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Stats")
	float WalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Stats")
	float RunSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Stats")
	float AttackCooldown = 2.0f;

	// ---- Loot ----

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Loot")
	TArray<FAnimalLootDrop> LootTable;

	/** Can this animal be harvested after death? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Loot")
	bool bHarvestable = true;

	/** Time (seconds) before corpse despawns */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Loot")
	float CorpseDespawnTime = 300.0f;

	// ---- Biome ----

	/** Which biomes this animal spawns in */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Spawning")
	TArray<FName> ValidBiomes;

	/** Spawn weight (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animal|Spawning")
	float SpawnWeight = 1.0f;

	// ---- API ----

	UFUNCTION(BlueprintCallable, Category="Animal")
	void SetState(EAnimalState NewState);

	UFUNCTION(BlueprintCallable, Category="Animal")
	void OnDamaged(float DamageAmount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category="Animal")
	TArray<FAnimalLootDrop> GenerateLoot() const;

	UFUNCTION(BlueprintCallable, Category="Animal")
	void Harvest(AActor* Harvester);

	UFUNCTION(BlueprintPure, Category="Animal")
	bool IsDead() const { return Health <= 0.0f; }

	UFUNCTION(BlueprintPure, Category="Animal")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimalStateChanged, EAnimalState, NewState);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnimalDied);

	UPROPERTY(BlueprintAssignable, Category="Animal")
	FOnAnimalStateChanged OnAnimalStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Animal")
	FOnAnimalDied OnAnimalDied;

protected:
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FVector SpawnLocation;
	float AttackCooldownTimer = 0.0f;
	bool bHasBeenHarvested = false;
};
