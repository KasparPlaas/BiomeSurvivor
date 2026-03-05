// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceNode.generated.h"

class UStaticMeshComponent;
class USphereComponent;

/**
 * Resource types that can be gathered from world nodes
 */
UENUM(BlueprintType)
enum class EResourceNodeType : uint8
{
	Tree		UMETA(DisplayName = "Tree"),
	Rock		UMETA(DisplayName = "Rock"),
	OreDeposit	UMETA(DisplayName = "Ore Deposit"),
	BerryBush	UMETA(DisplayName = "Berry Bush"),
	HerbPlant	UMETA(DisplayName = "Herb Plant"),
	WaterSource UMETA(DisplayName = "Water Source"),
	FiberPlant	UMETA(DisplayName = "Fiber Plant"),
	CattailReed UMETA(DisplayName = "Cattail Reed")
};

/**
 * Item gained from harvesting a resource node
 */
USTRUCT(BlueprintType)
struct FResourceYield
{
	GENERATED_BODY()

	/** Item ID to give */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	/** Base quantity per harvest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 BaseQuantity = 1;

	/** Max extra random quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 BonusQuantity = 0;

	/** Chance to get this item (0-1), for secondary drops */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float DropChance = 1.0f;

	/** Minimum gathering skill level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredSkillLevel = 0;
};

/**
 * A harvestable resource node placed in the world.
 * Players interact with these to gather resources.
 * Supports respawning after depletion.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API AResourceNode : public AActor
{
	GENERATED_BODY()

public:
	AResourceNode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ResourceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	// --- Properties ---

	/** Type of resource */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	EResourceNodeType ResourceType = EResourceNodeType::Tree;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	FText ResourceName;

	/** Current hitpoints - deplete to harvest */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Resource")
	float CurrentHealth;

	/** Maximum hitpoints */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	float MaxHealth = 100.0f;

	/** Items yielded on each hit */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	TArray<FResourceYield> HarvestYields;

	/** Items yielded when fully depleted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	TArray<FResourceYield> DepletionBonusYields;

	/** Required tool type to harvest (empty = any tool / bare hands) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	FName RequiredToolType;

	/** Damage multiplier for correct tool type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	float CorrectToolMultiplier = 2.0f;

	/** Respawn time in seconds after depletion. 0 = no respawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respawn")
	float RespawnTimeSeconds = 600.0f;

	/** Whether the node is currently depleted */
	UPROPERTY(ReplicatedUsing = OnRep_IsDepleted, BlueprintReadOnly, Category = "Resource")
	bool bIsDepleted = false;

	/** Gathering XP reward per hit */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	float GatheringXPPerHit = 5.0f;

	/** Biomes this resource can appear in */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<FName> ValidBiomes;

	// --- Interface ---

	/**
	 * Harvest the resource node. Called when a player hits it with a tool.
	 * @param Harvester - The player character
	 * @param Damage - Amount of damage to deal
	 * @param ToolType - Name of the tool being used
	 * @return True if resources were yielded
	 */
	UFUNCTION(BlueprintCallable, Category = "Resource")
	bool HarvestNode(AActor* Harvester, float Damage, FName ToolType);

	/** Get current health as percentage */
	UFUNCTION(BlueprintCallable, Category = "Resource")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	/** Is this resource available for harvesting? */
	UFUNCTION(BlueprintCallable, Category = "Resource")
	bool IsAvailable() const { return !bIsDepleted && CurrentHealth > 0.0f; }

private:
	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_IsDepleted();

	void DepleteResource();
	void RespawnResource();
	void GiveYields(AActor* Harvester, const TArray<FResourceYield>& Yields);

	FTimerHandle RespawnTimerHandle;
	FVector OriginalScale;
};
