// Copyright Biome Survivor. All Rights Reserved.

#include "Crafting/RecipeDatabase.h"
#include "BiomeSurvivor.h"

TMap<FName, UCraftingRecipe*> FRecipeDatabase::Recipes;
bool FRecipeDatabase::bInitialized = false;

UCraftingRecipe* FRecipeDatabase::RegisterRecipe(
	FName ID,
	const FText& Name,
	ECraftingCategory Category,
	const TArray<FCraftingIngredient>& Ingredients,
	FName OutputItemID,
	int32 OutputCount,
	float CraftTime,
	bool bNeedsUnlock)
{
	UCraftingRecipe* Recipe = NewObject<UCraftingRecipe>(GetTransientPackage(), *ID.ToString());
	Recipe->RecipeID = ID;
	Recipe->DisplayName = Name;
	Recipe->Category = Category;
	Recipe->Ingredients = Ingredients;
	Recipe->OutputItemID = OutputItemID;
	Recipe->OutputCount = OutputCount;
	Recipe->CraftingTime = CraftTime;
	Recipe->bRequiresUnlock = bNeedsUnlock;
	Recipe->RequiredCraftingLevel = 1;
	Recipe->ExperienceReward = 10.0f;
	Recipe->AddToRoot(); // Prevent GC

	Recipes.Add(ID, Recipe);
	return Recipe;
}

void FRecipeDatabase::Initialize()
{
	if (bInitialized) return;

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Initializing runtime recipe database..."));

	// ---- Helper lambda for ingredient lists ----
	auto Ing = [](FName ItemID, int32 Count) -> FCraftingIngredient
	{
		FCraftingIngredient I;
		I.ItemID = ItemID;
		I.Count = Count;
		return I;
	};

	// ============ BASIC / HANDCRAFT ============

	// Rope: 3 Fiber -> 1 Rope (2s)
	RegisterRecipe(
		FName("Craft_Rope"),
		FText::FromString(TEXT("Rope")),
		ECraftingCategory::Basic,
		{ Ing(FName("Fiber"), 3) },
		FName("Rope"), 1, 2.0f);

	// Bandage: 2 Fiber -> 1 Bandage (3s)
	RegisterRecipe(
		FName("Craft_Bandage"),
		FText::FromString(TEXT("Bandage")),
		ECraftingCategory::Basic,
		{ Ing(FName("Fiber"), 2) },
		FName("Bandage"), 1, 3.0f);

	// Flint: 2 Stone -> 1 Flint (2s)
	RegisterRecipe(
		FName("Craft_Flint"),
		FText::FromString(TEXT("Flint")),
		ECraftingCategory::Basic,
		{ Ing(FName("Stone"), 2) },
		FName("Flint"), 1, 2.0f);

	// Stone Axe: 3 Wood + 2 Stone + 1 Rope -> 1 StoneAxe (5s)
	RegisterRecipe(
		FName("Craft_StoneAxe"),
		FText::FromString(TEXT("Stone Axe")),
		ECraftingCategory::Basic,
		{ Ing(FName("Wood"), 3), Ing(FName("Stone"), 2), Ing(FName("Rope"), 1) },
		FName("StoneAxe"), 1, 5.0f);

	// Stone Pickaxe: 3 Wood + 3 Stone + 1 Rope -> 1 StonePickaxe (5s)
	RegisterRecipe(
		FName("Craft_StonePickaxe"),
		FText::FromString(TEXT("Stone Pickaxe")),
		ECraftingCategory::Basic,
		{ Ing(FName("Wood"), 3), Ing(FName("Stone"), 3), Ing(FName("Rope"), 1) },
		FName("StonePickaxe"), 1, 5.0f);

	// Bone Knife: 2 Bone + 1 Rope -> 1 BoneKnife (4s)
	RegisterRecipe(
		FName("Craft_BoneKnife"),
		FText::FromString(TEXT("Bone Knife")),
		ECraftingCategory::Basic,
		{ Ing(FName("Bone"), 2), Ing(FName("Rope"), 1) },
		FName("BoneKnife"), 1, 4.0f);

	// Torch: 1 Wood + 1 Fiber -> 1 Torch (1.5s)
	RegisterRecipe(
		FName("Craft_Torch"),
		FText::FromString(TEXT("Torch")),
		ECraftingCategory::Basic,
		{ Ing(FName("Wood"), 1), Ing(FName("Fiber"), 1) },
		FName("Torch"), 1, 1.5f);

	// Wooden Spear: 3 Wood + 1 Flint -> 1 WoodenSpear (4s)
	RegisterRecipe(
		FName("Craft_WoodenSpear"),
		FText::FromString(TEXT("Wooden Spear")),
		ECraftingCategory::Basic,
		{ Ing(FName("Wood"), 3), Ing(FName("Flint"), 1) },
		FName("WoodenSpear"), 1, 4.0f);

	// Wooden Shield: 5 Wood + 1 Rope -> 1 WoodenShield (6s)
	RegisterRecipe(
		FName("Craft_WoodenShield"),
		FText::FromString(TEXT("Wooden Shield")),
		ECraftingCategory::Basic,
		{ Ing(FName("Wood"), 5), Ing(FName("Rope"), 1) },
		FName("WoodenShield"), 1, 6.0f);

	// ============ COOKING FIRE ============

	// Cooked Meat: 1 RawMeat + 1 Wood -> 1 CookedMeat (4s)
	RegisterRecipe(
		FName("Craft_CookedMeat"),
		FText::FromString(TEXT("Cooked Meat")),
		ECraftingCategory::Cooking,
		{ Ing(FName("RawMeat"), 1), Ing(FName("Wood"), 1) },
		FName("CookedMeat"), 1, 4.0f);

	// Clean Water: 1 DirtyWater + 1 Wood -> 1 CleanWater (3s)
	RegisterRecipe(
		FName("Craft_CleanWater"),
		FText::FromString(TEXT("Clean Water")),
		ECraftingCategory::Cooking,
		{ Ing(FName("DirtyWater"), 1), Ing(FName("Wood"), 1) },
		FName("CleanWater"), 1, 3.0f);

	bInitialized = true;
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Recipe database initialized with %d recipes"), Recipes.Num());
}

const UCraftingRecipe* FRecipeDatabase::Get(FName RecipeID)
{
	if (!bInitialized) Initialize();
	if (UCraftingRecipe** Found = Recipes.Find(RecipeID))
	{
		return *Found;
	}
	return nullptr;
}

TArray<FName> FRecipeDatabase::GetAllRecipeIDs()
{
	if (!bInitialized) Initialize();
	TArray<FName> Result;
	Recipes.GetKeys(Result);
	return Result;
}

TArray<FName> FRecipeDatabase::GetRecipesByCategory(ECraftingCategory Category)
{
	if (!bInitialized) Initialize();
	TArray<FName> Result;
	for (const auto& Pair : Recipes)
	{
		if (Pair.Value && Pair.Value->Category == Category)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}
