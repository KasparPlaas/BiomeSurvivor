// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

/**
 * Represents a single item slot in the inventory grid
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIVOR_API UInventorySlotWidget : public UUserWidget
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
