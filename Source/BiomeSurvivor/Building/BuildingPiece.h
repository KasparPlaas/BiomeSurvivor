// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingPiece.generated.h"

/**
 * ABuildingPiece
 * Base class for all placeable building pieces (walls, floors, roofs, doors, stairs, etc.).
 * Supports snapping, health/destruction, and material tiers.
 * Values from plan.md Section 5.4.
 */

UENUM(BlueprintType)
enum class EBuildingPieceType : uint8
{
	Foundation	UMETA(DisplayName = "Foundation"),
	Wall		UMETA(DisplayName = "Wall"),
	Floor		UMETA(DisplayName = "Floor"),
	Roof		UMETA(DisplayName = "Roof"),
	Door		UMETA(DisplayName = "Door"),
	Window		UMETA(DisplayName = "Window"),
	Stairs		UMETA(DisplayName = "Stairs"),
	Ramp		UMETA(DisplayName = "Ramp"),
	Fence		UMETA(DisplayName = "Fence"),
	Pillar		UMETA(DisplayName = "Pillar"),
	Furniture	UMETA(DisplayName = "Furniture")
};

UENUM(BlueprintType)
enum class EBuildingMaterial : uint8
{
	Wood		UMETA(DisplayName = "Wood"),
	Stone		UMETA(DisplayName = "Stone"),
	Metal		UMETA(DisplayName = "Metal"),
	Thatch		UMETA(DisplayName = "Thatch")
};

USTRUCT(BlueprintType)
struct FSnapPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator LocalRotation = FRotator::ZeroRotator;

	/** Which piece types can snap to this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EBuildingPieceType> CompatibleTypes;
};

UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API ABuildingPiece : public AActor
{
	GENERATED_BODY()

public:
	ABuildingPiece();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	EBuildingPieceType PieceType = EBuildingPieceType::Wall;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	EBuildingMaterial Material = EBuildingMaterial::Wood;

	/** Current health of this piece */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Building")
	float Health = 100.0f;

	/** Max health depends on material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	float MaxHealth = 100.0f;

	/** Snap points where other pieces can attach */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	TArray<FSnapPoint> SnapPoints;

	/** Owner player's ID (for protection/permissions) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Building")
	FString OwnerPlayerID;

	/** Grid size for snapping (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	float GridSize = 300.0f;

	/** Is in preview/placement mode (translucent, no collision) */
	UPROPERTY(BlueprintReadWrite, Category="Building")
	bool bIsPreview = false;

	/** Can this piece be placed at its current location? */
	UPROPERTY(BlueprintReadOnly, Category="Building")
	bool bValidPlacement = false;

	// ---- API ----

	UFUNCTION(BlueprintCallable, Category="Building")
	void ApplyDamage(float DamageAmount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category="Building")
	void Repair(float Amount);

	UFUNCTION(BlueprintCallable, Category="Building")
	void SetPreviewMode(bool bPreview);

	UFUNCTION(BlueprintCallable, Category="Building")
	void ConfirmPlacement(const FString& PlayerID);

	/** Snap to nearest valid snap point on nearby pieces */
	UFUNCTION(BlueprintCallable, Category="Building")
	bool TrySnapToNearby(float SearchRadius = 400.0f);

	/** Set health based on material tier */
	UFUNCTION(BlueprintCallable, Category="Building")
	void SetMaterial(EBuildingMaterial NewMaterial);

	UFUNCTION(BlueprintPure, Category="Building")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingDamaged, float, RemainingHealth);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuildingDestroyed);

	UPROPERTY(BlueprintAssignable, Category="Building")
	FOnBuildingDamaged OnBuildingDamaged;

	UPROPERTY(BlueprintAssignable, Category="Building")
	FOnBuildingDestroyed OnBuildingDestroyed;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
