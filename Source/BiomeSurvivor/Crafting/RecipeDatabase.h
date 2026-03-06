// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Crafting/CraftingRecipe.h"

/**
 * FRecipeDatabase
 * Runtime crafting recipe registry — mirrors the FItemDatabase pattern.
 * Since we can't create .uasset CraftingRecipe data assets,
 * all recipes are registered here in C++ code.
 */
class BIOMESURVIVOR_API FRecipeDatabase
{
public:
	/** Initialize all built-in recipes. Safe to call multiple times. */
	static void Initialize();

	/** Get a recipe by ID. Returns nullptr if not found. */
	static const UCraftingRecipe* Get(FName RecipeID);

	/** Get all registered recipe IDs. */
	static TArray<FName> GetAllRecipeIDs();

	/** Get recipe IDs filtered by category. */
	static TArray<FName> GetRecipesByCategory(ECraftingCategory Category);

	/** Check if initialized */
	static bool IsInitialized() { return bInitialized; }

private:
	/** Register a handcraft recipe helper. */
	static UCraftingRecipe* RegisterRecipe(
		FName ID,
		const FText& Name,
		ECraftingCategory Category,
		const TArray<FCraftingIngredient>& Ingredients,
		FName OutputItemID,
		int32 OutputCount,
		float CraftTime,
		bool bNeedsUnlock = false);

	static TMap<FName, UCraftingRecipe*> Recipes;
	static bool bInitialized;
};
