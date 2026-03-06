// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * AWorldPickup
 *
 * Represents a dropped item in the world. Players can pick it up by
 * pressing E (interact) or walking over it (overlap auto-pickup).
 * Spawned when items are dropped from inventory or when a player dies.
 */
UCLASS(Blueprintable)
class BIOMESURVIVOR_API AWorldPickup : public AActor
{
	GENERATED_BODY()

public:
	AWorldPickup();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ---- Pickup Data ----

	/** The item ID this pickup represents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Pickup")
	FName ItemID;

	/** Stack count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Pickup")
	int32 ItemCount = 1;

	/** Time in seconds before this pickup despawns (0 = never) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float DespawnTime = 300.0f;

	/** Whether this pickup can be auto-collected on overlap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bAutoPickup = true;

	// ---- API ----

	/** Try to pick up this item. Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	bool PickUp(AActor* Collector);

	/** Static helper to spawn a pickup in the world */
	static AWorldPickup* SpawnPickup(UWorld* World, const FVector& Location, FName InItemID, int32 Count);

	// ---- Components ----

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> AutoPickupRadius;

protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Bobbing animation timer */
	float BobTimer = 0.0f;
	FVector SpawnLocation;

	/** Has been picked up (prevent double-collect) */
	bool bCollected = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Apply a color to the pickup mesh based on item type */
	void ApplyItemColor();
};
