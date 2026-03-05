// Copyright Biome Survivor. All Rights Reserved.

#include "Inventory/EquipmentComponent.h"
#include "Inventory/InventoryComponent.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEquipmentComponent, EquippedItems);
}

bool UEquipmentComponent::EquipItem(const FItemInstance& Item)
{
	if (Item.IsEmpty()) return false;

	const UItemDefinition* Def = UInventoryComponent::GetItemDefinition(Item.ItemID);
	if (!Def || !Def->bEquippable || Def->EquipSlot == EEquipSlot::None)
	{
		UE_LOG(LogBiomeSurvivor, Warning, TEXT("Cannot equip item %s - not equippable"), *Item.ItemID.ToString());
		return false;
	}

	EEquipSlot TargetSlot = Def->EquipSlot;

	// If slot is occupied, unequip current item first
	if (IsSlotOccupied(TargetSlot))
	{
		UnequipSlot(TargetSlot);
	}

	// Equip the new item
	FItemInstance Equipped = Item;
	Equipped.StackCount = 1; // Equipment is always single
	EquippedItems.Add(TargetSlot, Equipped);

	OnEquipmentChanged.Broadcast(TargetSlot, Equipped);
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Equipped %s in slot %d"), *Item.ItemID.ToString(), (int32)TargetSlot);

	return true;
}

FItemInstance UEquipmentComponent::UnequipSlot(EEquipSlot Slot)
{
	FItemInstance Removed;

	if (FItemInstance* Found = EquippedItems.Find(Slot))
	{
		Removed = *Found;
		EquippedItems.Remove(Slot);

		// Return to inventory
		if (UInventoryComponent* Inv = GetOwner()->FindComponentByClass<UInventoryComponent>())
		{
			Inv->AddItem(Removed.ItemID, 1);
		}

		FItemInstance Empty;
		OnEquipmentChanged.Broadcast(Slot, Empty);
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Unequipped %s from slot %d"), *Removed.ItemID.ToString(), (int32)Slot);
	}

	return Removed;
}

FItemInstance UEquipmentComponent::GetEquippedItem(EEquipSlot Slot) const
{
	if (const FItemInstance* Found = EquippedItems.Find(Slot))
	{
		return *Found;
	}
	return FItemInstance();
}

bool UEquipmentComponent::IsSlotOccupied(EEquipSlot Slot) const
{
	return EquippedItems.Contains(Slot) && !EquippedItems[Slot].IsEmpty();
}

float UEquipmentComponent::GetTotalArmor() const
{
	float Total = 0.0f;
	for (const auto& Pair : EquippedItems)
	{
		const UItemDefinition* Def = UInventoryComponent::GetItemDefinition(Pair.Value.ItemID);
		if (Def) Total += Def->Protection.Armor;
	}
	return Total;
}

float UEquipmentComponent::GetTotalWarmth() const
{
	float Total = 0.0f;
	for (const auto& Pair : EquippedItems)
	{
		const UItemDefinition* Def = UInventoryComponent::GetItemDefinition(Pair.Value.ItemID);
		if (Def) Total += Def->Protection.Warmth;
	}
	return Total;
}

float UEquipmentComponent::GetTotalWaterResistance() const
{
	float Total = 0.0f;
	for (const auto& Pair : EquippedItems)
	{
		const UItemDefinition* Def = UInventoryComponent::GetItemDefinition(Pair.Value.ItemID);
		if (Def) Total += Def->Protection.WaterResistance;
	}
	return FMath::Clamp(Total, 0.0f, 1.0f);
}

float UEquipmentComponent::GetTotalWindResistance() const
{
	float Total = 0.0f;
	for (const auto& Pair : EquippedItems)
	{
		const UItemDefinition* Def = UInventoryComponent::GetItemDefinition(Pair.Value.ItemID);
		if (Def) Total += Def->Protection.WindResistance;
	}
	return FMath::Clamp(Total, 0.0f, 1.0f);
}

void UEquipmentComponent::DegradeItem(EEquipSlot Slot, float Amount)
{
	if (FItemInstance* Item = EquippedItems.Find(Slot))
	{
		if (Item->CurrentDurability > 0.0f)
		{
			Item->CurrentDurability = FMath::Max(0.0f, Item->CurrentDurability - Amount);
			if (Item->CurrentDurability <= 0.0f)
			{
				UE_LOG(LogBiomeSurvivor, Log, TEXT("Equipment broken: %s"), *Item->ItemID.ToString());
				// Item breaks - remove it
				FItemInstance Empty;
				EquippedItems.Remove(Slot);
				OnEquipmentChanged.Broadcast(Slot, Empty);
			}
		}
	}
}

// ---- Server RPCs ----

void UEquipmentComponent::ServerEquipItem_Implementation(const FItemInstance& Item)
{
	EquipItem(Item);
}

void UEquipmentComponent::ServerUnequipSlot_Implementation(EEquipSlot Slot)
{
	UnequipSlot(Slot);
}
