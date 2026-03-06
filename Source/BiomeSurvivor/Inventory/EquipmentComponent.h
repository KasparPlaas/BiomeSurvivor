// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/ItemDefinition.h"
#include "EquipmentComponent.generated.h"

/**
 * UEquipmentComponent
 * Manages equipped items (armor, weapons, clothing).
 * 8 equipment slots: Head, Chest, Legs, Feet, Hands, Back, MainHand, OffHand.
 * Provides aggregate warmth, armor, and resistance values.
 * Replicated for multiplayer visual sync.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChanged, EEquipSlot, Slot, const FItemInstance&, Item);

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIVOR_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Equip an item from inventory to the appropriate slot */
	UFUNCTION(BlueprintCallable, Category="Equipment")
	bool EquipItem(const FItemInstance& Item);

	/** Unequip item from a slot, returns it to inventory */
	UFUNCTION(BlueprintCallable, Category="Equipment")
	FItemInstance UnequipSlot(EEquipSlot Slot);

	/** Get the item in a specific slot */
	UFUNCTION(BlueprintPure, Category="Equipment")
	FItemInstance GetEquippedItem(EEquipSlot Slot) const;

	/** Check if a slot has an item */
	UFUNCTION(BlueprintPure, Category="Equipment")
	bool IsSlotOccupied(EEquipSlot Slot) const;

	/** Get current weapon in main hand */
	UFUNCTION(BlueprintPure, Category="Equipment")
	FItemInstance GetMainHandWeapon() const { return GetEquippedItem(EEquipSlot::MainHand); }

	// ---- Aggregate Stats ----

	/** Total armor value from all equipped items */
	UFUNCTION(BlueprintPure, Category="Equipment")
	float GetTotalArmor() const;

	/** Total warmth from all clothing */
	UFUNCTION(BlueprintPure, Category="Equipment")
	float GetTotalWarmth() const;

	/** Total water resistance from all clothing */
	UFUNCTION(BlueprintPure, Category="Equipment")
	float GetTotalWaterResistance() const;

	/** Total wind resistance from all clothing */
	UFUNCTION(BlueprintPure, Category="Equipment")
	float GetTotalWindResistance() const;

	/** Reduce durability of equipped item in slot */
	UFUNCTION(BlueprintCallable, Category="Equipment")
	void DegradeItem(EEquipSlot Slot, float Amount);

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category="Equipment")
	FOnEquipmentChanged OnEquipmentChanged;

	// ---- Server RPC ----
	UFUNCTION(Server, Reliable)
	void ServerEquipItem(const FItemInstance& Item);

	UFUNCTION(Server, Reliable)
	void ServerUnequipSlot(EEquipSlot Slot);

private:
	UPROPERTY()
	TMap<EEquipSlot, FItemInstance> EquippedItems;

	void RecalculateStats();
};
