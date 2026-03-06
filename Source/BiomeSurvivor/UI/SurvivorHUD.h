// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SurvivorHUD.generated.h"

class UPlayerStatsComponent;
class UInventoryComponent;

/**
 * ASurvivorHUD
 *
 * Full game HUD with:
 * - Canvas-drawn gameplay HUD (stat bars, crosshair, compass, interaction prompts)
 * - Slate main menu overlay
 * - Slate pause menu overlay
 * - Death screen overlay
 *
 * All rendered programmatically - no Blueprint widget dependencies.
 */
UCLASS()
class BIOMESURVIVOR_API ASurvivorHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASurvivorHUD();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void DrawHUD() override;
	virtual void Tick(float DeltaTime) override;

	// ---- Menu Control ----

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideMainMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDeathScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideDeathScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideAllMenus();

	// ---- Inventory ----

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideInventory();

	// ---- HUD Prompts ----

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInteractionPrompt(const FText& ActionText, const FText& ObjectName);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowNotification(const FText& Message, float Duration = 3.0f);

	// ---- State Queries ----

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsAnyMenuOpen() const;

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsMainMenuVisible() const { return bMainMenuVisible; }

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsInventoryVisible() const { return bInventoryVisible; }

	// ---- State ----
	bool bMainMenuVisible = true;
	bool bPauseMenuVisible = false;
	bool bDeathScreenVisible = false;
	bool bInventoryVisible = false;

protected:
	// ---- Canvas HUD Drawing ----
	void DrawStatBars();
	void DrawCrosshair();
	void DrawInteractionPromptHUD();
	void DrawNotifications();
	void DrawCompass();
	void DrawDeathOverlay();
	void DrawQuickBar();
	void DrawInventoryGrid();

	/** Draw a single horizontal stat bar with label and value text. */
	void DrawStatBar(float X, float Y, float Width, float Height, float Percent,
		const FLinearColor& BarColor, const FString& Label, const FString& ValueText);

	/** Retrieve the player's stats component. */
	UPlayerStatsComponent* GetPlayerStats() const;

	/** Retrieve the player's inventory component. */
	UInventoryComponent* GetPlayerInventory() const;

	// ---- Interaction Prompt ----
	bool bShowInteraction = false;
	FText InteractActionText;
	FText InteractObjectText;

	// ---- Notifications ----
	struct FHUDNotification
	{
		FText Message;
		float TimeRemaining;
	};
	TArray<FHUDNotification> ActiveNotifications;

	// ---- Smoothed Display Values ----
	float DisplayHealth = 1.0f;
	float DisplayHunger = 1.0f;
	float DisplayThirst = 1.0f;
	float DisplayStamina = 1.0f;

	// ---- Slate Menus ----
	TSharedPtr<SWidget> MainMenuOverlay;
	TSharedPtr<SWidget> PauseMenuOverlay;
	TSharedPtr<SWidget> InventoryOverlay;

	void CreateMainMenuSlate();
	void CreatePauseMenuSlate();
	void CreateInventorySlate();
	void RefreshInventorySlate();
};
