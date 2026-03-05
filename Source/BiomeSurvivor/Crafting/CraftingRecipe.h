// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CraftingRecipe.generated.h"

/**
 * Crafting recipe data structures and data asset.
 * Values from plan.md Section 5.3.
 */

UENUM(BlueprintType)
enum class ECraftingCategory : uint8
{
	Basic		UMETA(DisplayName = "Basic / Handcraft"),
	Workbench	UMETA(DisplayName = "Workbench"),
	Forge		UMETA(DisplayName = "Forge"),
	Tannery		UMETA(DisplayName = "Tannery"),
	Cooking		UMETA(DisplayName = "Cooking Fire"),
	Alchemy		UMETA(DisplayName = "Alchemy Table"),
	Advanced	UMETA(DisplayName = "Advanced Workbench")
};

USTRUCT(BlueprintType)
struct FCraftingIngredient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;
};

/**
 * UCraftingRecipe - Primary Data Asset
 * Defines a single crafting recipe with ingredients, output, and requirements.
 */
UCLASS(BlueprintType)
class BIOMESURVIOR_API UCraftingRecipe : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique recipe ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName RecipeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FText DisplayName;

	/** Category / station required */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	ECraftingCategory Category = ECraftingCategory::Basic;

	/** Required ingredients */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	TArray<FCraftingIngredient> Ingredients;

	/** Output item ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName OutputItemID;

	/** Output count */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 OutputCount = 1;

	/** Crafting time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	float CraftingTime = 2.0f;

	/** Minimum skill level required (Crafting skill) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 RequiredCraftingLevel = 1;

	/** Experience gained upon crafting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	float ExperienceReward = 10.0f;

	/** Does this recipe need to be discovered/unlocked? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	bool bRequiresUnlock = false;

	/** Recipe icon for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	TSoftObjectPtr<UTexture2D> Icon;

	// UPrimaryDataAsset override
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("CraftingRecipe", RecipeID);
	}
};
