// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CraftingWidget.generated.h"

class UCraftingComponent;
class UInventoryComponent;
class UCraftingRecipe;
class UScrollBox;
class UProgressBar;

/**
 * Represents a single craftable recipe entry in the crafting list
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIVOR_API UCraftingRecipeEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set the recipe this entry displays */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void SetRecipe(UCraftingRecipe* InRecipe);

	/** Update whether the player can craft this (materials/level check) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
	void UpdateCraftability(bool bCanCraft);

	/** Update visual state (selected, highlighted) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
	void SetSelected(bool bSelected);

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<UCraftingRecipe> Recipe;
};

/**
 * Main crafting widget showing:
 * - List of available/discovered recipes (filtered by station type)
 * - Selected recipe details (ingredients, output, time)
 * - Crafting queue with progress bars
 * - Category filters
 *
 * Designed for Blueprint subclassing with UMG layout.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIVOR_API UCraftingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Initialize with component references */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void InitializeCrafting(UCraftingComponent* InCrafting, UInventoryComponent* InInventory);

	/** Refresh the recipe list based on current station and filters */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RefreshRecipeList();

	/** Select a recipe to show details */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void SelectRecipe(UCraftingRecipe* Recipe);

	/** Start crafting the currently selected recipe */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void CraftSelected();

	/** Cancel a queued crafting item */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void CancelQueueItem(int32 QueueIndex);

	/** Set category filter */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void SetCategoryFilter(uint8 Category);

	/** Get crafting progress (0-1) for current item */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	float GetCurrentCraftingProgress() const;

	/** Get formatted crafting time text */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	FText GetCraftingTimeText() const;

	/** Get the number of items in the queue */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	int32 GetQueueCount() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<UCraftingComponent> CraftingComp;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UCraftingRecipe> SelectedRecipe;

	/** Recipe entry widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TSubclassOf<UCraftingRecipeEntry> RecipeEntryClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TArray<UCraftingRecipeEntry*> RecipeEntries;

	/** Active category filter. 255 = show all */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	uint8 ActiveCategoryFilter = 255;
};
