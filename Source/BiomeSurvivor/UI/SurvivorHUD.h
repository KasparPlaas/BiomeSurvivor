// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SurvivorHUD.generated.h"

class USurvivorMainWidget;

/**
 * Main HUD class for the survival game.
 * Manages all UI widgets and their visibility states.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIVOR_API ASurvivorHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASurvivorHUD();

	virtual void BeginPlay() override;

	/** Widget class references (set in Blueprint subclass) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> MainWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> CraftingWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> MapWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> DeathScreenWidgetClass;

	/** Toggle specific UI panels */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleCrafting();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleMap();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDeathScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideAllMenus();

	/** Show a notification message on the HUD */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowNotification(const FText& Message, float Duration = 3.0f);

	/** Show an interaction prompt (e.g. "Press E to interact") */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInteractionPrompt(const FText& ActionText, const FText& ObjectName);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideInteractionPrompt();

	/** Check if any menu is currently open */
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool IsAnyMenuOpen() const;

protected:
	UPROPERTY()
	TObjectPtr<UUserWidget> MainWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> CraftingWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> MapWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> DeathScreenWidget;

private:
	void SetMenuMode(bool bMenuOpen);
};
