// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Building/BuildingPiece.h"
#include "BuildingComponent.generated.h"

/**
 * UBuildingComponent
 * Player's building controller. Manages preview ghost, placement validation,
 * rotation, snap-to-grid, and demolition.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingPlaced, ABuildingPiece*, Piece);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuildModeToggled);

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API UBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ---- Build Mode ----

	/** Toggle build mode on/off */
	UFUNCTION(BlueprintCallable, Category="Building")
	void ToggleBuildMode();

	/** Is build mode currently active? */
	UFUNCTION(BlueprintPure, Category="Building")
	bool IsBuildModeActive() const { return bBuildModeActive; }

	/** Select a building piece type to place */
	UFUNCTION(BlueprintCallable, Category="Building")
	void SelectBuildingPiece(TSubclassOf<ABuildingPiece> PieceClass);

	/** Rotate the preview piece */
	UFUNCTION(BlueprintCallable, Category="Building")
	void RotatePreview(float YawDelta);

	/** Confirm placement of the current preview */
	UFUNCTION(BlueprintCallable, Category="Building")
	bool PlaceBuilding();

	/** Cancel current preview */
	UFUNCTION(BlueprintCallable, Category="Building")
	void CancelPreview();

	/** Demolish a building piece the player is looking at */
	UFUNCTION(BlueprintCallable, Category="Building")
	bool DemolishTarget();

	/** Get the current preview piece */
	UFUNCTION(BlueprintPure, Category="Building")
	ABuildingPiece* GetPreviewPiece() const { return PreviewPiece; }

	// ---- Configuration ----

	/** Max placement distance from player in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	float MaxPlaceDistance = 800.0f;

	/** Valid (green) and invalid (red) placement materials */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	UMaterialInterface* ValidPlacementMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	UMaterialInterface* InvalidPlacementMaterial;

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category="Building")
	FOnBuildingPlaced OnBuildingPlaced;

	UPROPERTY(BlueprintAssignable, Category="Building")
	FOnBuildModeToggled OnBuildModeToggled;

	// ---- Server RPCs ----
	UFUNCTION(Server, Reliable)
	void ServerPlaceBuilding(TSubclassOf<ABuildingPiece> PieceClass, FVector Location, FRotator Rotation);

	UFUNCTION(Server, Reliable)
	void ServerDemolishPiece(ABuildingPiece* Piece);

private:
	bool bBuildModeActive = false;

	UPROPERTY()
	ABuildingPiece* PreviewPiece = nullptr;

	UPROPERTY()
	TSubclassOf<ABuildingPiece> SelectedPieceClass;

	float PreviewYawRotation = 0.0f;

	/** Update preview ghost position based on player view */
	void UpdatePreviewPosition();

	/** Check if current placement is valid */
	bool IsPlacementValid() const;

	/** Get the placement location from a line trace */
	bool GetPlacementTransform(FVector& OutLocation, FRotator& OutRotation) const;
};
