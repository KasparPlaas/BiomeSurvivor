// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Crafting/CraftingRecipe.h"
#include "CraftingComponent.generated.h"

/**
 * UCraftingComponent
 * Handles recipe discovery, material checking, crafting queue, and output.
 * Supports both handcrafting and station-based crafting.
 * Replicated for multiplayer.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingStarted, FName, RecipeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingCompleted, FName, RecipeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingFailed, FText, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingProgress, FName, RecipeID, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeUnlocked, FName, RecipeID);

USTRUCT(BlueprintType)
struct FCraftingQueueEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName RecipeID;

	UPROPERTY(BlueprintReadOnly)
	float TimeRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float TotalTime = 0.0f;

	float GetProgress() const { return TotalTime > 0.f ? 1.0f - (TimeRemaining / TotalTime) : 1.0f; }
};

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIVOR_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Maximum number of items in the crafting queue */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	int32 MaxQueueSize = 5;

	// ---- Core API ----

	/** Start crafting a recipe */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	bool StartCrafting(FName RecipeID);

	/** Cancel the current crafting task */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void CancelCrafting();

	/** Cancel a specific queue entry */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void CancelQueueEntry(int32 QueueIndex);

	/** Check if the player can craft a recipe (has materials + skill + station) */
	UFUNCTION(BlueprintPure, Category="Crafting")
	bool CanCraft(FName RecipeID) const;

	/** Check if ingredients are available, ignoring station/skill */
	UFUNCTION(BlueprintPure, Category="Crafting")
	bool HasIngredients(FName RecipeID) const;

	/** Get all recipes the player knows */
	UFUNCTION(BlueprintPure, Category="Crafting")
	TArray<FName> GetKnownRecipes() const { return KnownRecipes; }

	/** Get recipes filtered by category */
	UFUNCTION(BlueprintPure, Category="Crafting")
	TArray<FName> GetRecipesByCategory(ECraftingCategory Category) const;

	/** Unlock a recipe */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void UnlockRecipe(FName RecipeID);

	/** Check if a recipe is known */
	UFUNCTION(BlueprintPure, Category="Crafting")
	bool IsRecipeKnown(FName RecipeID) const;

	/** Is currently crafting something? */
	UFUNCTION(BlueprintPure, Category="Crafting")
	bool IsCrafting() const { return CraftingQueue.Num() > 0; }

	/** Get current queue */
	UFUNCTION(BlueprintPure, Category="Crafting")
	const TArray<FCraftingQueueEntry>& GetCraftingQueue() const { return CraftingQueue; }

	/** Set the current station type the player is using */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void SetCurrentStation(ECraftingCategory StationType) { CurrentStationType = StationType; }

	UFUNCTION(BlueprintPure, Category="Crafting")
	ECraftingCategory GetCurrentStation() const { return CurrentStationType; }

	// ---- Recipe Lookup ----
	UFUNCTION(BlueprintPure, Category="Crafting")
	static const UCraftingRecipe* GetRecipeDefinition(FName RecipeID);

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnCraftingStarted OnCraftingStarted;

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnCraftingCompleted OnCraftingCompleted;

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnCraftingFailed OnCraftingFailed;

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnCraftingProgress OnCraftingProgress;

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnRecipeUnlocked OnRecipeUnlocked;

	// ---- Server RPCs ----
	UFUNCTION(Server, Reliable)
	void ServerStartCrafting(FName RecipeID);

	UFUNCTION(Server, Reliable)
	void ServerCancelCrafting();

private:
	UPROPERTY()
	TArray<FName> KnownRecipes;

	UPROPERTY()
	TArray<FCraftingQueueEntry> CraftingQueue;

	/** Current station type (Basic = handcrafting) */
	ECraftingCategory CurrentStationType = ECraftingCategory::Basic;

	/** Consume ingredients from inventory */
	bool ConsumeIngredients(const UCraftingRecipe* Recipe);

	/** Grant output item to inventory */
	void GrantOutput(const UCraftingRecipe* Recipe);
};
