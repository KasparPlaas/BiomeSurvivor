// Copyright Biome Survivor. All Rights Reserved.

#include "UI/CraftingWidget.h"
#include "Crafting/CraftingComponent.h"
#include "Crafting/CraftingRecipe.h"
#include "Inventory/InventoryComponent.h"
#include "BiomeSurvivor.h"

void UCraftingRecipeEntry::SetRecipe(UCraftingRecipe* InRecipe)
{
	Recipe = InRecipe;
}

void UCraftingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCraftingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update recipe craftability status
	if (CraftingComp && InventoryComp)
	{
		for (UCraftingRecipeEntry* Entry : RecipeEntries)
		{
			if (Entry && Entry->Recipe)
			{
				bool bCanCraft = CraftingComp->CanCraft(Entry->Recipe);
				Entry->UpdateCraftability(bCanCraft);
			}
		}
	}
}

void UCraftingWidget::InitializeCrafting(UCraftingComponent* InCrafting, UInventoryComponent* InInventory)
{
	CraftingComp = InCrafting;
	InventoryComp = InInventory;

	RefreshRecipeList();
}

void UCraftingWidget::RefreshRecipeList()
{
	if (!CraftingComp) return;

	// Clear existing entries
	RecipeEntries.Empty();

	// Get discovered recipes from crafting component
	const TArray<UCraftingRecipe*>& Discovered = CraftingComp->GetDiscoveredRecipes();

	for (UCraftingRecipe* Recipe : Discovered)
	{
		if (!Recipe) continue;

		// Apply category filter
		if (ActiveCategoryFilter != 255)
		{
			if ((uint8)Recipe->Category != ActiveCategoryFilter)
				continue;
		}

		// Create entry widget
		if (RecipeEntryClass)
		{
			UCraftingRecipeEntry* Entry = CreateWidget<UCraftingRecipeEntry>(this, RecipeEntryClass);
			if (Entry)
			{
				Entry->SetRecipe(Recipe);
				RecipeEntries.Add(Entry);
			}
		}
	}
}

void UCraftingWidget::SelectRecipe(UCraftingRecipe* Recipe)
{
	// Deselect previous
	for (UCraftingRecipeEntry* Entry : RecipeEntries)
	{
		if (Entry)
		{
			Entry->SetSelected(Entry->Recipe == Recipe);
		}
	}

	SelectedRecipe = Recipe;
}

void UCraftingWidget::CraftSelected()
{
	if (!CraftingComp || !SelectedRecipe) return;

	CraftingComp->StartCrafting(SelectedRecipe);
}

void UCraftingWidget::CancelQueueItem(int32 QueueIndex)
{
	if (!CraftingComp) return;

	CraftingComp->CancelCrafting(QueueIndex);
}

void UCraftingWidget::SetCategoryFilter(uint8 Category)
{
	ActiveCategoryFilter = Category;
	RefreshRecipeList();
}

float UCraftingWidget::GetCurrentCraftingProgress() const
{
	if (!CraftingComp) return 0.0f;
	return CraftingComp->GetCraftingProgress();
}

FText UCraftingWidget::GetCraftingTimeText() const
{
	if (!SelectedRecipe) return FText::GetEmpty();

	float Seconds = SelectedRecipe->CraftingTime;
	if (Seconds < 60.0f)
	{
		return FText::FromString(FString::Printf(TEXT("%.0fs"), Seconds));
	}

	int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
	int32 Secs = FMath::FloorToInt(FMath::Fmod(Seconds, 60.0f));
	return FText::FromString(FString::Printf(TEXT("%dm %ds"), Minutes, Secs));
}

int32 UCraftingWidget::GetQueueCount() const
{
	if (!CraftingComp) return 0;
	return CraftingComp->GetQueueCount();
}
