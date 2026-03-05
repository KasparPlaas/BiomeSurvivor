// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/ItemDefinition.h"
#include "InventoryComponent.generated.h"

/**
 * UInventoryComponent
 * Grid-based inventory with weight limits, stacking, quick-bar slots.
 * 30 main slots + 8 quick-bar slots. Weight limit from PlayerStatsComponent.
 * Replicated for multiplayer.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, int32, SlotIndex, const FItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventorySlotCleared, int32, SlotIndex);

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Configuration ----

	/** Total number of main inventory slots */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 MaxSlots = 30;

	/** Number of quick-bar (hotbar) slots */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 QuickBarSlots = 8;

	// ---- Core API ----

	/** Try to add an item. Returns number of items that could NOT be added (0 = success). */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 AddItem(FName ItemID, int32 Count = 1);

	/** Remove a specific count of an item by ID. Returns actual amount removed. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 RemoveItem(FName ItemID, int32 Count = 1);

	/** Remove item from a specific slot */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	FItemInstance RemoveItemFromSlot(int32 SlotIndex, int32 Count = -1);

	/** Move item from one slot to another */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool MoveItem(int32 FromSlot, int32 ToSlot);

	/** Swap items between two slots */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool SwapItems(int32 SlotA, int32 SlotB);

	/** Split a stack: move Count items from SourceSlot to an empty slot */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool SplitStack(int32 SourceSlot, int32 Count);

	/** Check if player has at least Count of ItemID */
	UFUNCTION(BlueprintPure, Category="Inventory")
	bool HasItem(FName ItemID, int32 Count = 1) const;

	/** Get total count of a specific item across all slots */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetItemCount(FName ItemID) const;

	/** Get the item in a specific slot */
	UFUNCTION(BlueprintPure, Category="Inventory")
	FItemInstance GetItemAtSlot(int32 SlotIndex) const;

	/** Get all items (main inventory) */
	UFUNCTION(BlueprintPure, Category="Inventory")
	const TArray<FItemInstance>& GetAllItems() const { return InventorySlots; }

	/** Find the first slot containing this item */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 FindItemSlot(FName ItemID) const;

	/** Find first empty slot. Returns -1 if full. */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 FindEmptySlot() const;

	/** Calculate total weight of all inventory items */
	UFUNCTION(BlueprintPure, Category="Inventory")
	float GetTotalWeight() const;

	/** Get the number of used slots */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetUsedSlotCount() const;

	/** Use/consume an item at slot (e.g., eat food, apply bandage) */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool UseItem(int32 SlotIndex);

	/** Drop item from slot into the world */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool DropItem(int32 SlotIndex, int32 Count = 1);

	// ---- Quick-bar ----

	/** Assign an inventory slot to a quick-bar position */
	UFUNCTION(BlueprintCallable, Category="Inventory|QuickBar")
	void AssignToQuickBar(int32 InventorySlot, int32 QuickBarIndex);

	/** Get the quick-bar mapping */
	UFUNCTION(BlueprintPure, Category="Inventory|QuickBar")
	int32 GetQuickBarSlot(int32 QuickBarIndex) const;

	// ---- Item Database Lookup ----

	/** Resolve an ItemID to its definition. Uses the AssetManager. */
	UFUNCTION(BlueprintPure, Category="Inventory")
	static const UItemDefinition* GetItemDefinition(FName ItemID);

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventorySlotCleared OnInventorySlotCleared;

	// ---- Server RPCs ----
	UFUNCTION(Server, Reliable)
	void ServerAddItem(FName ItemID, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerRemoveItem(FName ItemID, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerMoveItem(int32 FromSlot, int32 ToSlot);

	UFUNCTION(Server, Reliable)
	void ServerDropItem(int32 SlotIndex, int32 Count);

private:
	UPROPERTY(Replicated)
	TArray<FItemInstance> InventorySlots;

	UPROPERTY()
	TArray<int32> QuickBarMapping; // QuickBarIndex -> InventorySlotIndex

	void UpdateWeight();
	void NotifySlotChanged(int32 SlotIndex);
};
