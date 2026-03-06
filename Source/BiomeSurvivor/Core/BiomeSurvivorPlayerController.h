// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BiomeSurvivorPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * ABiomeSurvivorPlayerController
 *
 * Handles input binding, UI management, and client-side logic.
 * Uses Enhanced Input System for all player input.
 */
UCLASS(Blueprintable)
class BIOMESURVIVOR_API ABiomeSurvivorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABiomeSurvivorPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// ---- UI Control ----

	/** Toggle inventory screen. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleInventory();

	/** Toggle crafting menu. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleCraftingMenu();

	/** Toggle build mode. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleBuildMode();

	/** Toggle map. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleMap();

	/** Toggle pause menu. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	/** Is any menu currently open? */
	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsAnyMenuOpen() const;

	// ---- Enhanced Input ----

	/** Default input mapping context for gameplay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Input mapping for menus/UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MenuMappingContext;

protected:
	/** Track which menus are open. */
	bool bInventoryOpen = false;
	bool bCraftingOpen = false;
	bool bBuildModeActive = false;
	bool bMapOpen = false;
	bool bPauseMenuOpen = false;

	/** Set mouse mode for gameplay vs menus. */
	void SetGameplayInputMode();
	void SetUIInputMode();
};
