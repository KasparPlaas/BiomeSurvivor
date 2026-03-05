// Copyright Biome Survivor. All Rights Reserved.

#include "UI/InventoryWidget.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/EquipmentComponent.h"
#include "BiomeSurvivor.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInventoryWidget::InitializeInventory(UInventoryComponent* InInventory, UEquipmentComponent* InEquipment)
{
	InventoryComp = InInventory;
	EquipmentComp = InEquipment;

	if (InventoryComp)
	{
		// Bind to inventory changes for auto-refresh
		InventoryComp->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::RefreshInventory);
	}

	RefreshInventory();
}

void UInventoryWidget::RefreshInventory()
{
	// Refresh all slot widgets
	for (UInventorySlotWidget* Slot : InventorySlots)
	{
		if (Slot) Slot->RefreshSlot();
	}

	for (UInventorySlotWidget* Slot : QuickbarSlots)
	{
		if (Slot) Slot->RefreshSlot();
	}

	for (UInventorySlotWidget* Slot : EquipmentSlots)
	{
		if (Slot) Slot->RefreshSlot();
	}
}

void UInventoryWidget::RefreshSlot(int32 SlotIndex)
{
	if (SlotIndex >= 0 && SlotIndex < InventorySlots.Num())
	{
		if (InventorySlots[SlotIndex])
		{
			InventorySlots[SlotIndex]->RefreshSlot();
		}
	}
}

void UInventoryWidget::SortInventory()
{
	if (!InventoryComp) return;

	// Sort is handled by inventory component
	// Refresh display after sort
	RefreshInventory();
}

void UInventoryWidget::DropAll()
{
	if (!InventoryComp) return;

	// Drop all items from inventory
	for (int32 i = InventoryComp->GetMainSlotCount() - 1; i >= 0; --i)
	{
		const FItemInstance& Item = InventoryComp->GetItemAtSlot(i);
		if (!Item.ItemID.IsNone())
		{
			InventoryComp->DropItem(i);
		}
	}
}

void UInventoryWidget::OnSlotDragStart(int32 SourceSlot)
{
	DragSourceSlot = SourceSlot;
}

void UInventoryWidget::OnSlotDrop(int32 TargetSlot)
{
	if (DragSourceSlot < 0 || !InventoryComp) return;

	if (DragSourceSlot != TargetSlot)
	{
		InventoryComp->SwapItems(DragSourceSlot, TargetSlot);
	}

	DragSourceSlot = -1;
	RefreshInventory();
}

void UInventoryWidget::OnSlotRightClick(int32 SlotIndex)
{
	if (!InventoryComp) return;

	const FItemInstance& Item = InventoryComp->GetItemAtSlot(SlotIndex);
	if (Item.ItemID.IsNone()) return;

	// Use or equip the item
	InventoryComp->UseItem(SlotIndex);
	RefreshInventory();
}

FText UInventoryWidget::GetWeightText() const
{
	if (!InventoryComp) return FText::GetEmpty();

	float Current = InventoryComp->GetCurrentWeight();
	float Max = InventoryComp->GetMaxCarryWeight();

	return FText::FromString(FString::Printf(TEXT("%.1f / %.1f kg"), Current, Max));
}
