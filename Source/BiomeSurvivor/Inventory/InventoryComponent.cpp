// Copyright Biome Survivor. All Rights Reserved.

#include "Inventory/InventoryComponent.h"
#include "Player/PlayerStatsComponent.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize inventory slots
	InventorySlots.SetNum(MaxSlots);
	for (int32 i = 0; i < MaxSlots; ++i)
	{
		InventorySlots[i].Clear();
		InventorySlots[i].SlotIndex = i;
	}

	// Initialize quick-bar mapping (-1 = unassigned)
	QuickBarMapping.Init(-1, QuickBarSlots);

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Inventory initialized: %d slots, %d quickbar slots"), MaxSlots, QuickBarSlots);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, InventorySlots);
}

// ============ CORE API ============

int32 UInventoryComponent::AddItem(FName ItemID, int32 Count)
{
	if (ItemID.IsNone() || Count <= 0) return Count;

	const UItemDefinition* Def = GetItemDefinition(ItemID);
	int32 Remaining = Count;

	// First pass: try to stack onto existing slots
	if (Def && Def->MaxStackSize > 1)
	{
		for (int32 i = 0; i < InventorySlots.Num() && Remaining > 0; ++i)
		{
			FItemInstance& Slot = InventorySlots[i];
			if (Slot.ItemID == ItemID && Slot.StackCount < Def->MaxStackSize)
			{
				int32 Space = Def->MaxStackSize - Slot.StackCount;
				int32 ToAdd = FMath::Min(Space, Remaining);
				Slot.StackCount += ToAdd;
				Remaining -= ToAdd;
				NotifySlotChanged(i);
			}
		}
	}

	// Second pass: put into empty slots
	while (Remaining > 0)
	{
		int32 EmptySlot = FindEmptySlot();
		if (EmptySlot < 0)
		{
			UE_LOG(LogBiomeSurvivor, Warning, TEXT("Inventory full! %d items of %s could not be added."), Remaining, *ItemID.ToString());
			break;
		}

		int32 MaxStack = Def ? Def->MaxStackSize : 1;
		int32 ToAdd = FMath::Min(MaxStack, Remaining);

		FItemInstance& Slot = InventorySlots[EmptySlot];
		Slot.ItemID = ItemID;
		Slot.StackCount = ToAdd;
		Slot.SlotIndex = EmptySlot;
		Slot.CachedDefinition = Def;
		if (Def && Def->MaxDurability > 0.0f)
		{
			Slot.CurrentDurability = Def->MaxDurability;
		}

		Remaining -= ToAdd;
		NotifySlotChanged(EmptySlot);
	}

	UpdateWeight();
	return Remaining; // 0 = all added successfully
}

int32 UInventoryComponent::RemoveItem(FName ItemID, int32 Count)
{
	int32 Remaining = Count;

	for (int32 i = InventorySlots.Num() - 1; i >= 0 && Remaining > 0; --i)
	{
		FItemInstance& Slot = InventorySlots[i];
		if (Slot.ItemID == ItemID)
		{
			int32 ToRemove = FMath::Min(Slot.StackCount, Remaining);
			Slot.StackCount -= ToRemove;
			Remaining -= ToRemove;

			if (Slot.StackCount <= 0)
			{
				Slot.Clear();
				Slot.SlotIndex = i;
				OnInventorySlotCleared.Broadcast(i);
			}
			NotifySlotChanged(i);
		}
	}

	UpdateWeight();
	return Count - Remaining; // Actual amount removed
}

FItemInstance UInventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 Count)
{
	if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num()) return FItemInstance();

	FItemInstance& Slot = InventorySlots[SlotIndex];
	if (Slot.IsEmpty()) return FItemInstance();

	FItemInstance Removed;
	Removed.ItemID = Slot.ItemID;

	if (Count < 0 || Count >= Slot.StackCount)
	{
		Removed.StackCount = Slot.StackCount;
		Removed.CurrentDurability = Slot.CurrentDurability;
		Slot.Clear();
		Slot.SlotIndex = SlotIndex;
		OnInventorySlotCleared.Broadcast(SlotIndex);
	}
	else
	{
		Removed.StackCount = Count;
		Slot.StackCount -= Count;
	}

	NotifySlotChanged(SlotIndex);
	UpdateWeight();
	return Removed;
}

bool UInventoryComponent::MoveItem(int32 FromSlot, int32 ToSlot)
{
	if (FromSlot < 0 || FromSlot >= InventorySlots.Num()) return false;
	if (ToSlot < 0 || ToSlot >= InventorySlots.Num()) return false;
	if (FromSlot == ToSlot) return true;

	FItemInstance& From = InventorySlots[FromSlot];
	FItemInstance& To = InventorySlots[ToSlot];

	if (From.IsEmpty()) return false;

	// If target is empty, just move
	if (To.IsEmpty())
	{
		To = From;
		To.SlotIndex = ToSlot;
		From.Clear();
		From.SlotIndex = FromSlot;
		NotifySlotChanged(FromSlot);
		NotifySlotChanged(ToSlot);
		return true;
	}

	// If same item, try to stack
	if (From.ItemID == To.ItemID)
	{
		const UItemDefinition* Def = GetItemDefinition(From.ItemID);
		int32 MaxStack = Def ? Def->MaxStackSize : 1;
		int32 Space = MaxStack - To.StackCount;
		if (Space > 0)
		{
			int32 ToMove = FMath::Min(Space, From.StackCount);
			To.StackCount += ToMove;
			From.StackCount -= ToMove;
			if (From.StackCount <= 0)
			{
				From.Clear();
				From.SlotIndex = FromSlot;
			}
			NotifySlotChanged(FromSlot);
			NotifySlotChanged(ToSlot);
			return true;
		}
	}

	// Otherwise swap
	return SwapItems(FromSlot, ToSlot);
}

bool UInventoryComponent::SwapItems(int32 SlotA, int32 SlotB)
{
	if (SlotA < 0 || SlotA >= InventorySlots.Num()) return false;
	if (SlotB < 0 || SlotB >= InventorySlots.Num()) return false;

	FItemInstance Temp = InventorySlots[SlotA];
	InventorySlots[SlotA] = InventorySlots[SlotB];
	InventorySlots[SlotA].SlotIndex = SlotA;
	InventorySlots[SlotB] = Temp;
	InventorySlots[SlotB].SlotIndex = SlotB;

	NotifySlotChanged(SlotA);
	NotifySlotChanged(SlotB);
	return true;
}

bool UInventoryComponent::SplitStack(int32 SourceSlot, int32 Count)
{
	if (SourceSlot < 0 || SourceSlot >= InventorySlots.Num()) return false;

	FItemInstance& Source = InventorySlots[SourceSlot];
	if (Source.IsEmpty() || Source.StackCount <= 1 || Count >= Source.StackCount) return false;

	int32 EmptySlot = FindEmptySlot();
	if (EmptySlot < 0) return false;

	FItemInstance& Target = InventorySlots[EmptySlot];
	Target.ItemID = Source.ItemID;
	Target.StackCount = Count;
	Target.SlotIndex = EmptySlot;
	Target.CachedDefinition = Source.CachedDefinition;

	Source.StackCount -= Count;

	NotifySlotChanged(SourceSlot);
	NotifySlotChanged(EmptySlot);
	return true;
}

bool UInventoryComponent::HasItem(FName ItemID, int32 Count) const
{
	return GetItemCount(ItemID) >= Count;
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	int32 Total = 0;
	for (const FItemInstance& Slot : InventorySlots)
	{
		if (Slot.ItemID == ItemID)
		{
			Total += Slot.StackCount;
		}
	}
	return Total;
}

FItemInstance UInventoryComponent::GetItemAtSlot(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < InventorySlots.Num())
	{
		return InventorySlots[SlotIndex];
	}
	return FItemInstance();
}

int32 UInventoryComponent::FindItemSlot(FName ItemID) const
{
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (InventorySlots[i].ItemID == ItemID)
		{
			return i;
		}
	}
	return -1;
}

int32 UInventoryComponent::FindEmptySlot() const
{
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (InventorySlots[i].IsEmpty())
		{
			return i;
		}
	}
	return -1;
}

float UInventoryComponent::GetTotalWeight() const
{
	float Total = 0.0f;
	for (const FItemInstance& Slot : InventorySlots)
	{
		if (!Slot.IsEmpty())
		{
			const UItemDefinition* Def = GetItemDefinition(Slot.ItemID);
			if (Def)
			{
				Total += Def->Weight * Slot.StackCount;
			}
		}
	}
	return Total;
}

int32 UInventoryComponent::GetUsedSlotCount() const
{
	int32 Count = 0;
	for (const FItemInstance& Slot : InventorySlots)
	{
		if (!Slot.IsEmpty()) Count++;
	}
	return Count;
}

bool UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num()) return false;

	FItemInstance& Slot = InventorySlots[SlotIndex];
	if (Slot.IsEmpty()) return false;

	const UItemDefinition* Def = GetItemDefinition(Slot.ItemID);
	if (!Def || !Def->bConsumable) return false;

	// Apply food/drink effects
	if (Def->Category == EItemCategory::Food || Def->Category == EItemCategory::Medical)
	{
		if (UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>())
		{
			if (Def->Nutrition.HungerRestore > 0.0f) Stats->RestoreHunger(Def->Nutrition.HungerRestore);
			if (Def->Nutrition.ThirstRestore > 0.0f) Stats->RestoreThirst(Def->Nutrition.ThirstRestore);
			if (Def->Nutrition.HealthRestore > 0.0f) Stats->Heal(Def->Nutrition.HealthRestore);
		}
	}

	// Consume the item
	Slot.StackCount -= 1;
	if (Slot.StackCount <= 0)
	{
		Slot.Clear();
		Slot.SlotIndex = SlotIndex;
		OnInventorySlotCleared.Broadcast(SlotIndex);
	}

	NotifySlotChanged(SlotIndex);
	UpdateWeight();
	return true;
}

bool UInventoryComponent::DropItem(int32 SlotIndex, int32 Count)
{
	if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num()) return false;

	FItemInstance Removed = RemoveItemFromSlot(SlotIndex, Count);
	if (Removed.IsEmpty()) return false;

	// TODO: Spawn pickup actor in world at player's feet
	// For now, items are just removed
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Dropped %d x %s"), Removed.StackCount, *Removed.ItemID.ToString());
	return true;
}

// ---- Quick-bar ----

void UInventoryComponent::AssignToQuickBar(int32 InventorySlot, int32 QuickBarIndex)
{
	if (QuickBarIndex >= 0 && QuickBarIndex < QuickBarMapping.Num())
	{
		QuickBarMapping[QuickBarIndex] = InventorySlot;
	}
}

int32 UInventoryComponent::GetQuickBarSlot(int32 QuickBarIndex) const
{
	if (QuickBarIndex >= 0 && QuickBarIndex < QuickBarMapping.Num())
	{
		return QuickBarMapping[QuickBarIndex];
	}
	return -1;
}

// ---- Static Item Lookup ----

const UItemDefinition* UInventoryComponent::GetItemDefinition(FName ItemID)
{
	if (ItemID.IsNone()) return nullptr;

	UAssetManager& Manager = UAssetManager::Get();
	FPrimaryAssetId AssetId("ItemDefinition", ItemID);
	FSoftObjectPath Path = Manager.GetPrimaryAssetPath(AssetId);

	if (Path.IsValid())
	{
		return Cast<UItemDefinition>(Path.TryLoad());
	}

	return nullptr;
}

// ---- Server RPCs ----

void UInventoryComponent::ServerAddItem_Implementation(FName ItemID, int32 Count)
{
	AddItem(ItemID, Count);
}

void UInventoryComponent::ServerRemoveItem_Implementation(FName ItemID, int32 Count)
{
	RemoveItem(ItemID, Count);
}

void UInventoryComponent::ServerMoveItem_Implementation(int32 FromSlot, int32 ToSlot)
{
	MoveItem(FromSlot, ToSlot);
}

void UInventoryComponent::ServerDropItem_Implementation(int32 SlotIndex, int32 Count)
{
	DropItem(SlotIndex, Count);
}

// ---- Private ----

void UInventoryComponent::UpdateWeight()
{
	if (UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>())
	{
		Stats->CurrentWeight = GetTotalWeight();
	}
}

void UInventoryComponent::NotifySlotChanged(int32 SlotIndex)
{
	if (SlotIndex >= 0 && SlotIndex < InventorySlots.Num())
	{
		OnInventoryChanged.Broadcast(SlotIndex, InventorySlots[SlotIndex]);
	}
}
