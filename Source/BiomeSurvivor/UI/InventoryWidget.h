// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UEquipmentComponent;
class UUniformGridPanel;
class UImage;
class UTextBlock;

/**
 * Represents a single item slot in the inventory grid
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The inventory slot index this widget represents */
	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	int32 SlotIndex = -1;

	/** Whether this is a quickbar slot */
	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	bool bIsQuickbarSlot = false;

	/** Whether this is an equipment slot */
	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	bool bIsEquipmentSlot = false;

	/** Equipment slot type (if applicable) */
	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	uint8 EquipSlotType = 0;

	/** Update the slot visual display */
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot")
	void RefreshSlot();

	/** Set highlight state (for drag/drop targeting) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot")
	void SetHighlighted(bool bHighlight);

	/** Get whether the slot contains an item */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	bool HasItem() const { return bHasItem; }

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Slot")
	bool bHasItem = false;
};

/**
 * Main inventory widget showing:
 * - Grid of inventory slots (30 main + 8 quickbar)
 * - Equipment slots (paperdoll view)
 * - Player stats summary
 * - Weight indicator
 * 
 * Supports drag-and-drop between all slot types.
 * Designed for Blueprint subclassing with UMG layout.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Initialize widget with component references */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(UInventoryComponent* InInventory, UEquipmentComponent* InEquipment);

	/** Refresh all slots display */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshInventory();

	/** Refresh a specific slot */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshSlot(int32 SlotIndex);

	/** Sort inventory by category */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SortInventory();

	/** Drop all items (for death) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropAll();

	/** Handle drag start from a slot */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OnSlotDragStart(int32 SourceSlot);

	/** Handle drop onto a slot */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OnSlotDrop(int32 TargetSlot);

	/** Called when right-clicking an item (use/equip) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OnSlotRightClick(int32 SlotIndex);

	/** Get formatted weight string (e.g. "12.5 / 40.0 kg") */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FText GetWeightText() const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<UEquipmentComponent> EquipmentComp;

	/** Inventory slot widget class to spawn per slot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TArray<UInventorySlotWidget*> InventorySlots;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TArray<UInventorySlotWidget*> QuickbarSlots;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TArray<UInventorySlotWidget*> EquipmentSlots;

private:
	int32 DragSourceSlot = -1;
};
