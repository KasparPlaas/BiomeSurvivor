// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Crafting/CraftingRecipe.h"
#include "CraftingStation.generated.h"

/**
 * ACraftingStation
 * World-placed or player-built workstation that filters recipes by category.
 * Players interact with the station to open the crafting UI for that category.
 */

UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIVOR_API ACraftingStation : public AActor
{
	GENERATED_BODY()

public:
	ACraftingStation();

	/** The category of recipes this station enables */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CraftingStation")
	ECraftingCategory StationType = ECraftingCategory::Workbench;

	/** Display name shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CraftingStation")
	FText StationName;

	/** Mesh component for visual representation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CraftingStation")
	UStaticMeshComponent* MeshComponent;

	/** Interaction range in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CraftingStation")
	float InteractionRange = 300.0f;

	/** Fuel-based stations: current fuel level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CraftingStation")
	float FuelLevel = 0.0f;

	/** Does this station require fuel to operate? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CraftingStation")
	bool bRequiresFuel = false;

	/** Is the station currently active/lit? */
	UPROPERTY(BlueprintReadOnly, Category="CraftingStation")
	bool bIsActive = false;

	/** Called when a player interacts with this station */
	UFUNCTION(BlueprintCallable, Category="CraftingStation")
	void OnInteract(AActor* Interactor);

	/** Add fuel to the station */
	UFUNCTION(BlueprintCallable, Category="CraftingStation")
	void AddFuel(float Amount);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
