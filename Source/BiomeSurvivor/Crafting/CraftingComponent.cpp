// Copyright Biome Survivor. All Rights Reserved.

#include "Crafting/CraftingComponent.h"
#include "Crafting/RecipeDatabase.h"
#include "Inventory/InventoryComponent.h"
#include "BiomeSurvivor.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 ticks/sec for smooth progress
	SetIsReplicatedByDefault(true);
}

void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize runtime recipe database
	FRecipeDatabase::Initialize();

	// Auto-learn all basic (non-unlock) recipes
	TArray<FName> AllRecipes = FRecipeDatabase::GetAllRecipeIDs();
	for (const FName& ID : AllRecipes)
	{
		const UCraftingRecipe* Recipe = FRecipeDatabase::Get(ID);
		if (Recipe && !Recipe->bRequiresUnlock)
		{
			if (!KnownRecipes.Contains(ID))
			{
				KnownRecipes.Add(ID);
			}
		}
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("CraftingComponent initialized on %s with %d known recipes"),
		*GetOwner()->GetName(), KnownRecipes.Num());
}

void UCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;
	if (CraftingQueue.Num() == 0) return;

	FCraftingQueueEntry& Current = CraftingQueue[0];
	Current.TimeRemaining -= DeltaTime;

	OnCraftingProgress.Broadcast(Current.RecipeID, Current.GetProgress());

	if (Current.TimeRemaining <= 0.0f)
	{
		// Crafting complete
		const UCraftingRecipe* Recipe = GetRecipeDefinition(Current.RecipeID);
		if (Recipe)
		{
			GrantOutput(Recipe);
			OnCraftingCompleted.Broadcast(Current.RecipeID);
			UE_LOG(LogBiomeSurvivor, Log, TEXT("Crafting completed: %s"), *Current.RecipeID.ToString());
		}

		CraftingQueue.RemoveAt(0);

		// Start next in queue if any
		if (CraftingQueue.Num() > 0)
		{
			OnCraftingStarted.Broadcast(CraftingQueue[0].RecipeID);
		}
	}
}

bool UCraftingComponent::StartCrafting(FName RecipeID)
{
	if (!CanCraft(RecipeID))
	{
		OnCraftingFailed.Broadcast(FText::FromString(TEXT("Cannot craft this recipe. Check materials, skill, and station.")));
		return false;
	}

	if (CraftingQueue.Num() >= MaxQueueSize)
	{
		OnCraftingFailed.Broadcast(FText::FromString(TEXT("Crafting queue is full.")));
		return false;
	}

	const UCraftingRecipe* Recipe = GetRecipeDefinition(RecipeID);
	if (!Recipe) return false;

	// Consume ingredients immediately
	if (!ConsumeIngredients(Recipe))
	{
		OnCraftingFailed.Broadcast(FText::FromString(TEXT("Missing ingredients.")));
		return false;
	}

	// Add to queue
	FCraftingQueueEntry Entry;
	Entry.RecipeID = RecipeID;
	Entry.TotalTime = Recipe->CraftingTime;
	Entry.TimeRemaining = Recipe->CraftingTime;
	CraftingQueue.Add(Entry);

	if (CraftingQueue.Num() == 1)
	{
		OnCraftingStarted.Broadcast(RecipeID);
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Crafting started: %s (%.1fs)"), *RecipeID.ToString(), Recipe->CraftingTime);
	return true;
}

void UCraftingComponent::CancelCrafting()
{
	if (CraftingQueue.Num() == 0) return;

	// Refund ingredients for the first item
	FCraftingQueueEntry& Current = CraftingQueue[0];
	const UCraftingRecipe* Recipe = GetRecipeDefinition(Current.RecipeID);
	if (Recipe)
	{
		UInventoryComponent* Inv = GetOwner()->FindComponentByClass<UInventoryComponent>();
		if (Inv)
		{
			for (const FCraftingIngredient& Ingredient : Recipe->Ingredients)
			{
				Inv->AddItem(Ingredient.ItemID, Ingredient.Count);
			}
		}
	}

	CraftingQueue.RemoveAt(0);
}

void UCraftingComponent::CancelQueueEntry(int32 QueueIndex)
{
	if (QueueIndex < 0 || QueueIndex >= CraftingQueue.Num()) return;

	// Refund ingredients
	const UCraftingRecipe* Recipe = GetRecipeDefinition(CraftingQueue[QueueIndex].RecipeID);
	if (Recipe)
	{
		UInventoryComponent* Inv = GetOwner()->FindComponentByClass<UInventoryComponent>();
		if (Inv)
		{
			for (const FCraftingIngredient& Ingredient : Recipe->Ingredients)
			{
				Inv->AddItem(Ingredient.ItemID, Ingredient.Count);
			}
		}
	}

	CraftingQueue.RemoveAt(QueueIndex);
}

bool UCraftingComponent::CanCraft(FName RecipeID) const
{
	const UCraftingRecipe* Recipe = GetRecipeDefinition(RecipeID);
	if (!Recipe) return false;

	// Check if recipe is known
	if (Recipe->bRequiresUnlock && !IsRecipeKnown(RecipeID)) return false;

	// Check station type
	if (Recipe->Category != ECraftingCategory::Basic && Recipe->Category != CurrentStationType)
	{
		return false;
	}

	// Check ingredients
	if (!HasIngredients(RecipeID)) return false;

	// TODO: Check crafting skill level from PlayerState

	return true;
}

bool UCraftingComponent::HasIngredients(FName RecipeID) const
{
	const UCraftingRecipe* Recipe = GetRecipeDefinition(RecipeID);
	if (!Recipe) return false;

	UInventoryComponent* Inv = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return false;

	for (const FCraftingIngredient& Ingredient : Recipe->Ingredients)
	{
		if (!Inv->HasItem(Ingredient.ItemID, Ingredient.Count))
		{
			return false;
		}
	}

	return true;
}

TArray<FName> UCraftingComponent::GetRecipesByCategory(ECraftingCategory Category) const
{
	TArray<FName> Result;
	for (const FName& RecipeID : KnownRecipes)
	{
		const UCraftingRecipe* Recipe = GetRecipeDefinition(RecipeID);
		if (Recipe && Recipe->Category == Category)
		{
			Result.Add(RecipeID);
		}
	}
	return Result;
}

void UCraftingComponent::UnlockRecipe(FName RecipeID)
{
	if (!KnownRecipes.Contains(RecipeID))
	{
		KnownRecipes.Add(RecipeID);
		OnRecipeUnlocked.Broadcast(RecipeID);
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Recipe unlocked: %s"), *RecipeID.ToString());
	}
}

bool UCraftingComponent::IsRecipeKnown(FName RecipeID) const
{
	return KnownRecipes.Contains(RecipeID);
}

const UCraftingRecipe* UCraftingComponent::GetRecipeDefinition(FName RecipeID)
{
	if (RecipeID.IsNone()) return nullptr;

	// Use runtime recipe database
	return FRecipeDatabase::Get(RecipeID);
}

bool UCraftingComponent::ConsumeIngredients(const UCraftingRecipe* Recipe)
{
	UInventoryComponent* Inv = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inv || !Recipe) return false;

	// Verify all ingredients first
	for (const FCraftingIngredient& Ingredient : Recipe->Ingredients)
	{
		if (!Inv->HasItem(Ingredient.ItemID, Ingredient.Count))
		{
			return false;
		}
	}

	// Remove all ingredients
	for (const FCraftingIngredient& Ingredient : Recipe->Ingredients)
	{
		Inv->RemoveItem(Ingredient.ItemID, Ingredient.Count);
	}

	return true;
}

void UCraftingComponent::GrantOutput(const UCraftingRecipe* Recipe)
{
	if (!Recipe) return;

	UInventoryComponent* Inv = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (Inv)
	{
		int32 Overflow = Inv->AddItem(Recipe->OutputItemID, Recipe->OutputCount);
		if (Overflow > 0)
		{
			UE_LOG(LogBiomeSurvivor, Warning, TEXT("Crafting output overflow: %d items of %s could not be stored"),
				Overflow, *Recipe->OutputItemID.ToString());
			// TODO: Drop overflow items on ground
		}
	}
}

// ---- Server RPCs ----

void UCraftingComponent::ServerStartCrafting_Implementation(FName RecipeID)
{
	StartCrafting(RecipeID);
}

void UCraftingComponent::ServerCancelCrafting_Implementation()
{
	CancelCrafting();
}
